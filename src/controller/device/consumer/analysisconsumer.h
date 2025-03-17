#ifndef ANALYSISCONSUMER_H
#define ANALYSISCONSUMER_H

#include "application_status.h"
#include "devicedataconsumer.h"

class AnalysisConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    AnalysisConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;

protected:
    void run() override;
    void lockAndResetAnalysis();
    virtual void initAnalysis() = 0;
    virtual void resetAnalysis() = 0;
    virtual void performAnalysis() = 0;

    void updateSamplingRate();
    void updateRanges();

    QMutex samplingRateMtx;
    QMutex rangesMtx;

    std::vector <double> buffer;
    bool flushAfterAnalysisFlag = true; /*! Get rid of some data, these analyses will work anyway */

    QMutex mutex;
};

#endif // ANALYSISCONSUMER_H
