#include "episodiccontroller.h"

#include <qwt_scale_map.h>

EpisodicController::EpisodicController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, MainWindow* mw, DeviceController* dc):
    CentralWidgetController(appStatus, producer, bigPlotWidget) {

    if (appStatus->getMessageDispatcher()->hasProtocols() == Success) {
        pw = static_cast <ProtocolDockWidget *> (mw->getDockWidget(MainWindow::DWProtocol));
    }

    auto model = std::make_unique<BigPlotModel>();
    consumer = new EpisodicPlotConsumer(appStatus, producer);
    consumer->onDurationChanged(defaultPlotDuration);
    this->episodicDataWriterConsumer = new EpisodicAbfDataWriterConsumer(appStatus, producer);
    auto plot = new BigPlot("", "[s]", "", BigPlot::Episodic, bigPlotWidget);

    plot->enableAxis(QwtPlot::yRight);

    episodicPainter = new QwtPlotDirectPainter(plot);

    episodicWidget = new EpisodicWidget(plot, mw);

    bpvc = std::make_unique<DurationBasedBigPlotViewController>(std::move(model), plot);
    bpvc->setup();

    bigPlotWidget->setEpisodicPlot(episodicWidget);
    //    creating curves for episodic
    currentCurves.resize(currentChannelsNum);
    voltageCurves.resize(voltageChannelsNum);
    for (auto &&curve : currentCurves) {
        curve.reserve(1024);
    }
    for (auto &&curve : voltageCurves) {
        curve.reserve(1024);
    }
    activeCurrentCurveData.resize(currentChannelsNum);
    activeVoltageCurveData.resize(voltageChannelsNum);
    auto recordingSettingsDialog = mw->getRecordSettingsDialog();
    connect(episodicWidget, &EpisodicWidget::sigStartRecording, this, [=]() {
        this->onRecordingRequest(true);
    });
    connect(episodicWidget, &EpisodicWidget::sigStopRecording, this, [=]() {
        this->onRecordingRequest(false);
    });
    connect(episodicWidget, &EpisodicWidget::sigAutoZoom, this, [=]() {
        bpvc->getPlot()->onAutoZoom({QwtPlot::yLeft, QwtPlot::yRight});
    });
    connect(episodicDataWriterConsumer, &DataWriterConsumer::sigRecording, [=](bool flag) {
        this->onRecordingExecution(flag);
        dc->handleRecording(flag);
    });
    connect(this, &EpisodicController::sigStartRecording, this, &EpisodicController::onStartRecording);
    connect(this, &EpisodicController::sigStopRecording, this, &EpisodicController::onStopRecording);
    connect(recordingSettingsDialog, &RecordSettingsDialog::sigSettingsSet, episodicDataWriterConsumer, &DataWriterConsumer::onRecordingSettingsSet);
    connect(episodicWidget, &EpisodicWidget::sigFileNameChanged, episodicDataWriterConsumer, &DataWriterConsumer::onFilenameSet);
    connect(episodicWidget, &EpisodicWidget::sigRecordPathChanged, episodicDataWriterConsumer, &DataWriterConsumer::onFilePathSet);
    connect(bpvc.get(), &DurationBasedBigPlotViewController::durationChanged, consumer, &PlotConsumer::onDurationChanged); /*! \todo FCON occhio che nei plot episodici la gestione della durata con
                                                                                                                           lo zoom è gestita diversamente: esiste una durata preferenziale
                                                                                                                           che è quella del protocollo e l'asse temporale non può cambiare
                                                                                                                           rispetto alla configurazione iniziale */
    connect(consumer, &PlotConsumer::setPlotData, this, &EpisodicController::onSetPlotData);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(PCS_MAX_SAMPLES_PER_EPISODIC_PLOT);
    consumer->onStopConsuming();
    recordingSettingsDialog->forceSettingsEmit();
}

EpisodicController::~EpisodicController() {
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    this->clearCurves();
    if (episodicDataWriterConsumer != nullptr) {
        episodicDataWriterConsumer->onStopConsuming();
        delete episodicDataWriterConsumer;
        episodicDataWriterConsumer = nullptr;
    }
    pw = nullptr;
}

