#include "controllercompensation.h"

ControllerCompensation::ControllerCompensation(ModelDevice * mDev) :
    mDev(mDev)
{
    compensationControlDockWidget = new CompensationControlDockWidget(mDev);
}

CompensationControlDockWidget * ControllerCompensation::getCompensationDockWidget(){
    return compensationControlDockWidget;
}
