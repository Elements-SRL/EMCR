#ifndef LIVENOISECONSUMER_H
#define LIVENOISECONSUMER_H

#define LNC_MIN_INTERVAL_S 0.5

#include "modeldevice.h"
#include "devicedataconsumer.h"

class LiveNoiseConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    LiveNoiseConsumer(ModelDevice * mDev, DeviceDataProducer * producer);
    virtual ~LiveNoiseConsumer();

    typedef struct {
        QVector <double> meanVoltage;
        QVector <double> stdVoltage;
        QVector <double> meanCurrent;
        QVector <double> stdCurrent;
    } Result_t;

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;

protected:
    void run() override;
    void initAnalysis();
    void lockAndResetAnalysis(int currentChannelIdx);
    void resetAnalysis(int currentChannelIdx);
    void performAnalysis();
    void updateSamplingRate();
    void updateRanges();

    QMutex samplingRateMtx;
    QMutex rangesMtx;

    bool consumptionStopped = false;
    bool exitedDataConsumingLoop = false;
    QMutex consumptionMtx;
    QWaitCondition exitedDataConsumingLoopCv;

    double voltageMultiplier;
    double currentMultiplier;

    QVector <double> analysisBuffer;
    QVector <double> buffer;

    QMutex mutex;

private:
    int bufferSize;
    int analysisSamples;
    int analysisIdx;
    int voltageIdx;
    int currentIdx;

    int minSamples = 0;

    QVector <double> voltageSum;
    QVector <double> voltageSum2;

    QVector <double> currentSum;
    QVector <double> currentSum2;

    double pushedSamplingRate = 1.0;
    double sweepSamplingRate = 1.0;

    Result_t res;

signals:
    void sigResult(LiveNoiseConsumer::Result_t);
};

#endif // LIVENOISECONSUMER_H
