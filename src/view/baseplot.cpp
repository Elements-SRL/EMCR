#include "baseplot.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_grid.h"
#include "qwt_scale_widget.h"
#include "themecontroller.h"

BasePlot::BasePlot(std::string title, std::string xUnit, std::string yUnit, QWidget* parent)
    : QwtPlot(parent) {
    this->plotLayout()->setAlignCanvasToScales(true);

    for (int axis = 0; axis < axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
    }
    
    QwtPlotCanvas* canvas = new QwtPlotCanvas();
    canvas->setFrameStyle(QFrame::NoFrame);
    this->setCanvas(canvas);

    QwtPlotGrid* grid = new QwtPlotGrid();
    // Grid color from the theme (@PLOT_GRID), updated when the theme changes.
    // The replot is queued so it runs after the other slots of the same signal
    // have updated the curve colors too.
    auto applyGridColor = [grid]() {
        QColor gridColor = ThemeController::color(QStringLiteral("@PLOT_GRID"));
        if (!gridColor.isValid()) {
            gridColor = QColor(Qt::lightGray);
        }
        grid->setPen(gridColor, 0.5);
        grid->setMinorPen(gridColor, 0.3);
    };
    applyGridColor();
    connect(&ThemeController::getInstance(), &ThemeController::sigThemeUpdated, this, [this, applyGridColor]() {
        applyGridColor();
        this->replot();
    }, Qt::QueuedConnection);
    grid->enableXMin(true);
    grid->attach(this);
    grid->show();

    this->title = produceTextLabel(title);
    this->xUnit = produceTextLabel(xUnit);
    this->yUnit = produceTextLabel(yUnit);
}

void BasePlot::drawCanvas(QPainter* p) {
    QwtPlot::drawCanvas(p);
    this->handleLabelsPosition();
}

void BasePlot::resizeEvent(QResizeEvent* e) {
    QwtPlot::resizeEvent(e);
    this->handleLabelsPosition();
}

void BasePlot::setLabel(std::string text, QwtPlot::Axis axis) {
    switch (axis) {
    case QwtPlot::Axis::xBottom:
        setAndFormatText(text, xUnit);
        break;
    case QwtPlot::Axis::yLeft:
        setAndFormatText(text, yUnit);
        break;
    case QwtPlot::Axis::yRight:
        setAndFormatText(text, title, Qt::AlignRight);
        break;
    }
}

void BasePlot::setAndFormatText(std::string text, QwtTextLabel* label, Qt::AlignmentFlag alignment) {
    label->setPlainText("[" + QString::fromStdString(text) + "]");
    QwtText t = label->text();
    t.setRenderFlags(alignment | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    label->setText(t);
}

QwtTextLabel* BasePlot::produceTextLabel(std::string text, Qt::AlignmentFlag alignment) {
    QFont font;
    font.setPointSize(10);
    const auto label = new QwtTextLabel(this);
    QwtText t = label->text();
    t.setRenderFlags(alignment | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    t.setText("[" + QString::fromStdString(text) + "]");
    label->setText(t);
    label->setFont(font);
    label->setMargin(0);
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    return label;
}

void BasePlot::handleLabelsPosition() {
    const auto x = this->canvas()->x();
    const auto y = this->canvas()->y();
    const auto w = this->canvas()->width();
    QSize siz = title->minimumSizeHint();
    title->setGeometry(x + w - siz.width(), y, siz.width(), siz.height());

    siz = xUnit->minimumSizeHint();
    xUnit->setGeometry(x + w - siz.width(), y + this->canvas()->height() - siz.height(), siz.width(), siz.height());

    siz = yUnit->minimumSizeHint();
    yUnit->setGeometry(x, y, siz.width(), siz.height());
}

QSize BasePlot::sizeHint() const {
    return QSize(200, 300);
}

QSize BasePlot::minimumSizeHint() const {
    return QSize(200, 300);
}

BasePlot* BasePlot::getPlot() {
    return this;
}
