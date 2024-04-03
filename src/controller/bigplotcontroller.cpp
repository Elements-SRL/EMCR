#include "bigplotcontroller.h"
#include <iostream>

BigPlotController::BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotDuration, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    bpw = new BigPlotWidget(mainWindow);
    bpm = new BigPlotModel();

    mainWindow->setBigPlotWidget(bpw);

    auto ivGraphConsumer = new IvGraphConsumer(appStatus, producer);
    auto gapFreePlotConsumer = new GapFreePlotConsumer(appStatus, producer);
    gapFreePlotConsumer->onDurationChanged(defaultPlotDuration);

    auto gapFreePlot = new BigPlot("", "[s]", "", bpw);
    gapFreePlot->enableAxis(QwtPlot::yRight);
    bpw->setGapFreePlot(gapFreePlot);
    connect(bpw, &BigPlotWidget::tabBarClicked, this, &BigPlotController::manageStatus);
    auto ivGraph = new BigPlot("", "[V]", "", bpw);
    bpw->setIvGraph(ivGraph);
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    voltageChannelsNum = appStatus->getVoltageChannelsNum();

    for (int i = 0; i < currentChannelsNum; i++){
        currentCurves.append(new Curve(CurveType_t::CurveTypePlotSolid));
    }

    for (int i = 0; i < voltageChannelsNum; i++){
        voltageCurves.append(new Curve(CurveType_t::CurveTypePlotDashed));
        voltageCurves[i]->setColor(QColor(Qt::red));
        voltageCurves[i]->setYAxis(QwtPlot::yRight);
    }

    consumers = {gapFreePlotConsumer, ivGraphConsumer};
    plots = {gapFreePlot, ivGraph};
    for(int i=0; i < consumers.size(); i++){
        consumers[i]->forceAxisUpdate();
        consumers[i]->setMaxSamplesPerPlot(4096);
        consumers[i]->onSelectChannels(false);
        consumers[i]->onStopConsuming();
        connect(plots[i], &BigPlot::zoomInRequest, this, &BigPlotController::handleZoomInRequest);
        connect(plots[i], &BigPlot::zoomOutRequest, this, &BigPlotController::handleZoomOutRequest);
        connect(plots[i], &BigPlot::zoomResetRequest, this, &BigPlotController::handleZoomResetRequest);
        connect(plots[i], &BigPlot::singleAxisZoomRequest, this, &BigPlotController::handleSingleAxisZoomRequest);
        connect(plots[i], &BigPlot::singleAxisShiftRequest, this, &BigPlotController::handleSingleAxisShiftRequest);

        connect(this, &BigPlotController::durationChanged, consumers[i], &PlotConsumer::onDurationChanged);
        connect(consumers[i], &PlotConsumer::setPlotData,         this, &BigPlotController::onSetPlotData);
        connect(consumers[i], &PlotConsumer::plotDataUpdated,     this, &BigPlotController::onReplot);
    }
    currentPlot = gapFreePlot;
    currentConsumer = gapFreePlotConsumer;
    bps = BigPlotStatus::GapFree;
}

void BigPlotController::manageStatus(int idx) {
    int  bps_idx = bps;
     if (idx == bps_idx) {
         return;
     }
     currentConsumer->onStopConsuming();
     currentPlot = plots[idx];
     currentConsumer = consumers[idx];
     currentConsumer->onStartConsuming();
}

BigPlotController::~BigPlotController() {
    this->clearCurves();

    for(int i=0; i < plots.size(); i++) {
        delete plots[i];
        plots[i] = nullptr;
    }
    for(int i=0; i < consumers.size(); i++) {
        delete consumers[i];
        consumers[i] = nullptr;
    }

    if (bpw != nullptr) {
        delete bpw;
        bpw = nullptr;
        mainWindow->setBigPlotWidget(bpw);
    }

    if (bpm != nullptr) {
        delete bpm;
        bpm = nullptr;
    }
}

BigPlot * BigPlotController::getPlot(){
    return currentPlot;
}

void BigPlotController::clearCurves() {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->detach();
        delete currentCurves[idx];
        delete [] currentCurves[idx];
    }

    for (int idx = 0; idx < voltageChannelsNum; idx++) {
        voltageCurves[idx]->detach();
        delete voltageCurves[idx];
        delete [] voltageCurves[idx];
    }

    currentCurves.clear();
    voltageCurves.clear();
}

bool BigPlotController::isAtLeastOneChannelExpanded() {
    auto channels = appStatus->getChannels();
    for(auto c: channels){
        if(c->isExpanded()){
            return true;
        }
    }
    return false;
}

