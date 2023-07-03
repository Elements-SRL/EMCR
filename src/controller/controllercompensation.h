#ifndef CONTROLLERCOMPENSATION_H
#define CONTROLLERCOMPENSATION_H

#include <QObject>
#include <QDebug>

#include "messagedispatcher.h"
#include "compensationcontroldockwidget.h"
#include "mainwindow.h"

class ControllerCompensation : public QObject {
    Q_OBJECT

public:
    ControllerCompensation(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    CompensationControlDockWidget * getCompensationDockWidget();

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    CompensationControlDockWidget * compensationControlDockWidget = nullptr;
};
#endif // CONTROLLERCOMPENSATION_H
