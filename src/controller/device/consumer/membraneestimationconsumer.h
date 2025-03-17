#ifndef MEMBRANEESTIMATIONCONSUMER_H
#define MEMBRANEESTIMATIONCONSUMER_H

#define MEC_TRANSIENT1_PERC 0.5
#define MEC_TRANSIENT2_PERC 0.0
#define MEC_TRAIL_PERC 0.25
#define MEC_MIN_BATCH_INTERVAL_S 0.001
#define MEC_MIN_PERIODS 5
#define MEC_INITIAL_DELAY_S 0.5

#include "squarevoltagebasedanalysisconsumer.h"
#include "membraneresult.h"
#include "resultwrapper.h"

class MembraneEstimationConsumer : public SquareVoltageBasedAnalysisConsumer {
    Q_OBJECT

public:
    MembraneEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

protected:
    void initAnalysis() override;
    void resetAnalysis() override;

    void computingPeriodEnd() override;
    void waitingForTransientEnd() override;
    void collectingDataExe() override;
    void waitingForTransient2End() override;
    void collectingData2Exe() override;
    void computeResults() override;

private:
    double voltageSum;
    double voltageSum2;
    std::vector <double> currentSum;
    std::vector <double> currentSum2;
    std::vector <std::vector <double>> currentTransient;
    std::vector <MembraneResult_t> results;

    int secondPulseIdx;
    int waitForRegimeSamples;
    int regimeSamples;

signals:
    void sigResult(MembraneResultWrapper_t res);
};

#endif // MEMBRANEESTIMATIONCONSUMER_H
