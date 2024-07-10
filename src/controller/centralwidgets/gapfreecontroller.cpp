#include "gapfreecontroller.h"

GapFreeController::GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController):
    CentralWidgetController(appStatus, producer, bigPlotWidget) {

    model = new BigPlotModel();
    consumer = new GapFreePlotConsumer(appStatus, producer);
    consumer->onDurationChanged(defaultPlotDuration);

    plot = new BigPlot("", "[s]", "", BigPlot::GapFree, bigPlotWidget);
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

    connect(plot, &BigPlot::zoomInRequest, bigPlotController, [=](Rect4 r) {
        bigPlotController->handleZoomInRequest(model, plot, r);
        });
    connect(plot, &BigPlot::zoomOutRequest, bigPlotController, [=]() {
        bigPlotController->handleZoomOutRequest(model, plot);
        });
    connect(plot, &BigPlot::zoomResetRequest, bigPlotController, [=]() {
        bigPlotController->handleZoomResetRequest(model, plot);
        });
    connect(plot, &BigPlot::singleAxisZoomRequest, bigPlotController, [=](QwtPlot::Axis axis, int zoomIn, QPointF mousePosition) {
        bigPlotController->handleSingleAxisZoomRequest(model, plot, axis, zoomIn, mousePosition);
        });
    connect(plot, &BigPlot::singleAxisShiftRequest, bigPlotController, [=](QwtPlot::Axis axis, int shift) {
        bigPlotController->handleSingleAxisShiftRequest(model, plot, axis, shift);
        });

    connect(bigPlotController, &BigPlotController::durationChanged, consumer, &PlotConsumer::onDurationChanged);
    connect(consumer, &PlotConsumer::setPlotData, this, &GapFreeController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &GapFreeController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    consumer->onPlotChannels(allChannels, false);
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

void GapFreeController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        currentCurves[ch]->detach();
    }
    for (auto ch : channelIndexes) {
        voltageCurves[ch]->detach();
    }
    plot->replot();
}

void GapFreeController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        currentCurves[ch]->attach(plot);
    }
    for (auto ch : channelIndexes) {
        voltageCurves[ch]->attach(plot);
    }
    plot->replot();
}

void GapFreeController::start() {
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves(appStatus->getExpandedChannelsIndexes());
    consumer->onStartConsuming();
}

void GapFreeController::stop() {
    consumer->onStopConsuming();
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    detachCurves(allChannels);
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

    } else if (newRange.unit == "V") {
        axisIdx = QwtPlot::yRight;
        model->setCurrentRange(axisIdx, newRange);

    } else if (newRange.unit == "A") {
        axisIdx = QwtPlot::yLeft;
        model->setCurrentRange(axisIdx, newRange);

    } else {
        return;
    }
    plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);
    plot->replot();
}

void GapFreeController::onExpandTrace(bool flag) {
    auto wasRunning = consumer->isRunning();
    if (wasRunning) {
        stop();
        consumer->onPlotSelectedChannels(flag);
        start();
    }
    else {
        consumer->onPlotSelectedChannels(flag);
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
