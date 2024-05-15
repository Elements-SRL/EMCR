#include "eventdetectioncontroller.h"
#include "eventdetectionwidget.h"
#include <QVector>

EventDetectionController::EventDetectionController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bpw) :
    CentralWidgetController(appStatus, producer, bigPlotWidget) {
    widget = new EventDetectionWidget();
    bpw->setEventDetectionTab(widget);
    consumer = new EventDetectionConsumer(appStatus, producer);
    // creating curves for eventdetection
    //for (int i = 0; i < currentChannelsNum; i++) {
    //    currentCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
    //}
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
    //for (auto eventsInChannel : events) {
    //    eventsInChannel.clear();
    //}
    events.clear();
}

void EventDetectionController::detachCurves() {
    for (auto curvesInChannel: eventCurves) {
        for (auto c : curvesInChannel.second) {
            c->detach();
        }
    }
    widget->getPlot()->replot();
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
    //for (int idx = 0; idx < currentChannelsNum; idx++) {
    //    events[idx]->setColor(colors[idx]);
    //}
}

void EventDetectionController::onCurrentColorChanged(int channelIdx, QColor color) {
    //for (int idx = 0; idx < currentChannelsNum; idx++) {
    //    currentCurves[channelIdx]->setColor(color);
    //}
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
    message = std::get<2>(plotmessage);
    auto plot = widget->getPlot();
    detachCurves();
    for (int i = 0; i < currentChannelsNum; i++) {
        eventCurves[i].clear();
    }
    events = message.events;
    for (const auto& pair : message.events) {
        auto chIdx = pair.first;
        eventCurves[chIdx].clear();
        
        for (const auto& event : pair.second) {
            QwtPlotCurve* curve = new QwtPlotCurve();
            eventCurves[chIdx].push_back(curve);
            
            std::vector<double> data = event.event;
            //std::vector<double> data = {1.0 ,2.0, 3.0};
            QVector<double> yData(data.size());
            std::copy(data.begin(), data.end(), yData.begin());

            QVector<double> xData;
            for (int i = 0; i < yData.size(); i++) {
                xData << i;
            }
            curve->setSamples(xData, yData);
            curve->attach(plot);
        }
    }

    plot->show();
    plot->replot();
}

PlotConsumer* EventDetectionController::getConsumer() {
    return consumer;
}