void EpisodicController::clearCurves() {
    std::vector <uint16_t> channelIndexes;
    for (uint16_t ch = 0; ch < currentChannelsNum; ch++) {
        channelIndexes.push_back(ch);
    }
    this->detachCurves(channelIndexes);

    for (auto ch : channelIndexes) {
        for (auto &&curve : currentCurves[ch]) {
            delete curve;
        }
        for (auto &&curve : voltageCurves[ch]) {
            delete curve;
        }
        currentCurves[ch].clear();
        voltageCurves[ch].clear();
    }
    sweepIdx = -1;
    bpvc->getPlot()->replot();
}

void EpisodicController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        for (auto curve : currentCurves[ch]) {
            curve->detach();
        }
        for (auto curve : voltageCurves[ch]) {
            curve->detach();
        }
    }
    bpvc->getPlot()->replot();
}

void EpisodicController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    auto plot = bpvc->getPlot();
    for (auto ch : channelIndexes) {
        for (auto curve : currentCurves[ch]) {
            curve->attach(plot);
        }
        for (auto curve : voltageCurves[ch]) {
            curve->attach(plot);
        }
    }
    plot->replot();
}

void EpisodicController::paintPlots(bool newSweepFlag) {
    auto plot = bpvc->getPlot();
    if (newSweepFlag) {
        for (int currentChannelIdx = 0; currentChannelIdx < currentChannelsNum; currentChannelIdx++) {
            if (persistentSweepsNum > 0) {
                if (sweepIdx > 0) {
                    currentCurves[currentChannelIdx][sweepIdx-1]->setColor(CurveTypePlotFaint);
                }
                if (sweepIdx >= persistentSweepsNum) {
                    currentCurves[currentChannelIdx][sweepIdx-persistentSweepsNum]->setVisible(false);
                }
            }
        }

        for (int voltageChannelIdx = 0; voltageChannelIdx < voltageChannelsNum; voltageChannelIdx++) {
            if (persistentSweepsNum > 0) {
                if (sweepIdx > 0) {
                    voltageCurves[voltageChannelIdx][sweepIdx-1]->setColor(CurveTypePlotFaintDashed);
                }
                if (sweepIdx >= persistentSweepsNum) {
                    voltageCurves[voltageChannelIdx][sweepIdx-persistentSweepsNum]->setVisible(false);
                }
            }
        }
        plot->replot();
        plot->repaint();
        activeSweepPlottedPoints = 1;
    }

    Curve * curve;
    CurveData * curveData;

    /*! \todo FCON this assumes that all current and voltage curves are in the same plot */
    const QwtScaleMap tMap = plot->canvasMap(QwtPlot::xBottom);
    const QwtScaleMap iMap = plot->canvasMap(QwtPlot::yLeft);
    const QwtScaleMap vMap = plot->canvasMap(QwtPlot::yRight);

    int newActiveSweepPlottedPoints = activeSweepPlottedPoints;
    for (int currentChannelIdx = 0; currentChannelIdx < currentChannelsNum; currentChannelIdx++) {
        curve = currentCurves[currentChannelIdx][sweepIdx];
        curveData = static_cast <CurveData *> (curve->data());

        QRectF br = qwtBoundingRect(* curveData, activeSweepPlottedPoints-1, (int)(curveData->size()-1));
        const QRect clipRect = QwtScaleMap::transform(tMap, iMap, br).toRect();

        episodicPainter->setClipRegion(clipRect);

        episodicPainter->drawSeries(curve, activeSweepPlottedPoints-1, (int)(curveData->size()-1));
        newActiveSweepPlottedPoints = (int)(curveData->size());
    }

    for (int voltageChannelIdx = 0; voltageChannelIdx < voltageChannelsNum; voltageChannelIdx++) {
        curve = voltageCurves[voltageChannelIdx][sweepIdx];
        curveData = static_cast <CurveData *> (curve->data());

        QRectF br = qwtBoundingRect(* curveData, activeSweepPlottedPoints-1, (int)(curveData->size()-1));
        const QRect clipRect = QwtScaleMap::transform(tMap, vMap, br).toRect();

        episodicPainter->setClipRegion(clipRect);

        episodicPainter->drawSeries(curve, activeSweepPlottedPoints-1, (int)(curveData->size()-1));
        newActiveSweepPlottedPoints = (int)(curveData->size());
    }

    // if (protocolFinished) {
    //     sweepPersistenceSbx->setEnabled(true);
    //     sweepSelectionCbx->setEnabled(true);
    //     freezeSweepsBtn->setEnabled(true);
    // }

    activeSweepPlottedPoints = newActiveSweepPlottedPoints;
}

