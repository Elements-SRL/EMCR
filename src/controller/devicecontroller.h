#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include <QObject>

#include "devicecontroldockwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"

class DeviceController : public QObject {
    Q_OBJECT

public:
    DeviceController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    ~DeviceController();
    void handleRecording(bool);
    void handleProtocolStatusChanged(bool);

public slots:
    void onVcCurrentRangeSelected(uint16_t selectedVcCurrentRangeIndex);
    void onVcVoltageRangeSelected(uint16_t selectedVcCurrentRangeIndex);
    void onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex);
    void onCcCurrentRangeSelected(uint16_t selectedCcVoltageRangeIndex);
    void onVcVoltageFilterSelected(uint16_t selectedVcVoltageFilterIndex);
    void onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex);
    void onSamplingRateSelected(uint16_t selectedCcCurrentFilterIndex);
    void onDownsamplingRatioSelected(uint16_t selectedDownsamplingRatioIndex);
    void onClampingModalitySelected(ClampingModality_t mode);
    void onCustomOptionSelected(uint32_t customOptionIdx, int idx);
    void onCustomDoubleChanged(uint32_t customDoubleIdx, double value);

private:
    std::vector<ClampingModality_t> clampingModalities;
    std::vector <RangedMeasurement_t> vcCurrentRanges;
    std::vector <RangedMeasurement_t> vcVoltageRanges;
    std::vector <RangedMeasurement_t> ccCurrentRanges;
    std::vector <RangedMeasurement_t> ccVoltageRanges;
    std::vector <Measurement_t> vcVoltageFilters;
    std::vector <Measurement_t> ccCurrentFilters;
    std::vector <Measurement_t> samplingRates;

    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow;
    DeviceControlDockWidget * deviceControlDockWidget = nullptr;
    bool recording = false;
    bool protocolRunning = false;
    bool getStatusFromRecordingAndProtocol();
    bool calcDefaultStatus(int, bool);

signals:
    void sigVcCurrentRangeSelected(int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int idx);
    void sigVcVoltageFilterSelected(int idx);
    void sigCcCurrentFilterSelected(int idx);
    void sigSamplingRateSelected(int idx);
    void sigDownsamplingRatioSelected(int idx);
    void sigClampingModalitySelected(ClampingModality_t mode);
};
#endif // DEVICECONTROLLER_H
