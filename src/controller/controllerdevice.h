#ifndef CONTROLLERDEVICE_H
#define CONTROLLERDEVICE_H

#include <QObject>
#include <QDebug>

#include "modeldevice.h"
#include "devicecontroldockwidget.h"
#include "mainwindow.h"

class ControllerDevice : public QObject {
    Q_OBJECT

public:
    ControllerDevice(ModelDevice * mDev, MainWindow * mainWindow);

public slots:
    void onVcCurrentRangeSelected(uint16_t selectedVcCurrentRangeIndex);
    void onVcVoltageRangeSelected(uint16_t selectedVcCurrentRangeIndex);
    void onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex);
    void onCcCurrentRangeSelected(uint16_t selectedCcVoltageRangeIndex);
    void onVcVoltageFilterSelected(uint16_t selectedVcVoltageFilterIndex);
    void onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex);
    void onSamplingRateSelected(uint16_t selectedCcCurrentFilterIndex);
    void onDownsamplingRatioSelected(uint16_t selectedDownsamplingRatioIndex);
    void onClampingModalitySelected(uint16_t selectedClampingModalityIndex);

private:
    ModelDevice * mDev = nullptr;
    MainWindow * mainWindow;
    DeviceControlDockWidget * deviceControlDockWidget = nullptr;

signals:
    void sigVcCurrentRangeSelected(int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int idx);
    void sigVcVoltageFilterSelected(int idx);
    void sigCcCurrentFilterSelected(int idx);
    void sigSamplingRateSelected(int idx);
    void sigDownsamplingRatioSelected(int idx);
    void sigClampingModalitySelected(int idx);
};
#endif // CONTROLLERDEVICE_H
