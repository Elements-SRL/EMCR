#ifndef RESISTANCEESTIMATIONCONSUMER_H
#define RESISTANCEESTIMATIONCONSUMER_H

#define REC_TRANSIENT_PERC 0.5
#define REC_TRAIL_PERC 0.25
#define REC_MIN_BATCH_INTERVAL_S 0.001
#define REC_MIN_PERIODS 5
#define REC_INITIAL_DELAY_S 0.5

#include "squarevoltagebasedanalysisconsumer.h"
#include "singlemeasresult.h"
#include "singlemeasresultwrapper.h"

class ResistanceEstimationConsumer : public SquareVoltageBasedAnalysisConsumer {
    Q_OBJECT

public:
    ResistanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

protected:
    void initAnalysis() override;
    void resetAnalysis() override;

    void computingPeriodEnd() override;
    void waitingForTransientEnd() override;
    void collectingDataExe() override;
    void collectingDataEnd() override;
    void waitingForTransient2End() override;
    void collectingData2Exe() override;
    void collectingData2End() override;
    void computeResults() override;

private:
    double voltageSum;
    double voltageSum2;
    std::vector <double> currentSum;
    std::vector <double> currentSum2;
    std::vector <SingleMeasResult_t> results;

signals:
    void sigResult(SingleMeasResultWrapper_t res);
};

#endif // RESISTANCEESTIMATIONCONSUMER_H
