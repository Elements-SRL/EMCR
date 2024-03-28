#include "bigplotcontroller.h"
#include <iostream>

BigPlotController::BigPlotController(ApplicationStatus * appStatus, PlotConsumer * plotConsumer, Measurement_t defaultPlotDuration, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow),
    plotConsumer(plotConsumer) {

    bpw = new BigPlotWidget(mainWindow);
    bpm = new BigPlotModel();

    mainWindow->setBigPlotWidget(bpw);
    plot = new BigPlot("", "[s]", "", bpw);
    plot->enableAxis(QwtPlot::yRight);
    bpw->setPlot(plot);
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

    plotConsumer->onDurationChanged(defaultPlotDuration);
    plotConsumer->forceAxisUpdate();
    plotConsumer->setMaxSamplesPerPlot(4096);
    plotConsumer->onSelectChannels(false);
    connect(plot, &BigPlot::zoomInRequest, this, &BigPlotController::handleZoomInRequest);
    connect(plot, &BigPlot::zoomOutRequest, this, &BigPlotController::handleZoomOutRequest);
    connect(plot, &BigPlot::zoomResetRequest, this, &BigPlotController::handleZoomResetRequest);
    connect(plot, &BigPlot::singleAxisZoomRequest, this, &BigPlotController::handleSingleAxisZoomRequest);
    connect(plot, &BigPlot::singleAxisShiftRequest, this, &BigPlotController::handleSingleAxisShiftRequest);

    connect(this, &BigPlotController::durationChanged, this->plotConsumer, &PlotConsumer::onDurationChanged);
    connect(plotConsumer, &PlotConsumer::setPlotData,         this, &BigPlotController::onSetPlotData);
    connect(plotConsumer, &PlotConsumer::plotDataUpdated,     this, &BigPlotController::onReplot);

}

PlotConsumer * BigPlotController::getPlotConsumer(){
    return plotConsumer;
}

BigPlotController::~BigPlotController() {
    this->clearCurves();
    if (plot != nullptr) {
        delete plot;
        plot = nullptr;
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
    if (plotConsumer!= nullptr) {
        plotConsumer->onStopConsuming();
        delete plotConsumer;
        plotConsumer = nullptr;
    }
}

BigPlot * BigPlotController::getPlot(){
    return plot;
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
    plotConsumer->onSelectChannels(flag);
    if (isAtLeastOneChannelExpanded()) {
        plotConsumer->onStartConsuming();
    } else {
        plotConsumer->onStopConsuming();
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
        plot->setStatus(BigPlotStatus::Iv);
        appStatus->getMessageDispatcher()->getVCVoltageRange(v);
        appStatus->getMessageDispatcher()->getVCCurrentRange(i);
        r = bpm->initRect(v.min, v.max, i.min, i.max);
        plot->setRect(r);
        ivMessage = std::get<0>(plotmessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            if (channels[idx]->isExpanded()) {
                double * voltages = ivMessage.voltageValues.data();
                currentCurves.at(idx)->attach(plot);
                voltageCurves.at(idx)->detach();
                currentCurves.at(idx)->setRawSamples(voltages, ivMessage.currentValues[idx], ivMessage.dataSize);
                std::cout<<"currents " << std::endl;
                for (int i=0; i<ivMessage.dataSize; i++){
                    std::cout<<" " <<ivMessage.currentValues[idx][i];
                }
                std::cout<< std::endl;
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
        plot->setStatus(BigPlotStatus::GapFree);
        gapFreeMessage = std::get<1>(plotmessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            if (channels[idx]->isExpanded()) {
                currentCurves.at(idx)->attach(plot);
                currentCurves.at(idx)->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.currentValues->at(idx), gapFreeMessage.dataSize);

                voltageCurves.at(idx)->attach(plot);
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
    if (plot != nullptr) {
        plot->replot();
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
    plot->setCanvasBackground(color);
}

void BigPlotController::handleZoomInRequest(Rect4 r){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(r);
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, int zoomIn, QPointF mousePosition){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(bpm->zoomOnSingleAxis(axis, zoomIn, mousePosition));
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    if (axis == QwtPlot::Axis::xBottom){
        emit durationChanged({zoom[QwtPlot::xBottom].width(), bpm->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    }
}

void BigPlotController::handleSingleAxisShiftRequest(QwtPlot::Axis axis, int shift){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    bpm->updateCurrentZoom(bpm->shiftOnSingleAxis(axis, shift));
    auto zoom = bpm->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
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
