#ifndef CONTROLLERWITHCONSUMER_H
#define CONTROLLERWITHCONSUMER_H

#include <QObject>

#include "devicedataconsumer.h"

class ControllerWithConsumer : public QObject {
    Q_OBJECT

public:
    ControllerWithConsumer(ApplicationStatus * appStatus);

    virtual std::vector <DeviceDataConsumer*> getConsumers() = 0;

protected:
    ApplicationStatus * getAppStatus();

    ApplicationStatus * appStatus;

public slots:
    virtual void onSamplingRateChanged(Measurement sr);
    virtual void onDownsamplingRatioChanged(unsigned int dsr);
    virtual void onCurrentRangeChanged();
    virtual void onVoltageRangeChanged();
    virtual void onClampingModalityChanged(ClampingModality_t mode);
    virtual void onStopConsuming();
};

#endif // CONTROLLERWITHCONSUMER_H
