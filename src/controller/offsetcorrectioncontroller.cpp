#include "offsetcorrectioncontroller.h"

#include "messagedispatcher.h"

using namespace e384CommLib;

OffsetCorrectionController::OffsetCorrectionController(ApplicationStatus * appStatus, QObject * parent) :
    QThread(parent),
    appStatus(appStatus) {

}

void OffsetCorrectionController::onStartChecking(OffsetCorrectionCheck_t check) {
    if (offsetCheck != CheckingNone) {
        return;
    }
    offsetCheck = check;
    this->start();
}

void OffsetCorrectionController::run() {

}
