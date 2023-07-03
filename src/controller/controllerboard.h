#ifndef CONTROLLERBOARD_H
#define CONTROLLERBOARD_H

#include <QObject>
#include<QDebug>

#include "messagedispatcher.h"
#include "boardcontroldockwidget.h"
#include "mainwindow.h"

class ControllerBoard : public QObject {
    Q_OBJECT

public:
    ControllerBoard(MessageDispatcher * msgDisp, MainWindow * mainWindow);

private:
    MessageDispatcher * msgDisp = nullptr;
    BoardControlDockWidget * boardControlDockWidget = nullptr;
    MainWindow * mainWindow = nullptr;

public slots:
    void onGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);

signals:
    void sigGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);
};

#endif // CONTROLLERBOARD_H
