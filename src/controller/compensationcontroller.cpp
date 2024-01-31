#include "compensationcontroller.h"

CompensationController::CompensationController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    if (msgDisp->hasCompFeature(MessageDispatcher::U_CpVc) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_Cm) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_Rs) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_RsCp) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_RsPg) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::CompRsCorr) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_CpCc) != Success) {
        return;
    }

    compensationControlDockWidget = new CompensationControlDockWidget(msgDisp);
    mainWindow->setCompensationControlsDw(compensationControlDockWidget);
}

CompensationController::~CompensationController() {
    delete compensationControlDockWidget;
    compensationControlDockWidget = nullptr;
    mainWindow->setCompensationControlsDw(compensationControlDockWidget);
}
