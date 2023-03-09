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

};
#endif // CONTROLLERDEVICE_H
