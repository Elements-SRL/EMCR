#include "multiplechannelcontroller.h"

#include <QMessageBox>
#include <QApplication>

#include "errormanager.h"

MultipleChannelController::MultipleChannelController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    msgDisp = appStatus->getMessageDispatcher();
    multipleChannelControlsDw = new MultipleChannelControlDockWidget(msgDisp);
    offsetCorrectionController = new OffsetCorrectionController(appStatus, this);

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

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigZap, this, [=](Measurement_t duration) {
        this->zap(duration);
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigStartOffsetCorrection, this, [=]() {
        multipleChannelControlsDw->enableExpertMode(false);
        this->offsetCorrection(OffsetCorrectionController::CheckingOffsetRecalibration);
    });
    connect(offsetCorrectionController, &OffsetCorrectionController::sigTaskPerformed, this, [=] (OffsetCorrectionController::OffsetCorrectionCheck_t step) {
        if (!multipleChannelControlsDw->getExpertMode()) {
            switch (step) {
            case OffsetCorrectionController::CheckingNone:
                /*! shouldn't happen*/
                break;

            case OffsetCorrectionController::CheckingOffsetRecalibration:
                this->offsetCorrection(OffsetCorrectionController::CheckingLiquidJunctionCorrection);
                break;

            case OffsetCorrectionController::CheckingLiquidJunctionCorrection:
                this->offsetCorrection(OffsetCorrectionController::CheckingNone);
                multipleChannelControlsDw->enableExpertMode(true);
                break;
            }

        } else {
            switch (step) {
            case OffsetCorrectionController::CheckingNone:
                /*! shouldn't happen*/
                break;

            case OffsetCorrectionController::CheckingOffsetRecalibration:
                this->turnSelectedOffsetRecalibrationOnOff(false);
                break;

            case OffsetCorrectionController::CheckingLiquidJunctionCorrection:
                this->turnSelectedLjcOnOff(false);
                break;
            }
            multipleChannelControlsDw->enableExpertMode(true);
        }
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigStopOffsetCorrection, this, [=]() {
        multipleChannelControlsDw->enableExpertMode(true);
//        this->offsetCorrection(OffsetCorrectionController::CheckingOffsetRecalibration);
        this->turnSelectedOffsetRecalibrationOnOff(false);
        this->turnSelectedLjcOnOff(false);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOn, this, [=]() {
        multipleChannelControlsDw->enableExpertMode(false);
        this->turnSelectedOffsetRecalibrationOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOff, this, [=]() {
        multipleChannelControlsDw->enableExpertMode(true);
        this->turnSelectedOffsetRecalibrationOnOff(false);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigResetOffsetRecalibration, this, [=]() {
        this->resetOffsetRecalibration();
    });

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnLjcOn, this, [=]() {
        multipleChannelControlsDw->enableExpertMode(false);
        this->turnSelectedLjcOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnLjcOff, this, [=]() {
        multipleChannelControlsDw->enableExpertMode(true);
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
    offsetCorrectionController->wait();
    delete offsetCorrectionController;
    offsetCorrectionController = nullptr;
    mainWindow->setMultipleChannelControlsDw(multipleChannelControlsDw);
}

void MultipleChannelController::addRemoveFromBigPlot(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->expandTraces(selectedChannels, values);

    emit sigAddRemoveFromBigPlot(flag);
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

void MultipleChannelController::zap(Measurement_t duration) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    msgDisp->zap(selectedChannels, duration);
}

void MultipleChannelController::offsetCorrection(OffsetCorrectionController::OffsetCorrectionCheck_t step) {
    switch (step) {
    case OffsetCorrectionController::CheckingNone:
        this->turnSelectedLjcOnOff(false);
        QMessageBox::information(multipleChannelControlsDw,
                                 GLB_SOFTWARE_NAME,
                                 "Offset correction procedure finished.\n"
                                 "The results are available in the Measurement overview widget.");
        break;

    case OffsetCorrectionController::CheckingOffsetRecalibration:
        QMessageBox::information(multipleChannelControlsDw,
                                 GLB_SOFTWARE_NAME,
                                 "Starting current offset recalibration.\n"
                                 "Remove any load from the device's input and click OK.");
        this->turnSelectedOffsetRecalibrationOnOff(true);
        break;

    case OffsetCorrectionController::CheckingLiquidJunctionCorrection:
        this->turnSelectedOffsetRecalibrationOnOff(false);
        QMessageBox::information(multipleChannelControlsDw,
                                 GLB_SOFTWARE_NAME,
                                 "Starting liquid junction compensation.\n"
                                 "Insert the DUT into the device's input and click OK.");
        this->turnSelectedLjcOnOff(true);
        break;
    }
}

void MultipleChannelController::turnSelectedOffsetRecalibrationOnOff(bool flag) {
    if (flag) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        offsetCorrectionController->onStartChecking(OffsetCorrectionController::CheckingOffsetRecalibration);

    } else {
        QApplication::restoreOverrideCursor();
    }
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
    msgDisp->resetOffsetRecalibration(selectedChannels, true);

    emit sigOffsetRecalibrationResetted();
}

void MultipleChannelController::turnSelectedLjcOnOff(bool flag) {
    if (flag) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        offsetCorrectionController->onStartChecking(OffsetCorrectionController::CheckingLiquidJunctionCorrection);

    } else {
        QApplication::restoreOverrideCursor();
    }
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    ErrorCodes_t err = msgDisp->liquidJunctionCompensation(selectedChannels, values, true);
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
