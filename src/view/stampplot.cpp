#include "stampplot.h"

#include "qboxlayout.h"
#include "qlabel.h"
#include "qstyle.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_canvas.h"
#include <QApplication>
#include "globaldefines.h"
#include <qwt_symbol.h>
#include <qwt_text.h>
#include <QFrame>

using namespace e384CommLib;

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

    QwtPlotCanvas * canvas = new QwtPlotCanvas();
    canvas->setFrameStyle(QFrame::NoFrame);
    canvas->setObjectName("stampPlotCanvas");
    this->setCanvas(canvas);
    this->setObjectName("stampPlotCard");
    this->setProperty("status", "default");

    QGridLayout *overlayLayout = new QGridLayout(canvas);
    overlayLayout->setContentsMargins(2, 2, 2, 2);
    overlayLayout->setSpacing(0);
    canvas->setLayout(overlayLayout);

    QwtText text;
    text.setRenderFlags(Qt::TextDontClip | Qt::TextSingleLine | Qt::AlignCenter);
    text.setPaintAttribute(QwtText::PaintUsingTextColor, true);

    channelIdxLbl = new QwtTextLabel(this);
    channelIdxLbl->setObjectName("stampChannelIdx");
    channelIdxLbl->setMinimumSize(10, 10);
    channelIdxLbl->setMaximumSize(26, 15);
    text.setText(QString("%1").fromStdString(channelname));
    channelIdxLbl->setText(text);

    // TOP LEFT: Channel name
    overlayLayout->addWidget(channelIdxLbl, 0, 0, Qt::AlignTop | Qt::AlignLeft);

    badgeContainer = new QWidget(this->canvas());
    badgeContainer->setObjectName("badgeContainer");

    // BOTTOM LEFT: Badges (E,X,P etc)
    overlayLayout->addWidget(badgeContainer, 1, 0, 1, 2, Qt::AlignBottom | Qt::AlignLeft);

    QHBoxLayout *badgeLayout = new QHBoxLayout(badgeContainer);
    badgeLayout->setContentsMargins(0, 0, 0, 0);
    badgeLayout->setSpacing(4);
    badgeLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    std::vector<QString> statuses = {"O", "R", "X", "C", "J", "E", "P"};
    for (const auto& key : statuses) {
        QLabel* badge = new QLabel(key, this);
        badge->setAlignment(Qt::AlignCenter);
        badge->setObjectName("propertyBadgeChess");
        badge->setAutoFillBackground(true);
        badge->setMinimumSize(10, 10);
        badge->setMaximumSize(15, 15);
        badge->setProperty("propertyValue", key);
        badge->setVisible(false);
        badgeLayout->addWidget(badge);
        m_badgeMap[key] = badge;
    }
    badgeContainer->raise();

    xAxisMaxMajor = this->axisMaxMajor(xBottom);
    yAxisMaxMajor = this->axisMaxMajor(yLeft);

    selected = false;
    colorLabel = new QFrame(this);
    colorLabel->setObjectName("colorChannel");
    colorLabel->setGeometry(this->canvas()->x()+this->canvas()->width()-SMP_LEGEND_SIZE, this->canvas()->y(), SMP_LEGEND_SIZE, SMP_LEGEND_SIZE);
    colorLabel->setMinimumSize(10, 10);
    colorLabel->setMaximumSize(15, 15);

    // TOP RIGHT: Color badge
    overlayLayout->addWidget(colorLabel, 0, 1, Qt::AlignTop | Qt::AlignRight);
    colorLabel->raise();
}

QSize StampPlot::sizeHint() const {
    return QSize(idealPlotWidth, idealPlotHeight);
}

QSize StampPlot::minimumSizeHint() const {
    return QSize(STAMP_PLOT_MIN_WIDTH, STAMP_PLOT_MIN_HEIGHT);
}

void StampPlot::setSelected(bool flag) {
    if (flag != selected) {
        selected = flag;

        this->setProperty("status", selected ? "selected" : "default");
        this->handleLabelsPosition();

        this->style()->unpolish(this);
        this->style()->polish(this);

        this->replot();
    }
}

void StampPlot::mousePressEvent(QMouseEvent *event){
    emit clicked(event);
    QwtPlot::mousePressEvent(event);
}

void StampPlot::setState(States_t newState) {
    state = newState;

    m_badgeMap["O"]->setVisible(state & StateSwitchedOff);
    m_badgeMap["R"]->setVisible(state & StateCalibrationResistorsOn);
    m_badgeMap["X"]->setVisible(state & StateStimuliDisabled);
    m_badgeMap["C"]->setVisible(state & StateOffsetRecalibrationOn);
    m_badgeMap["J"]->setVisible(state & StateLiquidJunctionCompensation);
    m_badgeMap["E"]->setVisible(state & StateTraceExpanded);
    m_badgeMap["P"]->setVisible(state & StatePlotDetailOn);

    badgeContainer->adjustSize();
    this->resizeEvent(nullptr);
    this->style()->unpolish(badgeContainer);
    this->style()->polish(badgeContainer);
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

void StampPlot::resizeEvent(QResizeEvent * e) {
    if (e != nullptr) {
        QwtPlot::resizeEvent(e);
    }

    this->handleLabelsPosition();
}

void StampPlot::setLegendColor(QColor color) {
    colorLabel->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
}

void StampPlot::setName(std::string name){
    QwtText text;
    text.setRenderFlags(Qt::AlignCenter | Qt::TextDontClip | Qt::TextSingleLine);
    text.setText(QString("%1").fromStdString(name));
    channelIdxLbl->setText(text);
    channelIdxLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

void StampPlot::drawCanvas(QPainter * p) {
    QwtPlot::drawCanvas(p);
    this->handleLabelsPosition();
}

bool StampPlot::isSelected(){
    return this->selected;
}

void StampPlot::handleLabelsPosition() {
    if (badgeContainer->layout()) {
        badgeContainer->layout()->invalidate();
    }
}