void BigPlotController::onExpandTrace(bool flag){
    currentConsumer->onStopConsuming();
    currentConsumer->onSelectChannels(flag);
    if (isAtLeastOneChannelExpanded()) {
        currentConsumer->onStartConsuming();
    }
}

void BigPlotController::onSetPlotData(PlotMessage plotmessage) {
    auto channels = appStatus->getChannels();
    IvMessage ivMessage;
    GapFreeMessage gapFreeMessage;
    RangedMeasurement v;
    RangedMeasurement i;
    Rect4 r;
    switch (plotmessage.index()) {
//    IvGraph message
    case 0:
        currentPlot->setStatus(BigPlotStatus::Iv);
        appStatus->getMessageDispatcher()->getVCVoltageRange(v);
        appStatus->getMessageDispatcher()->getVCCurrentRange(i);
        r = bpm->initRect(v.min, v.max, i.min, i.max);
        plots[1]->setRect(r);
//        ivGra ->setRect(r);
        ivMessage = std::get<0>(plotmessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            if (channels[idx]->isExpanded()) {
//                double * voltages = ivMessage.voltageValues.data();
                currentCurves.at(idx)->attach(currentPlot);
                voltageCurves.at(idx)->detach();
                currentCurves.at(idx)->setRawSamples(ivMessage.voltageValues, ivMessage.currentValues[idx], ivMessage.dataSize);
                currentCurves.at(idx)->setStyle(QwtPlotCurve::NoCurve);
                currentCurves.at(idx)->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::blue, Qt::NoPen, QSize(5, 5)));
            } else {
                currentCurves.at(idx)->detach();
                voltageCurves.at(idx)->detach();
            }
        }
        break;
//    GapFree message
    case 1:
        currentPlot->setStatus(BigPlotStatus::GapFree);
        gapFreeMessage = std::get<1>(plotmessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            if (channels[idx]->isExpanded()) {
                currentCurves.at(idx)->attach(currentPlot);
                currentCurves.at(idx)->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.currentValues->at(idx), gapFreeMessage.dataSize);

                voltageCurves.at(idx)->attach(currentPlot);
                voltageCurves.at(idx)->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.voltageValues->at(idx), gapFreeMessage.dataSize);

            } else {
                currentCurves.at(idx)->detach();
                voltageCurves.at(idx)->detach();
            }
        }
        break;
    }
}

void BigPlotController::onReplot() {
//    todo CHECK INTERNAL STATUS
    if (currentPlot != nullptr) {
        currentPlot->replot();
    }
}

void BigPlotController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setColor(colors[idx]);
        voltageCurves[idx]->setColor(colors[idx]);
    }
}

void BigPlotController::onCurrentColorChanged(int channelIdx, QColor color) {
    currentCurves[channelIdx]->setColor(color);
    voltageCurves[channelIdx]->setColor(color);
}

void BigPlotController::onBackgroundColorChanged(QColor color) {
    currentPlot->setCanvasBackground(color);
}

void BigPlotController::handleZoomInRequest(Rect4 r){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(r);
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    currentPlot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, int zoomIn, QPointF mousePosition){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(bpm->zoomOnSingleAxis(axis, zoomIn, mousePosition));
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    currentPlot->setRect(zoom);
    if (axis == QwtPlot::Axis::xBottom){
        emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    }
}

void BigPlotController::handleSingleAxisShiftRequest(QwtPlot::Axis axis, int shift){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(bpm->shiftOnSingleAxis(axis, shift));
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    currentPlot->setRect(zoom);
}

void BigPlotController::handleZoomOutRequest(){
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Previous);
    currentPlot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::handleZoomResetRequest(){
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Default);
    currentPlot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::onRangeUpdated(commlib::RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    bpm->setCurrentRange(axisIdx, newRange);
    currentPlot->setRect(bpm->getZoom(BigPlotModel::Zoom::Current));
    auto fullUnit = QString::fromStdString(bpm->getCurrentRange(axisIdx).getFullUnit());
    switch (axisIdx) {
    case QwtPlot::yLeft:
        currentPlot->setLabel(fullUnit, axisIdx);
        break;

    case QwtPlot::yRight:
        currentPlot->setLabel(fullUnit, axisIdx);
        break;

    case QwtPlot::xBottom:
        currentPlot->setLabel(fullUnit, axisIdx);
        Measurement_t duration = {bpm->getZoom(BigPlotModel::Zoom::Current)[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"};
        emit durationChanged(duration);
        break;
    }
    currentPlot->replot();
}

std::vector<PlotConsumer *> BigPlotController::getConsumers(){
    return consumers;
}
