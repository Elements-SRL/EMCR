#ifndef PIPETTECAPACITANCEESTIMATIONCONSUMER_H
#define PIPETTECAPACITANCEESTIMATIONCONSUMER_H

#define PEC_TRANSIENT1_PERC 0.5
#define PEC_TRANSIENT2_PERC 0.0
#define PEC_TRAIL_PERC 0.25
#define PEC_MIN_BATCH_INTERVAL_S 0.001
#define PEC_MIN_PERIODS 5
#define PEC_INITIAL_DELAY_S 0.5

#include "squarevoltagebasedanalysisconsumer.h"
#include "singlemeasresult.h"
#include "singlemeasresultwrapper.h"

class PipetteCapacitanceEstimationConsumer : public SquareVoltageBasedAnalysisConsumer {
    Q_OBJECT

public:
    PipetteCapacitanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

protected:
    void initAnalysis() override;
    void resetAnalysis() override;

    void computingPeriodEnd() override;
    void waitingForTransientEnd() override;
    void waitingForTransient2End() override;
    void collectingData2Exe() override;
    void computeResults() override;

private:
    double voltageSum;
    double voltageSum2;
    std::vector <double> currentSum2;
    std::vector <std::vector <double>> currentTransient;
    std::vector <SingleMeasResult_t> results;

    int secondPulseIdx;
    int waitForRegimeSamples;
    int regimeSamples;

signals:
    void sigResult(SingleMeasResultWrapper_t res);
};

#endif // PIPETTECAPACITANCEESTIMATIONCONSUMER_H
