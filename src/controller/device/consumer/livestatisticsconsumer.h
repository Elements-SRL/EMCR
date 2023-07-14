#ifndef LIVESTATISTICSCONSUMER_H
#define LIVESTATISTICSCONSUMER_H

#define LSC_MIN_INTERVAL_S 1.0
#define LSC_MIN_BATCH_INTERVAL_S 0.1

#include "messagedispatcher.h"
#include "devicedataconsumer.h"
#include "mainwindow.h"
#include "statisticsresult.h"
#include "statisticsresult.h"

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

    // To do on actions done on the chessboard
    void onSingleChannelClicked(uint16_t chIdx, bool newState);
    void onOneBoardClicked(uint16_t brdIdx, bool newState);
    void onOneRowClicked(uint16_t rowIdx, bool newState);
    void onAllChannelsClicked(bool newState);
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

    std::vector<int> activeChannelsIdxs;
    int minSamples = 0;
    MainWindow * mainWindow;
    QVector <double> voltageSum;
    QVector <double> voltageSum2;

    QVector <double> currentSum;
    QVector <double> currentSum2;

    double pushedSamplingRate = 1.0;
    double sweepSamplingRate = 1.0;

    StatisticsResult * res;
    bool isInVec(std::vector<int> vec, int elem);
    void removeElem(std::vector<int> vec, int elem);
    void getNewActiveChannels(std::vector <int>& newActiveChannels);

signals:
    void sigResult(StatisticsResult *); 
};

#endif // LIVESTATISTICSCONSUMER_H
