#include "chessboardcontroller.h"

ChessboardController::ChessboardController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    chessboard = new ChessboardDockWidget(msgDisp);;

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    msgDisp->getChannels(channels);
    int idealPlotWidth = chessboard->getIdealPlotWidth();
    int idealPlotHeight = chessboard->getIdealPlotHeight();

    plots.resize(currentChannelsNum);
    currentCurves.resize(currentChannelsNum);

    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        /*! buttare in una funzioncina di creazione del plot*/
        StampPlot * plot = new StampPlot(channelIdx, idealPlotWidth, idealPlotHeight);
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

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::sigAllChannelsClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::sigOneBoardClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::sigOneRowClicked);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::sigSingleChannelClicked);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::onSelectedPlotsUpdated);
    connect(chessboard, &ChessboardDockWidget::sigUpdateConsumer,       this,       &ChessboardController::onConsumerUpdated);
    mainWindow->setChessboardDw(chessboard);
}

ChessboardController::~ChessboardController() {
    this->clearCurves();
    this->clearPlots();

    /*! Decommentare quando non verrà più distrutto dalla mainwindow */
    delete chessboard;
    chessboard = nullptr;
    mainWindow->setChessboardDw(chessboard);
}

void ChessboardController::clearCurves() {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->detach();
        delete currentCurves[idx];
    }
    currentCurves.clear();
}

void ChessboardController::channelsTurnedOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);

    for (auto channelIdx : selectedChannels) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateSwitchedOff);

        } else {
            plots[channelIdx]->addState(StampPlot::StateSwitchedOff);
        }
    }
}

void ChessboardController::stimuliTurnedOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);

    for (auto channelIdx : selectedChannels) {
        if (flag) {
            plots[channelIdx]->removeState(StampPlot::StateStimuliDisabled);

        } else {
            plots[channelIdx]->addState(StampPlot::StateStimuliDisabled);
        }
    }
}

void ChessboardController::docTurnedOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);

    for (auto channelIdx : selectedChannels) {
        if (flag) {
            plots[channelIdx]->addState(StampPlot::StateOffsetCompensation);

        } else {
            plots[channelIdx]->removeState(StampPlot::StateOffsetCompensation);
        }
    }
}

void ChessboardController::tracesExpandedOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);

    for (auto channelIdx : selectedChannels) {
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
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for(int ii = 0; ii < currentChannelsNum; ii++){
        plots[ii]->setSelected(selectedChannels[ii]);
    }
}

void ChessboardController::onConsumerUpdated(bool flag) {
    if (flag) {

    }
}
