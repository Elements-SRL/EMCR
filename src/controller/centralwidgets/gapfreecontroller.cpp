#include "gapfreecontroller.h"

GapFreeController::GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, RangedMeasurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, MainWindow* mw, DeviceController* dc):
    CentralWidgetController(appStatus, producer, bigPlotWidget) {


    std::map<QwtPlot::Axis, AxisInfo> m;
    m[QwtPlot::Axis::yLeft] = {appStatus->getCurretRange()};
    m[QwtPlot::Axis::yRight] = {appStatus->getVoltageRange()};
    m[QwtPlot::Axis::xBottom] = {defaultPlotDuration, std::make_optional(0.0)};
    // TODO manage vc -> cc and vice versa
    pc = std::make_unique<PlotController>(m, mw);
    consumer = new GapFreePlotConsumer(appStatus, producer);

    consumer->onDurationChanged(defaultPlotDuration.getMax());

    this->abfDataWriterConsumer = new AbfDataWriterConsumer(appStatus, producer);

    gapFreeWidget = new GapFreeWidget(pc->getPlot(), mw);
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

    // TODO FARE QUESTO COMPATIBILE
    connect(gapFreeWidget, &GapFreeWidget::sigAutoZoom, pc.get(), &PlotController::onAutoZoom);

    connect(abfDataWriterConsumer, &DataWriterConsumer::sigRecording, [=](bool flag) {
        this->onRecordingExecution(flag);
        dc->handleRecording(flag);
    });
    connect(this, &GapFreeController::sigStartRecording, this, &GapFreeController::onStartRecording);
    connect(this, &GapFreeController::sigStopRecording, this, &GapFreeController::onStopRecording);
    connect(recordingSettingsDialog, &RecordSettingsDialog::sigSettingsSet, abfDataWriterConsumer, &DataWriterConsumer::onRecordingSettingsSet);
    connect(gapFreeWidget, &GapFreeWidget::sigFileNameChanged, abfDataWriterConsumer, &DataWriterConsumer::onFilenameSet);
    connect(gapFreeWidget, &GapFreeWidget::sigRecordPathChanged, abfDataWriterConsumer, &DataWriterConsumer::onFilePathSet);

    // TODO FARE QUESTO COMPATIBILE
    connect(pc.get(), &PlotController::sigPlotUpdated, this, &GapFreeController::onAxesChanged);
    connect(this, &GapFreeController::sigDurationChanged, consumer, &PlotConsumer::onDurationChanged);

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
    auto plot = pc->getPlot();
    for (auto ch : channelIndexes) {
        currentCurves[ch]->detach();
    }
    for (auto ch : channelIndexes) {
        voltageCurves[ch]->detach();
    }
    plot->replot();
}

void GapFreeController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    auto plot = pc->getPlot();
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
    auto plot = pc->getPlot();
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void GapFreeController::onReplot() {
    auto plot = pc->getPlot();
    if (plot != nullptr) {
        plot->replot();
    }
}

//todo Check clamping modality too
void GapFreeController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    std::map<QwtPlot::Axis, AxisInfo> m;
    QwtPlot::Axis axis;
    if (newRange.unit == "s") {
        axis = QwtPlot::xBottom;
        emit sigDurationChanged(newRange.getMax());
    } else if (newRange.unit == "V") {
        axis = QwtPlot::yRight;
    } else if (newRange.unit == "A") {
        axis = QwtPlot::yLeft;
    } else {
        return;
    }
    pc->setRangedMeasurement(axis, newRange);
    // plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    // plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);
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
        auto msgDisp = appStatus->getMessageDispatcher();
        auto selectedChannels = appStatus->getSelectedChannelsIndexes();

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

void GapFreeController::onAxesChanged() {
    auto rm = pc->getModel()->getAxisRangedMeasurement(QwtPlot::Axis::xBottom);
    auto delta = rm.delta();
    Measurement duration = {delta, rm.prefix, rm.unit};
    emit sigDurationChanged(duration);
}