void EpisodicController::start() {
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves(appStatus->getExpandedChannelsIndexes());
    consumer->onStartConsuming();
}

void EpisodicController::stop() {
    consumer->onStopConsuming();
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    detachCurves(allChannels);
}

void EpisodicController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        for (auto curve : currentCurves[idx]) {
            curve->setColor(colors[idx]);
        }
        for (auto curve : voltageCurves[idx]) {
            curve->setColor(colors[idx]);
        }
    }
}

void EpisodicController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (auto curve : currentCurves[channelIdx]) {
        curve->setColor(color);
    }
    for (auto curve : voltageCurves[channelIdx]) {
        curve->setColor(color);
    }
}

void EpisodicController::onBackgroundColorChanged(QColor color) {
    auto plot = bpvc->getPlot();
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void EpisodicController::onReplot() {
    auto plot = bpvc->getPlot();
    if (plot != nullptr) {
        plot->replot();
    }
}

//todo Check clamping modality too
void EpisodicController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    auto plot = bpvc->getPlot();
    auto model = bpvc->getModel();
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

void EpisodicController::onExpandTrace(bool flag) {
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

void EpisodicController::onSetPlotData(PlotMessage plotmessage) {
    auto plot = bpvc->getPlot();
    auto model = bpvc->getModel();
    EpisodicMessage episodicMessage = std::get<BigPlot::BigPlotStatus::Episodic>(plotmessage);
    if (episodicMessage.newProtocolFlag) {
        this->clearCurves();
        // bpvc->handleSingleAxisZoomRequest(model, plot, QwtPlot::xBottom, QwtInterval(0.0, episodicMessage.durationS)); /*! \todo FCON non è detto che qui serva in s la misura, verificare */
        plot->replot();

        updateDataTimer.start();

        lastUpdateTimeMs = updateDataTimer.elapsed();
        episodicMessage.newSweepFlag = true;
    }

    if (episodicMessage.newSweepFlag) {
        if (!episodicMessage.newProtocolFlag) {
            this->paintPlots(episodicMessage.newSweepFlag);
        }

        Curve * curve;
        sweepIdx++;

        for (int idx = 0; idx < currentChannelsNum; idx++) {
            curve = new Curve(CurveTypePlotSolid);
            activeCurrentCurveData[idx] = new CurveData(PCS_MAX_SAMPLES_PER_EPISODIC_PLOT);
            curve->setData(activeCurrentCurveData[idx]);
            currentCurves[idx].push_back(curve);
        }

        for (int idx = 0; idx < voltageChannelsNum; idx++) {
            curve = new Curve(CurveTypePlotDashed);
            curve->setYAxis(QwtPlot::yRight);
            activeVoltageCurveData[idx] = new CurveData(PCS_MAX_SAMPLES_PER_EPISODIC_PLOT);
            curve->setData(activeVoltageCurveData[idx]);
            voltageCurves[idx].push_back(curve);
        }

        for (auto idx : appStatus->getExpandedChannelsIndexes()) {
            currentCurves[idx][sweepIdx]->attach(plot);
            voltageCurves[idx][sweepIdx]->attach(plot);
        }
    }

    for (int idx = 0; idx < currentChannelsNum; idx++) {
        activeCurrentCurveData[idx]->append(episodicMessage.timeValues, episodicMessage.currentValues[idx]);
        activeVoltageCurveData[idx]->append(episodicMessage.timeValues, episodicMessage.voltageValues[idx]);
    }

    currentTimeMs = updateDataTimer.elapsed();
    if (currentTimeMs-lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS || episodicMessage.newSweepFlag) {
        lastUpdateTimeMs = currentTimeMs;
        this->paintPlots(episodicMessage.newSweepFlag);
    }
}

PlotConsumer* EpisodicController::getConsumer() {
    return consumer;
}

std::vector <DeviceDataConsumer *> EpisodicController::getConsumers() {
    return {consumer, episodicDataWriterConsumer};
}

void EpisodicController::onRecordingRequest(bool flag) {
    if (flag) {
        std::vector <uint16_t> selectedChannels;
        auto msgDisp = appStatus->getMessageDispatcher();
        msgDisp->getSelectedChannelsIndexes(selectedChannels);
        ProtocolList * pl;
        if (appStatus->getClampingModality() == e384CommLib::VOLTAGE_CLAMP) {
            pl = pw->getVoltageProtocolList();
        } else {
            pl = pw->getCurrentProtocolList();
        }
        ProtocolWidget * protocol = static_cast <ProtocolWidget *> (pl->currentItem());

        if (protocol->getType() != ProtocolTypeEpisodic) {
            QString err = "No episodic protocol selected";
            QString info = "Select an episodic protocol to start the recording";
            ErrorManager e(err, info);
            return;
        }

        if (!(selectedChannels.empty())) {
            emit sigStartRecording();

        }
        else {
            QString err = "Recording to file not possible";
            QString info = "No channel checked for recording";
            ErrorManager e(err, info);
        }

        pl->onStartProtocol(true);
    }
    else {
        emit sigStopRecording();
    }
}

void EpisodicController::onRecordingExecution(bool flag) {
    episodicWidget->setRecording(flag);
}

void EpisodicController::onProtocolStarted(unsigned int protocolId, ProtocolWidget * protocol) {
    consumer->onStopConsuming();
    if (protocol->getType() != ProtocolTypeEpisodic) {
        return;
    }
    consumer->setProtocolId(protocolId);
    consumer->setSweepsNum(protocol->getSweepsNum());
    auto duration = protocol->getTotalDuration();
    auto durationS = duration.getNoPrefixValue();
    if (durationS == 0.0) {
        /*! Don't do anything on null protocols, such as stop protocols */
        return;
    }
    this->onRangeUpdated({0.0, duration.value, 1.0, duration.prefix, duration.unit}); /*! Set default duration every time a new protocol starts */
    consumer->onDurationChanged(duration);
    consumer->onStartConsuming();
}

//TODO this could be moved at the controller level and be managed by single controllers
void EpisodicController::onStartRecording() {
    const auto selectedChannels = appStatus->getSelectedChannelsIndexes();
    std::vector <bool> values(selectedChannels.size(), true);
    episodicDataWriterConsumer->onRecordSelectedChannels(selectedChannels, values);
}

void EpisodicController::onStopRecording() {
    episodicDataWriterConsumer->onStopConsuming();
}

CurveData::CurveData(int size) {
    x.reserve(size);
    y.reserve(size);
}

CurveData::~CurveData() {
    x.clear();
    y.clear();
}

QPointF CurveData::sample(size_t idx) const {
    return QPointF(x[idx], y[idx]);
}

size_t CurveData::size() const {
    return x.size();
}

QRectF CurveData::boundingRect() const {
    if (m_boundingRect.isEmpty()) {
        // Calculate the bounding rect only when needed
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = minX;
        double maxY = maxX;

        for (const auto& point : x) {
            minX = std::min(minX, point);
            maxX = std::max(maxX, point);
        }
        for (const auto& point : y) {
            minY = std::min(minY, point);
            maxY = std::max(maxY, point);
        }

        m_boundingRect = QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
    }

    return m_boundingRect;
}

void CurveData::append(std::vector <double> newX, std::vector <double> newY) {
    x.insert(x.end(), newX.begin(), newX.end());
    y.insert(y.end(), newY.begin(), newY.end());
}
