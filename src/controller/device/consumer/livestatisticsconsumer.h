#ifndef LIVESTATISTICSCONSUMER_H
#define LIVESTATISTICSCONSUMER_H

#define LSC_MIN_INTERVAL_S 1.0
#define LSC_MIN_BATCH_INTERVAL_S 0.001

#include "analysisconsumer.h"
#include "statisticsresult.h"
#include "resultwrapper.h"

class LiveStatisticsConsumer : public AnalysisConsumer {
    Q_OBJECT

public:
    LiveStatisticsConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

    virtual void onClampingModalityChanged(ClampingModality_t mode) override;
protected:
    void initAnalysis() override;
    void resetAnalysis() override;
    void performAnalysis() override;

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
    std::vector <StatisticsResult_t> results;

signals:
    void sigResult(StatisticsResultWrapper_t res);
};

#endif // LIVESTATISTICSCONSUMER_H
