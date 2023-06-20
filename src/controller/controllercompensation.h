#ifndef CONTROLLERCOMPENSATION_H
#define CONTROLLERCOMPENSATION_H

#include <QObject>
#include <QDebug>

#include "modeldevice.h"
#include "compensationcontroldockwidget.h"
#include "mainwindow.h"

class ControllerCompensation : public QObject {
    Q_OBJECT

public:
    ControllerCompensation(ModelDevice * mDev, MainWindow * mainWindow);
    CompensationControlDockWidget * getCompensationDockWidget();

private:
    ModelDevice * mDev = nullptr;
    MainWindow * mainWindow = nullptr;
    CompensationControlDockWidget * compensationControlDockWidget = nullptr;
};
#endif // CONTROLLERCOMPENSATION_H
