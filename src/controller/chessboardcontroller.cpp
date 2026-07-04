#include "chessboardcontroller.h"

#include <QApplication>

ChessboardController::ChessboardController(ApplicationStatus * appStatus, DeviceDataProducer* dataProducer, Measurement_t defaultDuration, MainWindow * mainWindow) :
    ControllerWithConsumer(appStatus),
    mainWindow(mainWindow) {

    stampPlotConsumer = new GapFreePlotConsumer(appStatus, dataProducer);
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();

    chessboard = new ChessboardDockWidget(appStatus, mainWindow);

    int idealPlotWidth = chessboard->getIdealPlotWidth();
    int idealPlotHeight = chessboard->getIdealPlotHeight();

    plots.resize(currentChannelsNum);
    currentCurves.resize(currentChannelsNum);

    for(auto mapping: appStatus->getMappings()){
        /*! buttare in una funzioncina di creazione del plot*/
        const auto name = mapping.name;
        const auto channelIdx = mapping.index;
        const auto visibility = mapping.visible;
        StampPlot * plot = new StampPlot(channelIdx, name, idealPlotWidth, idealPlotHeight, chessboard);
        plot->resize(idealPlotWidth, idealPlotHeight);
        plot->setToolTip(QString("Ch %1\n"
                                 "Left click: exclusive select\n"
                                 "CTRL + Left click: append\n"
                                 "Right click: deselect").arg(QString::fromStdString(name)));
        plot->setSelected(false);

        plots[channelIdx] = plot;
        plot->setVisible(visibility);
        Curve * curve = new Curve(CurveType_t::CurveTypeStampPlotSolid);
        curve->attach(plot);
        currentCurves[channelIdx] = curve;

        chessboard->addPlot(plot, channelIdx);
    };

//    setting initial values for stampPlotConsumer
    stampPlotConsumer->onDurationChanged(defaultDuration);
    stampPlotConsumer->forceAxisUpdate();
    stampPlotConsumer->setMaxSamplesPerPlot(256);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       [=](bool newChannelState){
        onSelectedPlotsUpdated();
        onAllChannelsClicked(newChannelState);
        emit sigAllChannelsClicked(newChannelState);
    });
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       [=](uint16_t changedBoardIndex, bool newChannelState){
        onSelectedPlotsUpdated();
        onOneBoardClicked(changedBoardIndex, newChannelState);
        emit sigOneBoardClicked(changedBoardIndex, newChannelState);
    });
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       [=](uint16_t changedRowIndex, bool newChannelState){
        onSelectedPlotsUpdated();
        onOneRowClicked(changedRowIndex, newChannelState);
        emit sigAllChannelsClicked(newChannelState);
    });
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       [=](uint16_t changedChannelIndex, QMouseEvent * event){
        onSelectedPlotsUpdated();
        onSingleChannelClicked(changedChannelIndex, event);
        emit sigSingleChannelClicked(changedChannelIndex, event);
    });

    connect(chessboard, &ChessboardDockWidget::sigInvertSelectionClicked, this, [=]() {
        std::map<int, bool> newStatus;

        for (int i = 0; i < plots.size(); ++i) {
            StampPlot* plot = plots[i];
            if (plot) {
                // Status inversion
                bool nextState = !plot->isSelected();
                plot->setSelected(nextState);
                newStatus[i] = nextState;
            }
        }
        appStatus->setSelectedChannels(newStatus);
        emit sigInvertSelectionClicked();
    });

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,     this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,        this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,          this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked,   this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigInvertSelectionClicked, this,       &ChessboardController::onSelectedPlotsUpdated);

    connect(stampPlotConsumer, &PlotConsumer::setPlotData,              this,       &ChessboardController::onSetPlotData);
    connect(stampPlotConsumer, &PlotConsumer::plotDataUpdated,          this,       &ChessboardController::onReplot);
    connect(chessboard, &QDockWidget::visibilityChanged,                this,       &ChessboardController::onSetConsumerStatus);
    mainWindow->setDockWidget(MainWindow::DWChessboard, chessboard, true, Qt::LeftDockWidgetArea);
    onSelectedPlotsUpdated();
}

void ChessboardController::onSetConsumerStatus(bool status) {
    if (status) {
        stampPlotConsumer->onStartConsuming();
    }
    else {
        stampPlotConsumer->onStopConsuming();
    }
}

