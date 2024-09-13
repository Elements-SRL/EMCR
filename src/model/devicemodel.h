#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QObject>

#include "devicecontroldockwidget.h"
#include "application_status.h"

class DeviceModel : public QObject {
    Q_OBJECT

public:
    DeviceModel(ApplicationStatus * appStatus, DeviceControlDockWidget * view);

public slots:
    void onSamplingrateSelected(int idx);
    void onDownsamplingRatioChanged();
    void onDigFiltEnabled();
    void onDigFiltSettingsChanged();

private:
    void updateDownsamplingAndFilteringSettings();

    DeviceControlDockWidget * view = nullptr;
    ApplicationStatus * appStatus = nullptr;

    std::vector <Measurement_t> samplingRates;
    int downsamplingRatio = 1;
    bool downsamplingFlag = false;
    bool digFiltEnabledFlag = false;
    bool digFiltLowPassFlag = true;
    bool digFiltEnabledUserFlag = false;
    bool digFiltLowPassUserFlag = true;
    Measurement_t cutoffFrequency = {1.0, UnitPfxNone, "Hz"};
    Measurement_t maxCutoffFrequency = {1.0, UnitPfxNone, "Hz"};
    Measurement_t samplingRate = {1.0, UnitPfxNone, "Hz"};
    Measurement_t finalSamplingRate = {1.0, UnitPfxNone, "Hz"};

signals:
    void sigDownsamplingRatioSelected(int ratio);
    void sigDigitalFilterChanged(Measurement_t cutoffrequency, bool lowPassFlag, bool activeFlag);
};

#endif // DEVICEMODEL_H
