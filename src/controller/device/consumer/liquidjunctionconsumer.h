#ifndef LIQUIDJUNCTIONCONSUMER_H
#define LIQUIDJUNCTIONCONSUMER_H

#include "devicedataconsumer.h"

class LiquidJunctionConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    LiquidJunctionConsumer(MessageDispatcher * msgDisp, DeviceDataProducer * producer);
    virtual ~LiquidJunctionConsumer();

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    /*! not really needed */
    void onSamplingRateChanged(Measurement_t) override {};
    void onDownsamplingRatioChanged(unsigned int) override {};
    void onVoltageRangeChanged(RangedMeasurement_t) override {};
    void onCurrentRangeChanged(RangedMeasurement_t) override {};

protected:
    void run() override;

private:
    LiquidJunctionHook * ljHook = nullptr;

signals:
    void sigResult(QVector <Measurement_t> voltageOffsets);
};

#endif // LIQUIDJUNCTIONCONSUMER_H
