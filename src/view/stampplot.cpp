#include "stampplot.h"

#include "qwt_plot_layout.h"
#include "qwt_plot_canvas.h"

#include "globaldefines.h"

StampPlot::StampPlot(QWidget * parent) :
    QwtPlot(parent) {

    this->plotLayout()->setAlignCanvasToScales(true);

    for (int axis = 0; axis < axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
        this->enableAxis(axis, false);
    }

    QwtPlotCanvas * canvas = new QwtPlotCanvas();
    canvas->setFrameStyle(QFrame::NoFrame);
    this->setCanvas(canvas);
    this->setCanvasBackground(Qt::black);

    /*! select picker */
    selectPicker = new QwtPlotPicker(this->canvas());
    selectPicker->setStateMachine(new QwtPickerClickPointMachine());
    selectPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    selectPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton);
    connect(selectPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &StampPlot::onSelected);

    /*! deselect picker */
    deselectPicker = new QwtPlotPicker(this->canvas());
    deselectPicker->setStateMachine(new QwtPickerClickPointMachine());
    deselectPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    deselectPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(deselectPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &StampPlot::onDeselected);
}

void StampPlot::onSelected() {
    emit selected(true);
}

void StampPlot::onDeselected() {
    emit selected(false);
}

QSize StampPlot::sizeHint() const {
    return QSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
}

QSize StampPlot::minimumSizeHint() const {
    return QSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
}

void StampPlot::initializeRange(RangedMeasurement_t newRange, Axis axisIdx) {
    currentRange[axisIdx] = newRange;
    this->setAxisScale(axisIdx, currentRange[axisIdx].min, currentRange[axisIdx].max);
}

void StampPlot::onRangeUpdated(RangedMeasurement_t newRange, Axis axisIdx) {
    if (newRange != currentRange[axisIdx]) {
        currentRange[axisIdx].max = 1.0;
        currentRange[axisIdx].convertValues(newRange.prefix);
        double coeff = currentRange[axisIdx].max;
        currentRange[axisIdx].max = newRange.max;
        currentRange[axisIdx].min = newRange.min;

        double min = coeff*this->axisInterval(axisIdx).minValue();
        double max = coeff*this->axisInterval(axisIdx).maxValue();

        this->setAxisScale(axisIdx, min, max);
    }
    this->replot();
}

void StampPlot::onDurationUpdated(Measurement_t duration) {
    sweepDuration = duration;
    sweepDuration.convertValue(UnitPfxNone);
    this->setAxisScale(xBottom, 0.0, sweepDuration.value);

    this->replot();
}
