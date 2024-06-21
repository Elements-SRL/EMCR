#include "multiplechannelcontroller.h"

#include "errormanager.h"

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

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnCalibrationResistorsOn, this, [=]() {
        this->turnSelectedCalibrationResistorsOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnCalibrationResistorsOff, this, [=]() {
        this->turnSelectedCalibrationResistorsOnOff(false);
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnStimulsOn, this, [=]() {
        this->turnSelectedStimuliOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnStimulsOff, this, [=]() {
        this->turnSelectedStimuliOnOff(false);
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOn, this, [=]() {
        this->turnSelectedOffsetRecalibrationOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOff, this, [=]() {
        this->turnSelectedOffsetRecalibrationOnOff(false);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigResetOffsetRecalibration, this, [=]() {
        this->resetOffsetRecalibration();
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnLjcOn, this, [=]() {
        this->turnSelectedLjcOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnLjcOff, this, [=]() {
        this->turnSelectedLjcOnOff(false);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigResetLj, this, [=]() {
        this->resetLj();
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

MultipleChannelController::~MultipleChannelController(){
    delete multipleChannelControlsDw;
    multipleChannelControlsDw = nullptr;
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

void MultipleChannelController::turnSelectedCalibrationResistorsOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->turnCalSwOn(selectedChannels, values, true);

    emit sigCalibrationResistorsTurnedOnOff(flag);
}

void MultipleChannelController::turnSelectedStimuliOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->enableStimulus(selectedChannels, values, true);

    emit sigStimuliTurnedOnOff(flag);
}

void MultipleChannelController::turnSelectedOffsetRecalibrationOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    ErrorCodes_t err = msgDisp->readoutOffsetRecalibration(selectedChannels, values, true);
    if (err == Success) {
        emit sigOffsetRecalibrationTurnedOnOff(flag);

    } else {
        ErrorManager e(err);
    }
}

void MultipleChannelController::resetOffsetRecalibration() {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
//    msgDisp->resetLiquidJunctionVoltage(selectedChannels, true);

    emit sigOffsetRecalibrationResetted();
}

void MultipleChannelController::turnSelectedLjcOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    ErrorCodes_t err = msgDisp->digitalOffsetCompensation(selectedChannels, values, true);
    if (err == Success) {
        emit sigLjcTurnedOnOff(flag);

    } else {
        ErrorManager e(err);
    }
}

void MultipleChannelController::resetLj() {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    msgDisp->resetLiquidJunctionVoltage(selectedChannels, true);

    emit sigLjResetted();
}

void MultipleChannelController::addRemoveFromBigPlot(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->expandTraces(selectedChannels, values);

    emit sigAddRemoveFromBigPlot(flag);
}
