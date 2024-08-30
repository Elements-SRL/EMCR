#include "spectrumcontroller.h"

#include <qwt_date_scale_engine.h>

SpectrumController::SpectrumController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotBandwidth, BigPlotWidget * bigPlotWidget, BigPlotController * bigPlotController, MainWindow * mainWindow):
    CentralWidgetController(appStatus, producer, bigPlotWidget),
    mainWindow(mainWindow){

    model = new BigPlotModel(BigPlot::Spectrum);
    consumer = new SpectrumConsumer(appStatus, producer);
    consumer->onIntegrationWindowChanged({1.0, UnitPfxNone, "s"});

    plot = new BigPlot("", "[Hz]", "", BigPlot::Spectrum, bigPlotWidget);
    plot->enableAxis(QwtPlot::yRight);
    plot->setAxisAutoScale(QwtPlot::xBottom, false);
    plot->setAxisAutoScale(QwtPlot::yLeft, false);
    plot->setAxisAutoScale(QwtPlot::yRight, false);
    plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
    plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));

    spectrumWidget = new SpectrumWidget(currentChannelsNum, plot, bigPlotWidget);

    bigPlotWidget->setSpectrumPlot(spectrumWidget);
    //    creating curves for spectra
    for (int i = 0; i < currentChannelsNum; i++) {
        psdCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
        irmsCurves.push_back(new Curve(CurveType_t::CurveTypePlotDashed));
        irmsCurves[i]->setYAxis(QwtPlot::yRight);
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

    connect(consumer, &PlotConsumer::setPlotData, this, &SpectrumController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &SpectrumController::onReplot);
    connect(consumer, &SpectrumConsumer::sigRangeUpdate, this, &SpectrumController::onRangeUpdated);
    connect(spectrumWidget, &SpectrumWidget::integrationWindowChanged, this, [=] (double windowS) {
        consumer->onIntegrationWindowChanged({windowS, UnitPfxNone, "s"});
    });
    connect(spectrumWidget, &SpectrumWidget::startPressed, consumer, &SpectrumConsumer::onStartConsuming);
    connect(spectrumWidget, &SpectrumWidget::stopPressed, consumer, &SpectrumConsumer::onStopConsuming);
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
    psdCurves.clear();
    irmsCurves.clear();
}

void SpectrumController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        psdCurves[ch]->detach();
        irmsCurves[ch]->detach();
    }
    plot->replot();
}

void SpectrumController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        psdCurves[ch]->attach(plot);
        irmsCurves[ch]->attach(plot);
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
        psdCurves[idx]->setColor(colors[idx]);
        irmsCurves[idx]->setColor(colors[idx]);
    }
}

void SpectrumController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        psdCurves[channelIdx]->setColor(color);
        irmsCurves[channelIdx]->setColor(color);
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

/*! todo FCON Check clamping modality too */
void SpectrumController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    QwtPlot::Axis axisIdx;
    if (newRange.unit == "A") {
        axisIdx = QwtPlot::yLeft;
        model->setCurrentRangeLog(axisIdx, newRange);
        plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()) + "^2/Hz", axisIdx);

        axisIdx = QwtPlot::yRight;
        model->setCurrentRangeLog(axisIdx, newRange);
        plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()) + "rms", axisIdx);

    } else if (newRange.unit == "Hz") {
        axisIdx = QwtPlot::xBottom;
        model->setCurrentRange(axisIdx, newRange);
        plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);

    } else {
        return;
    }
    plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
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
        psdCurves[idx]->setRawSamples(message.frequencyValues, message.psdValues[idx], message.dataSize);
        irmsCurves[idx]->setRawSamples(message.frequencyValues, message.irmsValues[idx], message.dataSize);
    }
}

PlotConsumer * SpectrumController::getConsumer() {
    return consumer;
}
