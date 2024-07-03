#include "spectrumcontroller.h"

#include <qwt_date_scale_engine.h>

SpectrumController::SpectrumController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotBandwidth, BigPlotWidget * bigPlotWidget, BigPlotController * bigPlotController, MainWindow * mainWindow):
    CentralWidgetController(appStatus, producer, bigPlotWidget),
    mainWindow(mainWindow){

    model = new BigPlotModel();
    consumer = new SpectrumConsumer(appStatus, producer);
    consumer->onIntegrationWindowChanged({1.0, UnitPfxNone, "s"});
    spectrumWidget = new SpectrumWidget(currentChannelsNum, bigPlotWidget);
    mainWindow->setSpectrumWidget(spectrumWidget);

    plot = new BigPlot("", "[Hz]", "", BigPlot::Spectrum, bigPlotWidget);
    plot->setAxisAutoScale(QwtPlot::xBottom, true);
    plot->setAxisAutoScale(QwtPlot::yLeft, true);
    plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
    plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
    bigPlotWidget->setSpectrumPlot(plot);
    //    creating curves for spectra
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
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

    connect(this, &SpectrumController::integrationWindowChanged, consumer, &SpectrumConsumer::onIntegrationWindowChanged);
    connect(consumer, &PlotConsumer::setPlotData, this, &SpectrumController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &SpectrumController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(SPC_MAX_SAMPLES);
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    consumer->onPlotChannels(allChannels, false);
    consumer->onStopConsuming();
}

SpectrumController::~SpectrumController() {
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
}

void SpectrumController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        currentCurves[ch]->detach();
    }
    plot->replot();
}

void SpectrumController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        currentCurves[ch]->attach(plot);
    }
    plot->replot();
}

void SpectrumController::start() {
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves(appStatus->getExpandedChannelsIndexes());
    consumer->onStartConsuming();
}

void SpectrumController::stop() {
    consumer->onStopConsuming();
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    detachCurves(allChannels);
}

void SpectrumController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setColor(colors[idx]);
    }
}

void SpectrumController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[channelIdx]->setColor(color);
    }
}

void SpectrumController::onBackgroundColorChanged(QColor color) {
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void SpectrumController::onReplot() {
    if (plot != nullptr) {
        plot->replot();
    }
}

/*! todo Check clamping modality too */
void SpectrumController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    QwtPlot::Axis axisIdx;
    if (newRange.unit == "A") {
        axisIdx = QwtPlot::yLeft;
        model->setCurrentRangeSquared(axisIdx, newRange);

    } else {
        return;
    }
    plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()) + "^2/Hz", axisIdx);
    plot->replot();
}

void SpectrumController::onExpandTrace(bool flag) {
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

void SpectrumController::onSetPlotData(PlotMessage plotmessage) {
    SpectrumMessage message = std::get <3> (plotmessage);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setRawSamples(message.frequencyValues, message.currentValues[idx], message.dataSize);
    }
}

PlotConsumer * SpectrumController::getConsumer() {
    return consumer;
}