ChessboardController::~ChessboardController() {
    this->clearCurves();
    this->clearPlots();

    /*! Decommentare quando non verrà più distrutto dalla mainwindow */
    delete chessboard;
    chessboard = nullptr;
    mainWindow->setDockWidget(MainWindow::DWChessboard, chessboard);
    if (stampPlotConsumer!= nullptr) {
        stampPlotConsumer->onStopConsuming();
        delete stampPlotConsumer;
        stampPlotConsumer = nullptr;
    }
}

void ChessboardController::clearCurves() {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->detach();
        delete currentCurves[idx];
    }
    currentCurves.clear();
}

void ChessboardController::calibrationResistorsTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StateCalibrationResistorsOn);

        } else {
            plots[channelIdx]->removeState(StampPlot::StateCalibrationResistorsOn);
        }
    }
}

void ChessboardController::offsetRecalibrationTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StateOffsetRecalibrationOn);

        } else {
            plots[channelIdx]->removeState(StampPlot::StateOffsetRecalibrationOn);
        }
    }
}

void ChessboardController::ljcTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StateLiquidJunctionCompensation);

        } else {
            plots[channelIdx]->removeState(StampPlot::StateLiquidJunctionCompensation);
        }
    }
}

void ChessboardController::clearPlots() {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        delete plots[idx];
    }
    plots.clear();
}

void ChessboardController::onChannelsTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateSwitchedOff);
        }
        else {
            plots[channelIdx]->addState(StampPlot::StateSwitchedOff);
        }
    }
}

void ChessboardController::onChannelsTurnedOnOffEx(bool flag) {
    auto selectedChannels = appStatus->getSelectedChannels();
    for (int channelIdx = 0; channelIdx < appStatus->getCurrentChannelsNum(); channelIdx++) {
        if (flag == selectedChannels[channelIdx]) {
            plots[channelIdx]->removeState(StampPlot::StateSwitchedOff);
        }
        else {
            plots[channelIdx]->addState(StampPlot::StateSwitchedOff);
        }
    }
}

void ChessboardController::onCalibrationResistorsTurnedOnOff(bool flag) {
    this->calibrationResistorsTurnedOnOff(flag);
}

void ChessboardController::onStimuliTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateStimuliDisabled);
        }
        else {
            plots[channelIdx]->addState(StampPlot::StateStimuliDisabled);
        }
    }
}

void ChessboardController::onStimuliTurnedOnOffEx(bool flag) {
    auto selectedChannels = appStatus->getSelectedChannels();
    for (int channelIdx = 0; channelIdx < appStatus->getCurrentChannelsNum(); channelIdx++) {
        if (flag == selectedChannels[channelIdx]) {
            plots[channelIdx]->removeState(StampPlot::StateStimuliDisabled);
        }
        else {
            plots[channelIdx]->addState(StampPlot::StateStimuliDisabled);
        }
    }
}

void ChessboardController::onOffsetRecalibrationTurnedOnOff(bool flag) {
    this->offsetRecalibrationTurnedOnOff(flag);
}

void ChessboardController::onLjcTurnedOnOff(bool flag) {
    this->ljcTurnedOnOff(flag);
}

void ChessboardController::onTracesExpandedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StateTraceExpanded);
        }
        else {
            plots[channelIdx]->removeState(StampPlot::StateTraceExpanded);
        }
    }
}

void ChessboardController::onTracesExpandedOnOffEx(bool flag) {
    auto selectedChannels = appStatus->getSelectedChannels();
    for (int channelIdx = 0; channelIdx < appStatus->getCurrentChannelsNum(); channelIdx++) {
        if (flag == selectedChannels[channelIdx]) {
            plots[channelIdx]->addState(StampPlot::StateTraceExpanded);
        }
        else {
            plots[channelIdx]->removeState(StampPlot::StateTraceExpanded);
        }
    }
}

void ChessboardController::onPlotDetailOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StatePlotDetailOn);
        }
        else {
            plots[channelIdx]->removeState(StampPlot::StatePlotDetailOn);
        }
    }
}

