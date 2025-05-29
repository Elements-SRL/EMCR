#include "multiplechannelcontroller.h"

#include <QMessageBox>
#include <QApplication>

#include "errormanager.h"
#include "globaldefines.h"

MultipleChannelController::MultipleChannelController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    msgDisp = appStatus->getMessageDispatcher();
    multipleChannelControlsDw = new MultipleChannelControlDockWidget(msgDisp);
    offsetCorrectionController = new OffsetCorrectionController(appStatus, this);

    model = new MultipleChannelModel(appStatus, multipleChannelControlsDw);

    allChannels.resize(appStatus->getCurrentChannelsNum());
    for (int i = 0; i < allChannels.size(); i++) {
        allChannels[i] = i;
    }

    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnChannelOn, this, [=]() {
        this->turnSelectedChannelsOnOff(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnChannelOff, this, [=]() {
        this->turnSelectedChannelsOnOff(false);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnChannelAuto, this, [=](bool flag) {
        model->turnChannelsAuto(flag);
        this->onChannelsSelected();
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
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigTurnStimulusAuto, this, [=](bool flag) {
        model->turnStimulusAuto(flag);
        this->onChannelsSelected();
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
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigAddToBigPlot,          this, [=] () {
        this->addRemoveFromBigPlot(true);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigRemoveFromBigPlot,     this, [=] () {
        this->addRemoveFromBigPlot(false);
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigAddToBigPlotAuto,     this, [=] (bool flag) {
        model->turnExpandAuto(flag);
        this->onChannelsSelected();
    });
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigAddRemovePlotDetail,   this, &MultipleChannelController::addRemovePlotDetail);
    connect(multipleChannelControlsDw, &MultipleChannelControlDockWidget::sigAddPlotDetailAuto,     this, [=] (bool flag) {
        appStatus->setPlotDetailAuto(flag);
    });

    mainWindow->setDockWidget(MainWindow::DWMultipleChannelControl, multipleChannelControlsDw, false, Qt::RightDockWidgetArea);
}

MultipleChannelController::~MultipleChannelController(){
    delete model;
    model = nullptr;
    delete multipleChannelControlsDw;
    multipleChannelControlsDw = nullptr;
    offsetCorrectionController->wait();
    delete offsetCorrectionController;
    offsetCorrectionController = nullptr;
    mainWindow->setDockWidget(MainWindow::DWMultipleChannelControl, multipleChannelControlsDw);
}

void MultipleChannelController::addRemoveFromBigPlot(bool flag) {
    std::vector <uint16_t> selectedChannels = appStatus->getSelectedChannelsIndexes();
    std::map<uint16_t, bool> flags;
    for (auto &i : selectedChannels) {
        flags[i] = flag;
    }
    appStatus->setExpandedTraces(flags);
    emit sigAddRemoveFromBigPlot(flag);
}

void MultipleChannelController::addRemovePlotDetail(bool flag) {
    std::vector <uint16_t> selectedChannels = appStatus->getSelectedChannelsIndexes();
    std::map<uint16_t, bool> flags;
    for (auto &i : selectedChannels) {
        flags[i] = flag;
    }
    appStatus->setDetailedPlots(flags);
    emit sigAddRemovePlotDetail(flag);
}

void MultipleChannelController::addRemoveFromBigPlotEx(bool flag) {
    const auto selectedChannels = appStatus->getSelectedChannels();
    std::map<uint16_t, bool> expandedChannels;
    for (uint16_t i=0; i< selectedChannels.size(); ++i){
        const auto v = selectedChannels[i];
        expandedChannels[i] = !(v^flag);
    }
    appStatus->setExpandedTraces(expandedChannels);
    emit sigAddRemoveFromBigPlotEx(flag);
}

void MultipleChannelController::onChannelsSelected() {
    if (model->getChannelsAuto()) {
        turnSelectedChannelsOnOffEx(true);
    }
    if (model->getStimulusAuto()) {
        turnSelectedStimuliOnOffEx(true);
    }
    if (model->getExpandAuto()) {
        addRemoveFromBigPlotEx(true);
    }
}

void MultipleChannelController::turnSelectedChannelsOnOff(bool flag) {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), flag);
    msgDisp->turnChannelsOn(selectedChannels, values, true);

    emit sigChannelsTurnedOnOff(flag);
}

void MultipleChannelController::turnSelectedChannelsOnOffEx(bool flag) {
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (auto &&v : selectedChannels) {
        v = !(v^flag);
    }
    msgDisp->turnChannelsOn(allChannels, selectedChannels, true);

    emit sigChannelsTurnedOnOffEx(flag);
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

void MultipleChannelController::turnSelectedStimuliOnOffEx(bool flag) {
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (auto &&v : selectedChannels) {
        v = !(v^flag);
    }
    msgDisp->enableStimulus(allChannels, selectedChannels, true);

    emit sigStimuliTurnedOnOffEx(flag);
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

    case OffsetCorrectionController::CheckingOffsetRecalibration: {
        QMessageBox msgBox(QMessageBox::Information,
                           GLB_SOFTWARE_NAME,
                           "Starting current offset recalibration.\n"
                           "Remove any DUT (e.g. nanopore flowcell, model cell, adaptor PCB, etc.)\n"
                           "from the device's input and click OK.\n"
                           "If the DUT is not removed, the recalibration can add artefacts to the\n"
                           "acquired current.",
                           QMessageBox::Ok | QMessageBox::Cancel,
                           multipleChannelControlsDw);

        msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowCloseButtonHint);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Ok) {
            this->turnSelectedOffsetRecalibrationOnOff(true);
        }
        break;
    }

    case OffsetCorrectionController::CheckingLiquidJunctionCorrection: {
        this->turnSelectedOffsetRecalibrationOnOff(false);
        QMessageBox msgBox(QMessageBox::Information,
                           GLB_SOFTWARE_NAME,
                           "Starting liquid junction compensation.\n"
                           "Insert the DUT (e.g. nanopore flowcell, model cell, adaptor PCB, etc.)\n"
                           "into the device's input and click OK.\n"
                           "If the DUT is not plugged, the compensation can add artefacts to the\n"
                           "applied voltage and thus to the acquired current.",
                           QMessageBox::Ok | QMessageBox::Cancel,
                           multipleChannelControlsDw);

        msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowCloseButtonHint);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Ok) {
            this->turnSelectedLjcOnOff(true);
        }
        break;
    }
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
