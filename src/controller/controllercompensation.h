#ifndef CONTROLLERCOMPENSATION_H
#define CONTROLLERCOMPENSATION_H

#include <QObject>
#include <QDebug>

#include "modeldevice.h"
#include "compensationcontroldockwidget.h"

class ControllerCompensation : public QObject {
    Q_OBJECT

public:
    ControllerCompensation(ModelDevice * mDev);
    CompensationControlDockWidget * getCompensationDockWidget();

private:
    ModelDevice * mDev = nullptr;
    CompensationControlDockWidget * compensationControlDockWidget = nullptr;
};
#endif // CONTROLLERCOMPENSATION_H
