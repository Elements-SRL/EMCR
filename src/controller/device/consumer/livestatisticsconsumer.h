#ifndef LIVESTATISTICSCONSUMER_H
#define LIVESTATISTICSCONSUMER_H

#define LSC_MIN_INTERVAL_S 1.0
#define LSC_MIN_BATCH_INTERVAL_S 0.001

#include "application_status.h"
#include "devicedataconsumer.h"
#include "statisticsresult.h"
#include "statisticsresult.h"
#include "statisticsresultwrapper.h"

class MeasurementsOverviewDockWidget;

class LiveStatisticsConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    LiveStatisticsConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
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

    std::vector <double> buffer;

    QMutex mutex;

private:
    int analysisSamples;
    int totalAnalysisSamples;
    int analysisIdx;
    int voltageIdx;
    int currentIdx;

    int minSamples = 0;
    std::vector <double> voltageSum;
    std::vector <double> voltageSum2;
    std::vector <double> currentSum;
    std::vector <double> currentSum2;
    std::vector<StatisticsResult> results;

    bool isInVec(std::vector<int> vec, int elem);

signals:
    void sigResult(StatisticsResultWrapper res);
};

#endif // LIVESTATISTICSCONSUMER_H
