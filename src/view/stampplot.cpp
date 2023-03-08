#include "stampplot.h"

#include "qwt_plot_canvas.h"

StampPlot::StampPlot(QWidget * parent) :
    QwtPlot(parent) {

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
    return QSize(30, 30);
}

QSize StampPlot::minimumSizeHint() const {
    return QSize(30, 30);
}
