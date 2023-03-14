#ifndef DEVICEDATACONSUMER_H
#define DEVICEDATACONSUMER_H

#include <QThread>

#include "modeldevice.h"
#include "devicedataproducer.h"
#include "e384commlib_global.h"

class DeviceDataConsumer : public QThread {
    Q_OBJECT

public:
    DeviceDataConsumer(ModelDevice * mDev, DeviceDataProducer * producer);
    virtual ~DeviceDataConsumer();

public slots:
    virtual void onStartConsuming() = 0;
    virtual void onStopConsuming() = 0;
    virtual void onResetConsuming();

    virtual void onSamplingRateChanged(Measurement_t samplingRate) = 0;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) = 0;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) = 0;

protected:
    ModelDevice * mDev = nullptr;
    DeviceDataProducer * producer = nullptr;
    DataHook * hook = nullptr;

    int voltageChannelsNum = 0;
    int currentChannelsNum = 0;
    int totalChannelsNum = 0;

    bool pushedSamplingRateFlag = false;
    bool pushedVoltageRangeFlag = false;
    bool pushedCurrentRangeFlag = false;

    unsigned int minDataBatchSize = 0;
    double pushedSamplingRateHz = 1.0;
    double sweepSamplingRateHz = 1.0;

    RangedMeasurement_t pushedVoltageRange;
    RangedMeasurement_t voltageRange = {0.0, 1.0, 1.0, UnitPfxNone, "V"};

    RangedMeasurement_t pushedCurrentRange;
    RangedMeasurement_t currentRange = {0.0, 1.0, 1.0, UnitPfxNone, "A"};

    QMutex itemsMtx;
};

#endif // DEVICEDATACONSUMER_H
