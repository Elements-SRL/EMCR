#include "multiplechannelcontroller.h"

MultipleChannelController::MultipleChannelController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    multipleChannelControlsDw = new MultipleChannelControlDockWidget(msgDisp);

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnChannelOn, this, [=]() {
        this->turnSelectedChannelsOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnChannelOff, this, [=]() {
        this->turnSelectedChannelsOnOff(false);
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnStimulsOn, this, [=]() {
        this->turnSelectedStimuliOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnStimulsOff, this, [=]() {
        this->turnSelectedStimuliOnOff(false);
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnDocOn, this, [=]() {
        this->turnSelectedDocOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnDocOff, this, [=]() {
        this->turnSelectedDocOnOff(false);
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigStartRecording,        this, [=] () {
        this->onRecordingRequest(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigStopRecording,         this, [=] () {
        this->onRecordingRequest(false);
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigAddToBigPlot,          this, [=] () {
        this->addRemoveFromBigPlot(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigRemoveFromBigPlot,     this, [=] () {
        this->addRemoveFromBigPlot(false);
    });

    mainWindow->setMultipleChannelControlsDw(multipleChannelControlsDw);
}

void MultipleChannelController::onRecordingRequest(bool flag) {
    if (flag) {
        std::vector <uint16_t> selectedChannels;
        msgDisp->getSelectedChannelsIndexes(selectedChannels);

        if (!(selectedChannels.empty())) {
            emit sigStartRecording();

        } else {
            QString err = "Recording to file not possible";
            QString info = "No channel checked for recording";
            ErrorManager e(err, info);
        }

    } else {
        emit sigStopRecording();
    }
}

void MultipleChannelController::onRecordingExecution(bool flag) {
    multipleChannelControlsDw->setRecording(flag);
}

void MultipleChannelController::turnSelectedChannelsOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->turnChannelsOn(selectedChannels, values, true);

    emit sigChannelsTurnedOnOff(flag);
}

void MultipleChannelController::turnSelectedStimuliOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->enableStimulus(selectedChannels, values, true);

    emit sigStimuliTurnedOnOff(flag);
}

void MultipleChannelController::turnSelectedDocOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->digitalOffsetCompensation(selectedChannels, values, true);

    emit sigDocTurnedOnOff(flag);
}

void MultipleChannelController::addRemoveFromBigPlot(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->expandTraces(selectedChannels, values);

    emit sigAddRemoveFromBigPlot(flag);
}
