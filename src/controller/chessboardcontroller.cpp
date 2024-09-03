#include "chessboardcontroller.h"
#include <iostream>

ChessboardController::ChessboardController(ApplicationStatus * appStatus, PlotConsumer * plotConsumer, Measurement_t defaultDuration, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    stampPlotConsumer = plotConsumer;
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    channels = appStatus->getChannels();

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
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    stampPlotConsumer->onPlotChannels(allChannels, true);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::sigAllChannelsClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::sigOneBoardClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::sigOneRowClicked);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::sigSingleChannelClicked);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::onSelectedPlotsUpdated);

    connect(stampPlotConsumer, &PlotConsumer::setPlotData,              this,       &ChessboardController::onSetPlotData);
    connect(stampPlotConsumer, &PlotConsumer::plotDataUpdated,          this,       &ChessboardController::onReplot);
    connect(chessboard, &QDockWidget::visibilityChanged,                this,       &ChessboardController::onSetConsumerStatus);
    mainWindow->setDockWidget(MainWindow::DWChessboard, chessboard, true, Qt::LeftDockWidgetArea);
    onSelectedPlotsUpdated();
}

void ChessboardController::onSetConsumerStatus(bool status) {
    if (status) {
        stampPlotConsumer->onStartConsuming();
    } else {
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

void ChessboardController::channelsTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateSwitchedOff);

        } else {
            plots[channelIdx]->addState(StampPlot::StateSwitchedOff);
        }
    }
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

void ChessboardController::stimuliTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateStimuliDisabled);

        } else {
            plots[channelIdx]->addState(StampPlot::StateStimuliDisabled);
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

void ChessboardController::tracesExpandedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StateTraceExpanded);

        } else {
            plots[channelIdx]->removeState(StampPlot::StateTraceExpanded);
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
    this->channelsTurnedOnOff(flag);
}

void ChessboardController::onCalibrationResistorsTurnedOnOff(bool flag) {
    this->calibrationResistorsTurnedOnOff(flag);
}

void ChessboardController::onStimuliTurnedOnOff(bool flag) {
    this->stimuliTurnedOnOff(flag);
}

void ChessboardController::onOffsetRecalibrationTurnedOnOff(bool flag) {
    this->offsetRecalibrationTurnedOnOff(flag);
}

void ChessboardController::onLjcTurnedOnOff(bool flag) {
    this->ljcTurnedOnOff(flag);
}

void ChessboardController::onTracesExpandedOnOff(bool flag) {
    this->tracesExpandedOnOff(flag);
}

void ChessboardController::onRangeUpdated(RangedMeasurement_t newRange) {
    for (auto plot : plots) {
        plot->onRangeUpdated(newRange);
    }
}

void ChessboardController::onDurationUpdated(Measurement_t duration) {
    for (auto plot : plots) {
        plot->onDurationUpdated(duration);
    }
}

void ChessboardController::onSetPlotData(PlotMessage plotMessage) {
    switch (plotMessage.index()) {
    //    GapFree message
    case 0:{
        GapFreeMessage message = std::get<0>(plotMessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves.at(idx)->setRawSamples(message.timeValues, message.currentValues[idx], message.dataSize);
        }
        break;
    }
        //    IvGraph message
    case 1:{
        IvMessage message = std::get<1>(plotMessage);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves.at(idx)->setRawSamples(message.voltageValues[idx], message.currentValues[idx], message.dataSize[idx]);
        }
        break;
    }
        //    IvGraph message
    case 3:{
        SpectrumMessage message = std::get<3>(plotMessage);
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
    for(int i = 0; i < currentChannelsNum; i++){
        plots[i]->setSelected(selectedChannels[i]);
    }
}

PlotConsumer * ChessboardController::getPlotConsumer(){
    return stampPlotConsumer;
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

void ChessboardController::onBoardMappingLoaded() {
    updateChessboard();
    appStatus->setAllChannelsSelected(false);
}

void ChessboardController::updateChessboard(){
    const auto mappings = appStatus->getMappings();
    for(int i = 0; i<appStatus->getCurrentChannelsNum(); i++){
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
