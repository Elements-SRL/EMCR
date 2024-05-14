#include "gapfreecontroller.h"

GapFreeController::GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController):
    CentralWidgetController(appStatus, producer, bigPlotWidget) {

    model = new BigPlotModel();
    consumer = new GapFreePlotConsumer(appStatus, producer);
    consumer->onDurationChanged(defaultPlotDuration);

    plot = new BigPlot("", "[s]", "", BigPlotStatus::GapFree, bigPlotWidget);
    plot->enableAxis(QwtPlot::yRight);
    bigPlotWidget->setGapFreePlot(plot);
    //    creating curves for gapfree
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
    }
    for (int i = 0; i < voltageChannelsNum; i++) {
        voltageCurves.push_back(new Curve(CurveType_t::CurveTypePlotDashed));
        voltageCurves[i]->setYAxis(QwtPlot::yRight);
    }

    connect(plot, &BigPlot::zoomInRequest, bigPlotController, &BigPlotController::handleZoomInRequest);
    connect(plot, &BigPlot::zoomOutRequest, bigPlotController, &BigPlotController::handleZoomOutRequest);
    connect(plot, &BigPlot::zoomResetRequest, bigPlotController, &BigPlotController::handleZoomResetRequest);
    connect(plot, &BigPlot::singleAxisZoomRequest, bigPlotController, &BigPlotController::handleSingleAxisZoomRequest);
    connect(plot, &BigPlot::singleAxisShiftRequest, bigPlotController, &BigPlotController::handleSingleAxisShiftRequest);

    connect(bigPlotController, &BigPlotController::durationChanged, consumer, &PlotConsumer::onDurationChanged);
    connect(consumer, &PlotConsumer::setPlotData, this, &GapFreeController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &GapFreeController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    consumer->onSelectChannels(false);
    consumer->onStopConsuming();
}

GapFreeController::~GapFreeController() {
    if (model != nullptr) {
        delete model;
        model = nullptr;
    }
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    if (plot != nullptr) {
        delete plot;
        plot = nullptr;
    }
    currentCurves.clear();
    voltageCurves.clear();
}

void GapFreeController::detachCurves() {
    for (auto c : currentCurves) {
        c->detach();
    }
    for (auto c : voltageCurves) {
        c->detach();
    }
    plot->replot();
}

void GapFreeController::attachCurves() {
    for (auto c : currentCurves) {
        c->attach(plot);
    }
    for (auto c : voltageCurves) {
        c->attach(plot);
    }
    plot->replot();
}

void GapFreeController::start() {
    //onSetPlotData(messages[idx]);
    attachCurves();
    if (isAtLeastOneChannelExpanded()) {
        consumer->onStartConsuming();
    }
}

void GapFreeController::stop() {
    consumer->onStopConsuming();
    detachCurves();
}

void GapFreeController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setColor(colors[idx]);
        voltageCurves[idx]->setColor(colors[idx]);
    }
}

void GapFreeController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[channelIdx]->setColor(color);
        voltageCurves[channelIdx]->setColor(color);
    }
}

void GapFreeController::onBackgroundColorChanged(QColor color) {
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void GapFreeController::onReplot() {
    if (plot != nullptr) {
        plot->replot();
    }
}

//todo Check clamping modality too
void GapFreeController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    QwtPlot::Axis axisIdx;
    if (newRange.unit == "s") {
        axisIdx = QwtPlot::xBottom;
        model->setCurrentRange(axisIdx, newRange);
        Measurement_t duration = { model->getZoom(BigPlotModel::Zoom::Current)[axisIdx].width(), model->getCurrentRange(axisIdx).prefix, "s" };
        emit durationChanged(duration);

    }
    else if (newRange.unit == "V") {
        axisIdx = QwtPlot::yRight;
        model->setCurrentRange(axisIdx, newRange);

    }
    else if (newRange.unit == "A") {
        axisIdx = QwtPlot::yLeft;
        model->setCurrentRange(axisIdx, newRange);
    }
    plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);
    plot->replot();
}

void GapFreeController::onExpandTrace(bool flag) {
    auto isRunning = consumer->isRunning();
    stop();
    consumer->onSelectChannels(flag);
    if (isRunning || isAtLeastOneChannelExpanded()) {
        start();
    }
}

void GapFreeController::onSetPlotData(PlotMessage plotmessage) {
    GapFreeMessage gapFreeMessage = std::get<0>(plotmessage);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.currentValues[idx], gapFreeMessage.dataSize);
        voltageCurves[idx]->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.voltageValues[idx], gapFreeMessage.dataSize);
    }
}

PlotConsumer* GapFreeController::getConsumer() {
    return consumer;
}