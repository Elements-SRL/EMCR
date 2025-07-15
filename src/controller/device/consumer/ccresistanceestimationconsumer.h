#ifndef CCRESISTANCEESTIMATIONCONSUMER_H
#define CCRESISTANCEESTIMATIONCONSUMER_H

#include "squarecurrentbasedanalysisconsumer.h"
#include "singlemeasresult.h"
#include "resultwrapper.h"

class CcResistanceEstimationConsumer : public SquareCurrentBasedAnalysisConsumer {
    Q_OBJECT

public:
    CcResistanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

protected:
    void initAnalysis() override;
    void resetAnalysis() override;

    void waitingForTransientEnd() override;
    void collectingDataExe() override;
    void collectingDataEnd() override;
    void waitingForTransient2End() override;
    void collectingData2Exe() override;
    void collectingData2End() override;
    void computeResults() override;

private:
    double currentSum;
    double currentSum2;
    std::vector <double> voltageSum;
    std::vector <double> voltageSum2;
    std::vector <SingleMeasResult_t> results;

signals:
    void sigResult(SingleMeasResultWrapper_t res);
};

#endif // CCRESISTANCEESTIMATIONCONSUMER_H
