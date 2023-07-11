#include "stampplotcontroller.h"

StampPlotController::StampPlotController(MessageDispatcher * msgDisp, ChessboardDockWidget * chessboardDw) :
    msgDisp(msgDisp) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    msgDisp->getChannels(channels);
    int idealPlotWidth = chessboardDw->getIdealPlotWidth();
    int idealPlotHeight = chessboardDw->getIdealPlotHeight();

    plots.resize(currentChannelsNum);
    currentCurves.resize(currentChannelsNum);

    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        /*! buttare in una funzioncina di creazione del plot*/
        StampPlot * plot = new StampPlot(channelIdx, idealPlotWidth, idealPlotHeight);
        plot->setFixedSize(idealPlotWidth, idealPlotHeight);
        plot->setToolTip(QString("Ch %1\nRight click: select\nLeft click: deselect").arg(channelIdx+1));
        plot->setSelected(false);

        plots[channelIdx] = plot;

        Curve * curve = new Curve(CurveType_t::CurveTypeStampPlotSolid);
        curve->attach(plot);
        currentCurves[channelIdx] = curve;

        chessboardDw->addPlot(plot, channelIdx);
    }
}

void StampPlotController::clearCurves() {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->detach();
        delete currentCurves[idx];
        delete [] currentCurves[idx];
    }
    currentCurves.clear();
}

void StampPlotController::channelsTurnedOnOff(bool flag) {
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

void StampPlotController::stimuliTurnedOnOff(bool flag) {
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

void StampPlotController::docTurnedOnOff(bool flag) {
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

void StampPlotController::onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    for (auto plot : plots) {
        plot->onRangeUpdated(newRange, axisIdx);
    }
}

void StampPlotController::onDurationUpdated(Measurement_t duration) {
    for (auto plot : plots) {
        plot->onDurationUpdated(duration);
    }
}

/*! channelsToPlotNumber is ignored by the chessBoard*/
void StampPlotController::onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves.at(idx)->setRawSamples(timeValues, currentValues->at(idx), dataSize);
    }
}

void StampPlotController::onReplot() {
    for (auto plot : plots) {
        plot->replot();
    }
}

void StampPlotController::onSelectedPlotsUdpated() {
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for(int ii = 0; ii < currentChannelsNum; ii++){
        plots[ii]->setSelected(selectedChannels[ii]);
    }
}
