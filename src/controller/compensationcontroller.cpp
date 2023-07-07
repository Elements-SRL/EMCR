#include "compensationcontroller.h"

CompensationController::CompensationController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp) {

    compensationControlDockWidget = new CompensationControlDockWidget(msgDisp);
    this->mainWindow = mainWindow;
    mainWindow->setCompensationControlsDw(compensationControlDockWidget);
}
