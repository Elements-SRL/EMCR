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
    connect(plot, &BigPlot::singleAxisZoomRequest, this, &BigPlotController::handleSingleAxisZoomRequest);
}

void BigPlotController::handleZoomInRequest(Rect4 r){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(r);
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, int zoomIn){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(bpm->zoomOnSingleAxis(axis, zoomIn));
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    if (axis == QwtPlot::Axis::xBottom){
        emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    }
}

void BigPlotController::handleZoomOutRequest(){
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Previous);
    plot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::handleZoomResetRequest(){
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Default);
    plot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::onRangeUpdated(commlib::RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    bpm->setCurrentRange(axisIdx, newRange);
    plot->setRect(bpm->getZoom(BigPlotModel::Zoom::Current));
    auto fullUnit = QString::fromStdString(bpm->getCurrentRange(axisIdx).getFullUnit());
    switch (axisIdx) {
    case QwtPlot::yLeft:
        plot->setLabel(fullUnit, axisIdx);
        break;

    case QwtPlot::yRight:
        plot->setLabel(fullUnit, axisIdx);
        break;

    case QwtPlot::xBottom:
        plot->setLabel(fullUnit, axisIdx);
        Measurement_t duration = {bpm->getZoom(BigPlotModel::Zoom::Current)[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"};
        emit durationChanged(duration);
        break;
    }
    plot->replot();
}
