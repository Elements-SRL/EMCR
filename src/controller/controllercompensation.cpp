#include "controllercompensation.h"

ControllerCompensation::ControllerCompensation(ModelDevice * mDev, MainWindow * mainWindow) :
    mDev(mDev)
{
    compensationControlDockWidget = new CompensationControlDockWidget(mDev);
    this->mainWindow = mainWindow;
    mainWindow->setCompensationControlsDw(compensationControlDockWidget);
}
