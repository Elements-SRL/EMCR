#ifndef CONTROLLERBOARD_H
#define CONTROLLERBOARD_H

#include <QObject>
#include<QDebug>

#include "modeldevice.h"
#include "boardcontroldockwidget.h"

class ControllerBoard : public QObject {
    Q_OBJECT

public:
    ControllerBoard(ModelDevice * mDev);
    BoardControlDockWidget * getBoardControlDockWidget();
private:
    ModelDevice * mDev = nullptr;
    BoardControlDockWidget * boardControlDockWidget = nullptr;

public slots:
    void onGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);

signals:
    void sigGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);
};

#endif // CONTROLLERBOARD_H
