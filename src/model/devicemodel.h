#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#define DIG_FILT_CUTOFF_FREQ_UNIT_PFX UnitPfxKilo

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
    Measurement_t cutoffFrequency = {0.0, UnitPfxKilo, "Hz"};
    Measurement_t maxCutoffFrequency = {0.0, UnitPfxKilo, "Hz"};
    Measurement_t samplingRate = {0.0, UnitPfxKilo, "Hz"};
    Measurement_t finalSamplingRate = {0.0, UnitPfxKilo, "Hz"};
    const double finalSamplingRateMultiplier = finalSamplingRate.multiplier();

signals:
    void sigDownsamplingRatioSelected(int ratio);
    void sigDigitalFilterChanged(Measurement_t cutoffrequency, bool lowPassFlag, bool activeFlag);
};

#endif // DEVICEMODEL_H