void ChessboardController::onPlotDetailOnOffEx(bool flag) {
    auto selectedChannels = appStatus->getSelectedChannels();
    for (int channelIdx = 0; channelIdx < appStatus->getCurrentChannelsNum(); channelIdx++) {
        if (flag == selectedChannels[channelIdx]) {
            plots[channelIdx]->addState(StampPlot::StateTraceExpanded);
        }
        else {
            plots[channelIdx]->removeState(StampPlot::StateTraceExpanded);
        }
    }
}

void ChessboardController::onPlotDetailCreation(std::vector<uint16_t> channels){
    for (auto channelIdx : channels) {
        plots[channelIdx]->addState(StampPlot::StatePlotDetailOn);
    }
}

void ChessboardController::onPlotDetailDeletion(std::vector<uint16_t> channels){
    for (auto channelIdx : channels) {
        plots[channelIdx]->removeState(StampPlot::StatePlotDetailOn);
    }
}

void ChessboardController::onRangeUpdated(std::vector <RangedMeasurement_t> newRange) {
    for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
        plots[chIdx]->onRangeUpdated(newRange[chIdx]);
    }
}

void ChessboardController::onDurationUpdated(Measurement_t duration) {
    for (auto plot : plots) {
        plot->onDurationUpdated(duration);
    }
}

void ChessboardController::onSetPlotData(PlotMessage plotMessage) {
    ClampingModality_t mode;
    appStatus->getMessageDispatcher()->getClampingModality(mode);
    switch (plotMessage.index()) {
    case BigPlot::BigPlotStatus::GapFree: {
        GapFreeMessage message = std::get<BigPlot::BigPlotStatus::GapFree>(plotMessage);
        switch (mode) {
        case e384CommLib::VOLTAGE_CLAMP:
            for (int idx = 0; idx < currentChannelsNum; idx++) {
                currentCurves.at(idx)->setRawSamples(message.timeValues, message.currentValues[idx], message.dataSize);
            }
            break;

        case e384CommLib::CURRENT_CLAMP:
        case e384CommLib::ZERO_CURRENT_CLAMP:
            for (int idx = 0; idx < voltageChannelsNum; idx++) {
                currentCurves.at(idx)->setRawSamples(message.timeValues, message.voltageValues[idx], message.dataSize);
            }
            break;
        }
        break;
    }

    case BigPlot::BigPlotStatus::Iv: {
        IvMessage message = std::get<BigPlot::BigPlotStatus::Iv>(plotMessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves.at(idx)->setRawSamples(message.voltageValues[idx], message.currentValues[idx], message.dataSize[idx]);
        }
        break;
    }

    case BigPlot::BigPlotStatus::Spectrum: {
        SpectrumMessage message = std::get<BigPlot::BigPlotStatus::Spectrum>(plotMessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves.at(idx)->setRawSamples(message.frequencyValues, message.psdValues[idx], message.dataSize);
        }
        break;
    }
    }
}

void ChessboardController::onReplot() {
    for (auto plot : plots) {
        plot->replot();
    }
}

void ChessboardController::onSelectedPlotsUpdated() {
    auto selectedChannels = appStatus->getSelectedChannels();
    for (int i = 0; i < currentChannelsNum; i++) {
        plots[i]->setSelected(selectedChannels[i]);
    }
    chessboard->updateSelectedCounter(appStatus->getSelectedChannelsIndexes().size(), appStatus->getChannels().size());
}

PlotConsumer * ChessboardController::getPlotConsumer(){
    return stampPlotConsumer;
}

std::vector <DeviceDataConsumer*> ChessboardController::getConsumers() {
    return {stampPlotConsumer};
}

void ChessboardController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        plots[idx]->setLegendColor(colors[idx]);
    }
}

void ChessboardController::onCurrentColorChanged(int channelIdx, QColor color) {
    plots[channelIdx]->setLegendColor(color);
}

//void ChessboardController::onBackgroundColorChanged(QColor color) {
//    for (auto plot: plots) {
//        plot->setCanvasBackground(color);
//    }
//}

void ChessboardController::onSyncFaults(std::vector <bool> syncFaultFlags) {
    int boardIdx = 0;
    for (auto f : syncFaultFlags) {
        chessboard->setFaultyBoard(boardIdx++, f);
    }
}

void ChessboardController::onBoardMappingLoaded() {
    updateChessboard();
    appStatus->setAllChannelsSelected(false);
}

