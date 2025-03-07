#ifndef RESISTANCEESTIMATIONCONSUMER_H
#define RESISTANCEESTIMATIONCONSUMER_H

#define REC_TRANSIENT_PERC 0.5
#define REC_TRAIL_PERC 0.25
#define REC_MIN_BATCH_INTERVAL_S 0.001
#define REC_MIN_PERIODS 5
#define REC_INITIAL_DELAY_S 0.5

#include "analysisconsumer.h"
#include "singlemeasresult.h"
#include "singlemeasresultwrapper.h"

class ResistanceEstimationConsumer : public AnalysisConsumer {
    Q_OBJECT

public:
    ResistanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

protected:
    void initAnalysis() override;
    void resetAnalysis() override;
    void performAnalysis() override;

private:
    typedef enum Status {
        WaitingForInitialDelay,
        WaitingForFirstEdge,
        ComputingPeriod,
        WaitingForTransient,
        CollectingData,
        WaitingForEdge,
        WaitingForTransient2,
        CollectingData2,
        WaitingForEdge2
    } Status_t;

    Status_t status = WaitingForInitialDelay;
    int collectedPeriods = 0;

    int initialDelaySamples;
    int initialDelaySamplesPassed;
    int waitingSamples;
    int collectingSamples;
    int periodSamples;
    int transientSamples;
    int toBeCollectedSamples;
    int analysisSamples;
    double prevVoltage;

    double voltageSum;
    double voltageSum2;
    std::vector <double> currentSum;
    std::vector <double> currentSum2;
    std::vector <SingleMeasResult_t> results;

signals:
    void sigResult(SingleMeasResultWrapper_t res);
};

#endif // RESISTANCEESTIMATIONCONSUMER_H
