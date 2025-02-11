#ifndef CONTROLLERWITHCONSUMER_H
#define CONTROLLERWITHCONSUMER_H

#include <QObject>

#include "devicedataconsumer.h"

class ControllerWithConsumer : public QObject {
    Q_OBJECT

public:
    ControllerWithConsumer();

    virtual std::vector <DeviceDataConsumer*> getConsumers() = 0;

public slots:
    virtual void onSamplingRateChanged(Measurement sr);
    virtual void onDownsamplingRatioChanged(unsigned int dsr);
    virtual void onCurrentRangeChanged(RangedMeasurement rm);
    virtual void onVoltageRangeChanged(RangedMeasurement rm);
    virtual void onStopConsuming();
};

#endif // CONTROLLERWITHCONSUMER_H
