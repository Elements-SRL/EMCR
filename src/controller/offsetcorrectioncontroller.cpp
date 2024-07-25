#include "offsetcorrectioncontroller.h"

#include "messagedispatcher.h"

using namespace e384CommLib;

OffsetCorrectionController::OffsetCorrectionController(ApplicationStatus * appStatus, QObject * parent) :
    QThread(parent),
    appStatus(appStatus) {

    msgDisp = appStatus->getMessageDispatcher();
}

void OffsetCorrectionController::onStartChecking(OffsetCorrectionCheck_t check) {
    if (offsetCheck != CheckingNone) {
        return;
    }
    offsetCheck = check;
    this->start();
}

void OffsetCorrectionController::run() {
    if (offsetCheck == CheckingOffsetRecalibration) {
        this->checkOffsetRecalibration();

    } else if (offsetCheck == CheckingLiquidJunctionCorrection) {
        this->checkLiquidJunctioncorrection();
    }
    offsetCheck = CheckingNone;
    emit sigTaskPerformed();
}

void OffsetCorrectionController::checkOffsetRecalibration() {
    bool taskPerformed = false;

    std::vector <uint16_t> channels = appStatus->getOffsetRecalibratingChannelsIndexes();
    std::vector <OffsetRecalibStatus_t> statuses;

    while (!taskPerformed) {
        QThread::msleep(500);
        taskPerformed = true;
        msgDisp->getReadoutOffsetRecalibrationStatuses(channels, statuses);
        for (auto status : statuses) {
            switch (status) {
            case OffsetRecalibNotPerformed:
            case OffsetRecalibExecuting:
            case OffsetRecalibResetted:
                taskPerformed = false;
                break;

            case OffsetRecalibSucceded:
            case OffsetRecalibInterrupted:
            case OffsetRecalibFailed:
                break;
            }
        }
    }
}

void OffsetCorrectionController::checkLiquidJunctioncorrection() {
    bool taskPerformed = false;

    std::vector <uint16_t> channels = appStatus->getLiquidJunctionCompensatingChannelsIndexes();
    std::vector <LiquidJunctionStatus_t> statuses;

    while (!taskPerformed) {
        QThread::msleep(500);
        taskPerformed = true;
        msgDisp->getLiquidJunctionStatuses(channels, statuses);
        for (auto status : statuses) {
            switch (status) {
            case LiquidJunctionNotPerformed:
            case LiquidJunctionExecuting:
            case LiquidJunctionResetted:
                taskPerformed = false;
                break;

            case LiquidJunctionSucceded:
            case LiquidJunctionInterrupted:
            case LiquidJunctionFailedOpenCircuit:
            case LiquidJunctionFailedSaturation:
            case LiquidJunctionFailedTooManySteps:
                break;
            }
        }
    }
}
