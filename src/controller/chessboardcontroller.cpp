#include "chessboardcontroller.h"

ChessboardController::ChessboardController(ApplicationStatus * appStatus, GapFreePlotConsumer * plotConsumer, Measurement_t defaultDuration, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    stampPlotConsumer = plotConsumer;
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    channels = appStatus->getChannels();

    chessboard = new ChessboardDockWidget(voltageChannelsNum, currentChannelsNum, appStatus->getBoardsNum(), mainWindow);;

    int idealPlotWidth = chessboard->getIdealPlotWidth();
    int idealPlotHeight = chessboard->getIdealPlotHeight();

    plots.resize(currentChannelsNum);
    currentCurves.resize(currentChannelsNum);

//    for(auto chAndName: appStatus->getChannelsAndNames()){
//        /*! buttare in una funzioncina di creazione del plot*/
//        auto channelIdx = chAndName.channelIndex;
//        StampPlot * plot = new StampPlot(channelIdx, chAndName.name, idealPlotWidth, idealPlotHeight);
//        plot->setFixedSize(idealPlotWidth, idealPlotHeight);
//        plot->setToolTip(QString("Ch %1\n"
//                                 "Left click: exclusive select\n"
//                                 "CTRL + Left click: append\n"
//                                 "Right click: deselect").arg(QString::fromStdString(chAndName.name)));
//        plot->setSelected(false);

//        plots[channelIdx] = plot;

//        Curve * curve = new Curve(CurveType_t::CurveTypeStampPlotSolid);
//        curve->attach(plot);
//        currentCurves[channelIdx] = curve;

//        chessboard->addPlot(plot, channelIdx);
//    };

    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        /*! buttare in una funzioncina di creazione del plot*/
        StampPlot * plot = new StampPlot(channelIdx, std::to_string(channelIdx), idealPlotWidth, idealPlotHeight);
        plot->setFixedSize(idealPlotWidth, idealPlotHeight);
        plot->setToolTip(QString("Ch %1\n"
                                 "Left click: exclusive select\n"
                                 "CTRL + Left click: append\n"
                                 "Right click: deselect").arg(channelIdx+1));
        plot->setSelected(false);

        plots[channelIdx] = plot;

        Curve * curve = new Curve(CurveType_t::CurveTypeStampPlotSolid);
        curve->attach(plot);
        currentCurves[channelIdx] = curve;

        chessboard->addPlot(plot, channelIdx);
    }

//    setting initial values for stampPlotConsumer
    stampPlotConsumer->onDurationChanged(defaultDuration);
    stampPlotConsumer->forceAxisUpdate();
    stampPlotConsumer->setMaxSamplesPerPlot(256);
    stampPlotConsumer->onSelectChannels(true);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::sigAllChannelsClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::sigOneBoardClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::sigOneRowClicked);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::sigSingleChannelClicked);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::onSelectedPlotsUpdated);

    connect(stampPlotConsumer, &GapFreePlotConsumer::setPlotData,       this,       &ChessboardController::onSetGapFreePlotData);
    connect(stampPlotConsumer, &GapFreePlotConsumer::plotDataUpdated,   this,       &ChessboardController::onReplot);
    connect(chessboard, &QDockWidget::visibilityChanged,                this,       &ChessboardController::onSetConsumerStatus);
    mainWindow->setChessboardDw(chessboard);
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
    mainWindow->setChessboardDw(chessboard);
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
    for (auto channelIdx : appStatus->getSelectedChannels()) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateSwitchedOff);

        } else {
            plots[channelIdx]->addState(StampPlot::StateSwitchedOff);
        }
    }
}

void ChessboardController::stimuliTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannels()) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateStimuliDisabled);

        } else {
            plots[channelIdx]->addState(StampPlot::StateStimuliDisabled);
        }
    }
}

void ChessboardController::docTurnedOnOff(bool flag) {
    for (auto channelIdx : appStatus->getSelectedChannelsIndexes()) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StateOffsetCompensation);

        } else {
            plots[channelIdx]->removeState(StampPlot::StateOffsetCompensation);
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

void ChessboardController::onStimuliTurnedOnOff(bool flag) {
    this->stimuliTurnedOnOff(flag);
}

void ChessboardController::onDocTurnedOnOff(bool flag) {
    this->docTurnedOnOff(flag);
}

void ChessboardController::onTracesExpandedOnOff(bool flag) {
    this->tracesExpandedOnOff(flag);
}

void ChessboardController::onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    for (auto plot : plots) {
        plot->onRangeUpdated(newRange, axisIdx);
    }
}

void ChessboardController::onDurationUpdated(Measurement_t duration) {
    for (auto plot : plots) {
        plot->onDurationUpdated(duration);
    }
}

void ChessboardController::onSetGapFreePlotData(double * timeValues, QVector <double *> *, QVector <double *> * currentValues, int dataSize) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves.at(idx)->setRawSamples(timeValues, currentValues->at(idx), dataSize);
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
