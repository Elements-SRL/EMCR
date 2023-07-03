#include "controllercompensation.h"

ControllerCompensation::ControllerCompensation(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp) {

    compensationControlDockWidget = new CompensationControlDockWidget(msgDisp);
    this->mainWindow = mainWindow;
    mainWindow->setCompensationControlsDw(compensationControlDockWidget);
}
