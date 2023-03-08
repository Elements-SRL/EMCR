#ifndef CONTROLLERBOARD_H
#define CONTROLLERBOARD_H

#include <QObject>

#include "modeldevice.h"

class ControllerBoard : public QObject {
    Q_OBJECT

public:
    ControllerBoard(ModelDevice * mDev);

    void setModelDevice(ModelDevice * mDev);

private:
    ModelDevice * mDev = nullptr;

public slots:
    void onGateSourceVoltagesApplied(vector<uint16_t> gateVoltageBoardIndexes, vector<Measurement_t> gateVoltages, vector<uint16_t> sourceVoltageBoardIndexes, vector<Measurement_t> sourceVoltages);


    signals:
    void sigGateSourceVoltagesApplied(vector<uint16_t> gateVoltageBoardIndexes, vector<Measurement_t> gateVoltages, vector<uint16_t> sourceVoltageBoardIndexes, vector<Measurement_t> sourceVoltages);


};

#endif // CONTROLLERBOARD_H