void ChessboardController::onSingleChannelClicked(uint16_t chIdx, QMouseEvent *event){
    bool newState = event->button() == Qt::LeftButton;
    clickBehaviour(newState);
    auto msgDisp = appStatus->getMessageDispatcher();
    if (newState) {
        // slightly inefficient
        auto selectedIndexes = appStatus->getSelectedChannelsIndexes();
        bool isChSelected = false;
        for (auto idx: selectedIndexes){
            if (idx == chIdx){
                isChSelected = true;
                break;
            }
        }
        // if the channel is selected but the user is pressing ctrl toggle it
        appStatus->setChannelSelected(chIdx, !((QApplication::keyboardModifiers() & Qt::ControlModifier) && isChSelected));
    }
    else {
        appStatus->setChannelSelected(chIdx, newState);
    }
}

void ChessboardController::onOneBoardClicked(uint16_t brdIdx, bool newState) {
    clickBehaviour(newState);
    setSelectedStatus(appStatus->getVisibleChannelsOnBoard(brdIdx), newState);
}

void ChessboardController::onOneRowClicked(uint16_t rowIdx, bool newState) {
    clickBehaviour(newState);
    setSelectedStatus(appStatus->getVisibleChannelsOnRow(rowIdx), newState);
}

void ChessboardController::onAllChannelsClicked(bool newState) {
    clickBehaviour(newState);
    setSelectedStatus(appStatus->getVisibleChannels(), newState);
}

void ChessboardController::clickBehaviour(bool newState) {
    //    if the newState is false or the user is not pressing ctrl, don't make anything
    if (!newState || (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
        return;
    }
    // Ctrl key is pressed
    appStatus->setAllChannelsSelected(false);
}

void ChessboardController::updateChessboard(){
    const auto mappings = appStatus->getMappings();
    for (int i = 0; i<appStatus->getCurrentChannelsNum(); i++) {
        const auto mapping = mappings[i];
        const auto name = mapping.name;
//        const auto channelIdx = mapping.index;
        const auto visibility = mapping.visible;
        plots[i]->setSelected(false);
        plots[i]->setVisible(visibility);
        plots[i]->setName(name);
    }
    const auto visibleBoards = appStatus->getVisibleBoards();
    chessboard->updateBoardMappings(visibleBoards);
}

void ChessboardController::setSelectedStatus(std::vector<int> channelIndexes, bool newStatus) {
    std::map <int, bool> channelsAndStatus;
    for (auto chIdx: channelIndexes) {
        channelsAndStatus[chIdx] = newStatus;
    }
    appStatus->setSelectedChannels(channelsAndStatus);
}

void ChessboardController::connectSingleChannelController(SingleChannelController* scc) {
    connect(this, &ChessboardController::sigAllChannelsClicked, scc, &SingleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigOneBoardClicked, scc, &SingleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigOneRowClicked, scc, &SingleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigSingleChannelClicked, scc, &SingleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigInvertSelectionClicked, scc, &SingleChannelController::onChannelsSelected);
}

void ChessboardController::connectMultipleChannelController(MultipleChannelController* mcc) {
    connect(this, &ChessboardController::sigAllChannelsClicked, mcc, &MultipleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigOneBoardClicked, mcc, &MultipleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigOneRowClicked, mcc, &MultipleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigSingleChannelClicked, mcc, &MultipleChannelController::onChannelsSelected);
    connect(this, &ChessboardController::sigInvertSelectionClicked, mcc, &MultipleChannelController::onChannelsSelected);
}

void ChessboardController::connectMeasurementOverviewController(MeasurementOverviewController* moc) {
    connect(this, &ChessboardController::sigAllChannelsClicked, moc, &MeasurementOverviewController::onChannelsUpdated);
    connect(this, &ChessboardController::sigOneBoardClicked, moc, &MeasurementOverviewController::onChannelsUpdated);
    connect(this, &ChessboardController::sigOneRowClicked, moc, &MeasurementOverviewController::onChannelsUpdated);
    connect(this, &ChessboardController::sigSingleChannelClicked, moc, &MeasurementOverviewController::onChannelsUpdated);
    connect(this, &ChessboardController::sigInvertSelectionClicked, moc, &MeasurementOverviewController::onChannelsUpdated);
}
