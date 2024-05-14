#include "eventdetectioncontroller.h"

EventDetectionController::EventDetectionController(ApplicationStatus* appStatus, DeviceDataProducer* producer) :
    CentralWidgetController(appStatus, producer, bigPlotWidget) {

    consumer = new EventDetectionConsumer(appStatus, producer);
    // creating curves for eventdetection
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
    }
    connect(consumer, &PlotConsumer::setPlotData, this, &EventDetectionController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &EventDetectionController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    consumer->onSelectChannels(false);
    consumer->onStopConsuming();
}

EventDetectionController::~EventDetectionController() {
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    currentCurves.clear();
}

void EventDetectionController::detachCurves() {
    //for (auto c : currentCurves) {
    //    c->detach();
    //}
    //plot->replot();
}

void EventDetectionController::attachCurves() {
    //for (auto c : currentCurves) {
    //    c->attach(plot);
    //}
    //plot->replot();
}

void EventDetectionController::start() {
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves();
    consumer->onStartConsuming();
}

void EventDetectionController::stop() {
    consumer->onStopConsuming();
    detachCurves();
}

void EventDetectionController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setColor(colors[idx]);
    }
}

void EventDetectionController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[channelIdx]->setColor(color);
    }
}

void EventDetectionController::onBackgroundColorChanged(QColor color) {
    //if (plot != nullptr) {
    //    plot->setCanvasBackground(color);
    //}
}

void EventDetectionController::onReplot() {
    //if (plot != nullptr) {
    //    plot->replot();
    //}
}

//todo Check clamping modality too
void EventDetectionController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    //QwtPlot::Axis axisIdx;
    //if (newRange.unit == "s") {
    //    axisIdx = QwtPlot::xBottom;
    //    model->setCurrentRange(axisIdx, newRange);
    //    Measurement_t duration = { model->getZoom(BigPlotModel::Zoom::Current)[axisIdx].width(), model->getCurrentRange(axisIdx).prefix, "s" };
    //    emit durationChanged(duration);

    //}
    //else if (newRange.unit == "V") {
    //    axisIdx = QwtPlot::yRight;
    //    model->setCurrentRange(axisIdx, newRange);

    //}
    //else if (newRange.unit == "A") {
    //    axisIdx = QwtPlot::yLeft;
    //    model->setCurrentRange(axisIdx, newRange);
    //}
    //plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    //plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);
    //plot->replot();
}

void EventDetectionController::onExpandTrace(bool flag) {
    auto wasRunning = consumer->isRunning();
    if (wasRunning) {
        stop();
        consumer->onSelectChannels(flag);
        start();
    }
    else {
        consumer->onSelectChannels(flag);
    }
}

void EventDetectionController::onSetPlotData(PlotMessage plotmessage) {

}

PlotConsumer* EventDetectionController::getConsumer() {
    return consumer;
}