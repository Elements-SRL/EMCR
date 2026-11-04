#include "stampplot.h"
#include "stampoverlaystyle.h"


#include "qstyle.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_canvas.h"
#include <QApplication>
#include "globaldefines.h"
#include "themecontroller.h"
#include "curve.h"
#include <QFontMetricsF>
#include <QPainter>
#include <QPixmapCache>
#include <QtMath>

using namespace e384CommLib;

namespace {
// Canvas of the mini-plots (the signal trace preview). Its QSS background is a
// solid color, so the canvas stays opaque: a transparent QSS background makes Qt
// drop WA_OpaquePaintEvent and Qwt copies the card background at every replot.
class StampPlotCanvas : public QwtPlotCanvas {
public:
    StampPlotCanvas() : QwtPlotCanvas() {}

protected:
    void paintEvent(QPaintEvent *e) override {
        // The card was off screen at the last replot (see StampPlot::replotData):
        // the cached image is old, redraw it with the current data
        StampPlot *plot = qobject_cast<StampPlot *>(parentWidget());
        if (plot != nullptr && plot->takeDataDirty()) {
            invalidateBackingStore();
        }
        QwtPlotCanvas::paintEvent(e);
    }
};

// --- Overlay Drawing -------------------------------------------
// Params to tune aspect are in - StampOverlayStyle.
using Style = StampOverlayStyle;

QColor paletteColor(const char *var) {
    return ThemeController::color(QLatin1String(var));
}

QFont overlayFont(const QFont &base, int pixelSize) {
    QFont f(base);
    f.setPixelSize(pixelSize);
    f.setWeight(QFont::Bold);
    return f;
}

// Box rounded with text centered, rendered only once in a pixmap
QPixmap renderTextBox(const QString &text, const QFont &font, const QSize &size, qreal radius,
                      const QColor &bg, const QColor &fg, const QColor &border, qreal borderWidth, qreal dpr) {
    QPixmap pm(qCeil(size.width() * dpr), qCeil(size.height() * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF box(0.0, 0.0, size.width(), size.height());
    if (borderWidth > 0.0) {
        const qreal h = borderWidth / 2.0;
        p.setPen(QPen(border, borderWidth));
        p.setBrush(bg);
        p.drawRoundedRect(box.adjusted(h, h, -h, -h), radius, radius);
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(box, radius, radius);
    }

    p.setFont(font);
    p.setPen(fg);
    p.drawText(box, Qt::AlignCenter | Qt::TextSingleLine | Qt::TextDontClip, text);
    return pm;
}

// Badge pixmap, shared among all the cards through QPixmapCache.
// The key contains colors, font and DPR: when the theme or the monitor changes
// a new variant is created, without having to clear the cache.
QPixmap badgePixmap(const Style::Badge &badge, const QFont &baseFont, qreal dpr) {
    const QString pfx = QStringLiteral("@BDG_%1_").arg(QLatin1String(badge.paletteKey));
    const QColor bg = ThemeController::color(pfx + QStringLiteral("BG_CH"));
    const QColor fg = ThemeController::color(pfx + QStringLiteral("FG_CH"));
    const QFont font = overlayFont(baseFont, Style::badgeFontPx);
    const QString letter = QString(QLatin1Char(badge.letter));

    const QString key = QStringLiteral("emcr.stampBadge|%1|%2|%3|%4|%5")
            .arg(letter)
            .arg(bg.rgba())
            .arg(fg.rgba())
            .arg(dpr)
            .arg(font.key());

    QPixmap pm;
    if (!QPixmapCache::find(key, &pm)) {
        const QFontMetricsF fm(font);
        const int w = qBound(Style::minBoxSize, qCeil(fm.horizontalAdvance(letter)) + 2 * Style::badgeHPadding, Style::maxBoxSize);
        const int h = qBound(Style::minBoxSize, qCeil(fm.height()), Style::maxBoxSize);
        pm = renderTextBox(letter, font, QSize(w, h), Style::badgeRadius, bg, fg, QColor(), 0.0, dpr);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}
}

StampPlot::StampPlot(int channelIdx, std::string channelname, int idealPlotWidth, int idealPlotHeight, QWidget * parent) :
    QwtPlot(parent),
    channelIdx(channelIdx),
    idealPlotWidth(idealPlotWidth),
    idealPlotHeight(idealPlotHeight) {

    this->plotLayout()->setAlignCanvasToScales(true);

    for (int axis = 0; axis < axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
        this->enableAxis(axis, false);
    }

    QwtPlotCanvas * canvas = new StampPlotCanvas();
    canvas->setFrameStyle(QFrame::NoFrame);
    canvas->setObjectName("stampPlotCanvas");
    // The canvas is rectangular and opaque: it is inset so that it does not
    // cover the rounded corners of the card (QSS #stampPlotCard)
    this->setContentsMargins(StampOverlayStyle::canvasInset, StampOverlayStyle::canvasInset,
                             StampOverlayStyle::canvasInset, StampOverlayStyle::canvasInset);
    this->setCanvas(canvas);
    this->setObjectName("stampPlotCard");
    this->setProperty("status", "default");

    channelName = QString::fromStdString(channelname);

    xAxisMaxMajor = this->axisMaxMajor(xBottom);
    yAxisMaxMajor = this->axisMaxMajor(yLeft);

    selected = false;

    // Theme aware overlay changes (and curve color)
    connect(&ThemeController::getInstance(), &ThemeController::sigThemeUpdated, this, [this]() {
        invalidateOverlayCache();
        applyThemeToCurves();
        requestReplot();
    });
}

QSize StampPlot::sizeHint() const {
    return QSize(idealPlotWidth, idealPlotHeight);
}

QSize StampPlot::minimumSizeHint() const {
    // Minimum space for overlays
    const QFontMetricsF nameFm(overlayFont(this->font(), Style::nameFontPx));
    const QFontMetricsF badgeFm(overlayFont(this->font(), Style::badgeFontPx));
    const int border = qCeil(Style::nameBorderWidth);

    const QString widestName = QString::fromLatin1(Style::widestName);
    const int nameW = qMax(Style::minBoxSize, qCeil(qMax(nameFm.horizontalAdvance(widestName), nameFm.horizontalAdvance(channelName))) + 2 * (border + Style::nameHPadding));
    const int nameH = qBound(Style::minBoxSize, qCeil(nameFm.height()) + 2 * border, Style::maxBoxSize);

    int badgeW = Style::minBoxSize;
    for (const Style::Badge &badge : Style::badges) {
        const int w = qCeil(badgeFm.horizontalAdvance(QLatin1Char(badge.letter))) + 2 * Style::badgeHPadding;
        badgeW = qMax(badgeW, qBound(Style::minBoxSize, w, Style::maxBoxSize));
    }
    const int badgeH = qBound(Style::minBoxSize, qCeil(badgeFm.height()), Style::maxBoxSize);

    const int canvasW = 2 * Style::margin + qMax(nameW + Style::badgeSpacing + Style::colorSize,
                                                 Style::minVisibleBadges * badgeW + (Style::minVisibleBadges - 1) * Style::badgeSpacing);
    const int canvasH = 2 * Style::margin + nameH + Style::badgeSpacing + badgeH;

    // Space between card and canvas: QSS border (up to 2 px when selected) + canvas inset
    const int frame = 2 * (2 + Style::canvasInset);

    return QSize(qMax(STAMP_PLOT_MIN_WIDTH, canvasW + frame), qMax(STAMP_PLOT_MIN_HEIGHT, canvasH + frame));
}

void StampPlot::setSelected(bool flag) {
    if (flag != selected) {
        selected = flag;

        this->setProperty("status", selected ? "selected" : "default");

        this->style()->unpolish(this);
        this->style()->polish(this);
        // Canvas background is based on card status (QSS rule)
        this->canvas()->style()->unpolish(this->canvas());
        this->canvas()->style()->polish(this->canvas());

        // Queued replot: with ALL/INVERT on 384 cards this avoids 384 synchronous
        // redraws; each card is redrawn once, after the selection loop
        this->requestReplot();
    }
}

void StampPlot::mousePressEvent(QMouseEvent *event){
    emit clicked(event);
    QwtPlot::mousePressEvent(event);
}

void StampPlot::setState(States_t newState) {
    if (newState == state) {
        return;
    }
    state = newState;
    requestReplot();
}

void StampPlot::addState(States_t newState) {
    this->setState((States_t)(state | newState));
}

void StampPlot::removeState(States_t newState) {
    this->setState((States_t)(state &~ newState));
}

void StampPlot::onRangeUpdated(RangedMeasurement_t newRange) {
    Axis axisIdx = QwtPlot::yLeft;
    currentRange[axisIdx] = newRange;
    this->setAxisScale(axisIdx, currentRange[axisIdx].min, currentRange[axisIdx].max);
    this->replot();
}

void StampPlot::onDurationUpdated(Measurement_t duration) {
    sweepDuration = duration;
    sweepDuration.convertValue(UnitPfxNone);
    this->setAxisScale(xBottom, 0.0, sweepDuration.value);

    this->replot();
}

void StampPlot::setLegendColor(QColor color) {
    if (color == legendColor) {
        return;
    }
    legendColor = color;
    requestReplot();
}

void StampPlot::setName(std::string name){
    const QString newName = QString::fromStdString(name);
    if (newName == channelName) {
        return;
    }
    channelName = newName;
    nameCache = QPixmap();
    this->updateGeometry(); // min size depends on name
    requestReplot();
}

void StampPlot::drawCanvas(QPainter * p) {
    drawGrid(p);
    QwtPlot::drawCanvas(p);
    drawOverlays(p);
}

// True if at least part of the widget is inside all its ancestors (e.g. not
// scrolled out of the chessboard scroll area) and the window is not minimized.
// Cheaper than QWidget::visibleRegion(), which also subtracts all the siblings.
static bool isOnScreen(const QWidget *w) {
    if (!w->isVisible() || w->window()->isMinimized()) {
        return false;
    }
    QRect r = w->rect();
    const QWidget *current = w;
    while (!current->isWindow() && current->parentWidget() != nullptr) {
        r.translate(current->pos());
        current = current->parentWidget();
        r &= current->rect();
        if (r.isEmpty()) {
            return false;
        }
    }
    return true;
}

// Replot only the cards that are on screen, the others are marked dirty
void StampPlot::replotData() {
    if (isOnScreen(this->canvas())) {
        dataDirty = false;
        this->replot();
    } else {
        dataDirty = true;
    }
}

bool StampPlot::takeDataDirty() {
    const bool wasDirty = dataDirty;
    dataDirty = false;
    return wasDirty;
}

bool StampPlot::isSelected(){
    return this->selected;
}

void StampPlot::changeEvent(QEvent *event) {
    // The font comes from the QSS (QWidget { font-family ... }): when it changes,
    // the name box must be rendered again
    if (event->type() == QEvent::FontChange) {
        invalidateOverlayCache();
        this->updateGeometry(); // font based size
    }
    QwtPlot::changeEvent(event);
}

void StampPlot::invalidateOverlayCache() {
    nameCache = QPixmap();
}

// Mini-plot curve color from the theme (StampOverlayStyle::curveColor)
void StampPlot::applyThemeToCurves() {
    const QColor color = paletteColor(Style::curveColor);
    if (!color.isValid()) {
        return;
    }
    const QwtPlotItemList curves = this->itemList(QwtPlotItem::Rtti_PlotCurve);
    for (QwtPlotItem *item : curves) {
        Curve *curve = dynamic_cast<Curve *>(item);
        if (curve != nullptr && curve->getCurveType() == CurveTypeStampPlotSolid) {
            curve->setColor(color);
        }
    }
}

// Coalesces the changes (e.g. ALL on 384 channels, or state + color) into one
// queued replot per card
void StampPlot::requestReplot() {
    if (replotPending) {
        return;
    }
    replotPending = true;
    QMetaObject::invokeMethod(this, [this]() {
        replotPending = false;
        this->replot();
    }, Qt::QueuedConnection);
}

const QPixmap &StampPlot::namePixmap(qreal dpr) {
    if (nameCache.isNull() || !qFuzzyCompare(nameCache.devicePixelRatio(), dpr)) {
        const QFont font = overlayFont(this->font(), Style::nameFontPx);
        const QFontMetricsF fm(font);
        const int textW = qCeil(fm.horizontalAdvance(channelName));
        const int border = qCeil(Style::nameBorderWidth);
        const int w = qMax(Style::minBoxSize, textW + 2 * (border + Style::nameHPadding));
        const int h = qBound(Style::minBoxSize, qCeil(fm.height()) + 2 * border, Style::maxBoxSize);
        nameCache = renderTextBox(channelName, font, QSize(w, h), Style::nameRadius,
                                  paletteColor(Style::nameBackground),
                                  paletteColor(Style::nameText),
                                  paletteColor(Style::nameBorder),
                                  Style::nameBorderWidth, dpr);
    }
    return nameCache;
}

// Simple background grid for mini-plots - theme aware
void StampPlot::drawGrid(QPainter *p) {
    const QColor color = paletteColor(Style::gridColor);
    if (!color.isValid()) {
        return;
    }
    const QRect area = this->canvas()->contentsRect().adjusted(Style::margin, Style::margin, -Style::margin, -Style::margin);
    if (area.width() <= 2 * Style::gridInset || area.height() <= 2 * Style::gridInset) {
        return;
    }

    p->save();
    QPen pen(color, 1.0);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->setRenderHint(QPainter::Antialiasing, false);

    // Integer coordinates: sharp 1 px lines, no antialiasing
    const int left = area.left() + Style::gridInset;
    const int right = area.right() - Style::gridInset;
    const int top = area.top() + Style::gridInset;
    const int bottom = area.bottom() - Style::gridInset;
    for (int i = 1; i < Style::gridColumns; i++) {
        const int x = area.left() + qRound(area.width() * i / double(Style::gridColumns));
        p->drawLine(x, top, x, bottom);
    }
    for (int i = 1; i < Style::gridRows; i++) {
        const int y = area.top() + qRound(area.height() * i / double(Style::gridRows));
        p->drawLine(left, y, right, y);
    }
    p->restore();
}

void StampPlot::drawOverlays(QPainter *p) {
    const QRect area = this->canvas()->contentsRect().adjusted(Style::margin, Style::margin, -Style::margin, -Style::margin);
    if (area.width() <= 0 || area.height() <= 0) {
        return;
    }
    const qreal dpr = this->canvas()->devicePixelRatioF();

    p->save();

    // TOP LEFT: Channel name
    if (!channelName.isEmpty()) {
        p->drawPixmap(area.topLeft(), namePixmap(dpr));
    }

    // BOTTOM LEFT: status badges
    if (state != StateNoEffects) {
        const QFont base = this->font();
        int x = area.left();
        const int bottom = area.bottom() + 1;
        for (const Style::Badge &badge : Style::badges) {
            if (!(state & badge.state)) {
                continue;
            }
            const QPixmap pm = badgePixmap(badge, base, dpr);
            const QSize size = pm.deviceIndependentSize().toSize();
            p->drawPixmap(x, bottom - size.height(), pm);
            x += size.width() + Style::badgeSpacing;
        }
    }

    // TOP RIGHT: channel color
    {
        const qreal h = Style::colorBorderWidth / 2.0;
        const QRectF colorRect(area.right() + 1 - Style::colorSize, area.top(), Style::colorSize, Style::colorSize);
        p->setRenderHint(QPainter::Antialiasing, true);
        p->setPen(QPen(paletteColor(Style::colorBorder), Style::colorBorderWidth));
        p->setBrush(legendColor);
        p->drawRoundedRect(colorRect.adjusted(h, h, -h, -h), Style::colorRadius, Style::colorRadius);
    }

    p->restore();
}
