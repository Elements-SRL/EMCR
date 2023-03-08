#ifndef CONTROLLERDEVICE_H
#define CONTROLLERDEVICE_H

#include <QObject>
#include <QDebug>

#include "modeldevice.h"

class ControllerDevice : public QObject {
    Q_OBJECT

public:
    ControllerDevice(ModelDevice * mDev);

    void setModelDevice(ModelDevice * mDev);

private:
    ModelDevice * mDev = nullptr;

public slots:
    void onVcCurrentRangeSelected(uint16_t selectedVcCurrentRangeIndex);
    void onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex);
    void onVcVoltageFilterSelected(uint16_t selectedVcVoltageFilterIndex);
    void onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex);

    signals:
    void sigVcCurrentRangeSelected(uint16_t selectedVcCurrentRangeIndex);
    void sigCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex);
    void sigVcVoltageFilterSelected(uint16_t selectedVcVoltageRangeIndex);
    void sigCcCurrentFilterSelected(uint16_t selectedCcCurrentRangeIndex);

};
#endif // CONTROLLERDEVICE_H
