#ifndef LIVESTATISTICSCONSUMER_H
#define LIVESTATISTICSCONSUMER_H

#define LSC_MIN_INTERVAL_S 1.0
#define LSC_MIN_BATCH_INTERVAL_S 0.1

#include "messagedispatcher.h"
#include "devicedataconsumer.h"
#include "mainwindow.h"
#include "statisticsmodel.h"

class MeasurementsOverviewDockWidget;

class LiveStatisticsConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    LiveStatisticsConsumer(MessageDispatcher * msgDisp, MainWindow * mainWindow, DeviceDataProducer * producer);
    virtual ~LiveStatisticsConsumer();

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;

    void onExportLiveNoiseEstimates();

protected:
    void run() override;
    void initAnalysis();
    void lockAndResetAnalysis(int currentChannelIdx);
    void resetAnalysis(int currentChannelIdx);
    void performAnalysis();
    void updateSamplingRate();
    void updateRanges();

    MeasurementsOverviewDockWidget * modw = nullptr;

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
    int totalAnalysisSamples;
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
    void sigResult(LiveStatisticsConsumer::Result_t);
};

#endif // LIVESTATISTICSCONSUMER_H
