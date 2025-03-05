#include "gapfreecontroller.h"

GapFreeController::GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, MainWindow* mw, DeviceController* dc):
    CentralWidgetController(appStatus, producer, bigPlotWidget) {
    
    auto model = std::make_unique<BigPlotModel>();
    auto plot = new BigPlot("", "[s]", "", BigPlot::GapFree, bigPlotWidget);
    plot->enableAxis(QwtPlot::yRight);
    dbbovc = std::make_unique<DurationBasedBigPlotViewController>(std::move(model), plot);
    dbbovc->setup();
    consumer = new GapFreePlotConsumer(appStatus, producer);
    consumer->onDurationChanged(defaultPlotDuration);
    this->abfDataWriterConsumer = new AbfDataWriterConsumer(appStatus, producer);

    gapFreeWidget = new GapFreeWidget(plot, mw);
    bigPlotWidget->setGapFreePlot(gapFreeWidget);
    //    creating curves for gapfree
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
    }
    for (int i = 0; i < voltageChannelsNum; i++) {
        voltageCurves.push_back(new Curve(CurveType_t::CurveTypePlotDashed));
        voltageCurves[i]->setYAxis(QwtPlot::yRight);
    }
    auto recordingSettingsDialog = mw->getRecordSettingsDialog();

    connect(gapFreeWidget, &GapFreeWidget::sigStartRecording, this, [=]() {
        this->onRecordingRequest(true);
    });
    connect(gapFreeWidget, &GapFreeWidget::sigStopRecording, this, [=]() {
        this->onRecordingRequest(false);
    });
    connect(gapFreeWidget, &GapFreeWidget::sigAutoZoom, this, [=]() {
        dbbovc->getPlot()->onAutoZoom({QwtPlot::yLeft, QwtPlot::yRight});
    });
    connect(abfDataWriterConsumer, &DataWriterConsumer::sigRecording, [=](bool flag) {
        this->onRecordingExecution(flag);
        dc->handleRecording(flag);
    });
    connect(this, &GapFreeController::sigStartRecording, this, &GapFreeController::onStartRecording);
    connect(this, &GapFreeController::sigStopRecording, this, &GapFreeController::onStopRecording);
    connect(recordingSettingsDialog, &RecordSettingsDialog::sigSettingsSet, abfDataWriterConsumer, &DataWriterConsumer::onRecordingSettingsSet);
    connect(gapFreeWidget, &GapFreeWidget::sigFileNameChanged, abfDataWriterConsumer, &DataWriterConsumer::onFilenameSet);
    connect(gapFreeWidget, &GapFreeWidget::sigRecordPathChanged, abfDataWriterConsumer, &DataWriterConsumer::onFilePathSet);
    connect(dbbovc.get(), &DurationBasedBigPlotViewController::durationChanged, consumer, &PlotConsumer::onDurationChanged);
    connect(consumer, &PlotConsumer::setPlotData, this, &GapFreeController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &GapFreeController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    consumer->onStopConsuming();
    recordingSettingsDialog->forceSettingsEmit();
}

GapFreeController::~GapFreeController() {
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    if (abfDataWriterConsumer != nullptr) {
        abfDataWriterConsumer->onStopConsuming();
        delete abfDataWriterConsumer;
        abfDataWriterConsumer = nullptr;
    }
    currentCurves.clear();
    voltageCurves.clear();
}

void GapFreeController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    auto plot = dbbovc->getPlot();
    for (auto ch : channelIndexes) {
        currentCurves[ch]->detach();
    }
    for (auto ch : channelIndexes) {
        voltageCurves[ch]->detach();
    }
    plot->replot();
}

void GapFreeController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    auto plot = dbbovc->getPlot();
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
    currentCurves[channelIdx]->setColor(color);
    voltageCurves[channelIdx]->setColor(color);
}

void GapFreeController::onBackgroundColorChanged(QColor color) {    
    auto plot = dbbovc->getPlot();
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void GapFreeController::onReplot() {
    auto plot = dbbovc->getPlot();
    if (plot != nullptr) {
        plot->replot();
    }
}

//todo Check clamping modality too
void GapFreeController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    auto plot = dbbovc->getPlot();
    auto model = dbbovc->getModel();
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
    GapFreeMessage gapFreeMessage = std::get<BigPlot::BigPlotStatus::GapFree>(plotmessage);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.currentValues[idx], gapFreeMessage.dataSize);
        voltageCurves[idx]->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.voltageValues[idx], gapFreeMessage.dataSize);
    }
}

PlotConsumer* GapFreeController::getConsumer() {
    return consumer;
}

std::vector <DeviceDataConsumer*> GapFreeController::getConsumers() {
    return {consumer, abfDataWriterConsumer};
}

void GapFreeController::onRecordingRequest(bool flag) {
    if (flag) {
        std::vector <uint16_t> selectedChannels;
        auto msgDisp = appStatus->getMessageDispatcher();
        msgDisp->getSelectedChannelsIndexes(selectedChannels);

        if (!(selectedChannels.empty())) {
            emit sigStartRecording();

        }
        else {
            QString err = "Recording to file not possible";
            QString info = "No channel checked for recording";
            ErrorManager e(err, info);
        }

    }
    else {
        emit sigStopRecording();
    }
}

void GapFreeController::onRecordingExecution(bool flag) {
    gapFreeWidget->setRecording(flag);
}

void GapFreeController::onProtocolStarted(unsigned int, ProtocolWidget * protocol) {
    if (protocol->getType() != ProtocolTypeGapfree) {
        consumer->onStopConsuming();
        return;
    }
    if (!(consumer->isRunning())) {
        consumer->onStartConsuming();
    }
}

//TODO this could be moved at the controller level and b managed by single controllers
void GapFreeController::onStartRecording() {
    const auto selectedChannels = appStatus->getSelectedChannelsIndexes();
    std::vector <bool> values(selectedChannels.size(), true);
    abfDataWriterConsumer->onRecordSelectedChannels(selectedChannels, values);
}

void GapFreeController::onStopRecording() {
    abfDataWriterConsumer->onStopConsuming();
}
