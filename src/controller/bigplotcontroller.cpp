#include "bigplotcontroller.h"

BigPlotController::BigPlotController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {
    bpw = new BigPlotWidget(msgDisp);
    bpm = new BigPlotModel();

    mainWindow->setBigPlotWidget(bpw);
    plot = bpw->getPlot();
    connect(plot, &BigPlot::zoomInRequest, this, &BigPlotController::handleZoomInRequest);
    connect(plot, &BigPlot::zoomOutRequest, this, &BigPlotController::handleZoomOutRequest);
    connect(plot, &BigPlot::zoomResetRequest, this, &BigPlotController::handleZoomResetRequest);
}

void BigPlotController::handleZoomInRequest(Rect4 r){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(r);
    plot->setRect(bpm->getZoom(BigPlotModel::Zoom::Current));
}

void BigPlotController::handleZoomOutRequest(){
    plot->setRect(bpm->getZoom(BigPlotModel::Zoom::Previous));
}

void BigPlotController::handleZoomResetRequest(){
    plot->setRect(bpm->getZoom(BigPlotModel::Zoom::Default));
}

void BigPlotController::onRangeUpdated(commlib::RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    bpm->setCurrentRange(axisIdx, newRange);
    auto interval = bpm->getCurrentZoomInterval(axisIdx);
    plot->setAxisScale(axisIdx, interval.minValue(), interval.maxValue());
    auto fullUnit = QString::fromStdString(bpm->getCurrentRange(axisIdx).getFullUnit());
    if (axisIdx == QwtPlot::yLeft) {
        plot->setYUnit(fullUnit);
    } else {
        plot->setTitle(fullUnit);
    }
    plot->replot();
}
