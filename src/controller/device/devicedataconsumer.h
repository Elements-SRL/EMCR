#ifndef DEVICEDATACONSUMER_H
#define DEVICEDATACONSUMER_H

#include <QThread>

#include "application_status.h"
#include "devicedataproducer.h"
#include "e384commlib_global.h"

class DeviceDataConsumer : public QThread {
    Q_OBJECT

public:
    DeviceDataConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    virtual ~DeviceDataConsumer();

public slots:
    virtual void onStartConsuming() = 0;
    virtual void onStopConsuming() = 0;
    virtual void onResetConsuming();

    virtual void onSamplingRateChanged(Measurement_t samplingRate) = 0;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) = 0;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) = 0;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) = 0;

protected:
    ApplicationStatus * appStatus = nullptr;
    DeviceDataProducer * producer = nullptr;
    AbstractDataHook * hook = nullptr;

    int voltageChannelsNum = 0;
    int currentChannelsNum = 0;
    int totalChannelsNum = 0;

    int bytesPerChannel = 1;

    bool pushedSamplingRateFlag = false;
    bool pushedDownsamplingRatioFlag = false;
    bool pushedVoltageRangeFlag = false;
    bool pushedCurrentRangeFlag = false;

    unsigned int minDataBatchSize = 0;
    double pushedSamplingRateHz = 1.0;
    double samplingRateHz = 1.0;

    unsigned int pushedDownsamplingRatio = 1;
    unsigned int downsamplingRatio = 1;

    RangedMeasurement_t pushedVoltageRange;
    RangedMeasurement_t voltageRange = {0.0, 1.0, 1.0, UnitPfxNone, "V"};

    RangedMeasurement_t pushedCurrentRange;
    RangedMeasurement_t currentRange = {0.0, 1.0, 1.0, UnitPfxNone, "A"};

    QMutex itemsMtx;

    bool consumptionStopped = false;
    bool exitedDataConsumingLoop = false;
    QMutex consumptionMtx;
    QWaitCondition exitedDataConsumingLoopCv;
};

#endif // DEVICEDATACONSUMER_H
