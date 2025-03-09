#ifndef SQUAREVOLTAGEBASEDANALYSISCONSUMER_H
#define SQUAREVOLTAGEBASEDANALYSISCONSUMER_H

#include "analysisconsumer.h"

class SquareVoltageBasedAnalysisConsumer : public AnalysisConsumer {
    Q_OBJECT

public:
    SquareVoltageBasedAnalysisConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);

protected:
    void resetAnalysis() override;
    void performAnalysis() override;

    virtual void waitingForInitialDelayExe() {};
    virtual void waitingForInitialDelayEnd() {};
    virtual void waitingForFirstEdgeExe() {};
    virtual void waitingForFirstEdgeEnd() {};
    virtual void computingPeriodExe() {};
    virtual void computingPeriodEnd() {};
    virtual void waitingForTransientExe() {};
    virtual void waitingForTransientEnd() {};
    virtual void collectingDataExe() {};
    virtual void collectingDataEnd() {};
    virtual void waitingForEdgeExe() {};
    virtual void waitingForEdgeEnd() {};
    virtual void waitingForTransient2Exe() {};
    virtual void waitingForTransient2End() {};
    virtual void collectingData2Exe() {};
    virtual void collectingData2End() {};
    virtual void waitingForEdge2Exe() {};
    virtual void waitingForEdge2End() {};
    virtual void computeResults() {};

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

    double transient1Perc = 0.5;
    double transient2Perc = 0.5;
    double trail1Perc = 0.25;
    double trail2Perc = 0.25;
    double initialDelayS = 0.5;
    int minPeriods = 5;
    double minBatchIntervalS = 1.0e-3;

    int initialDelaySamples;
    int initialDelaySamplesPassed;
    int waitingSamples;
    int collectingSamples;
    int periodSamples;
    int transientSamples;
    int transientSamples2;
    int toBeCollectedSamples;
    int toBeCollectedSamples2;
    int analysisSamples;
    double prevVoltage;

    int analysisIdx;
    int voltageIdx = 0; /*! \todo FCON sarebbe da assegnare il primo canale di tensione che non ha il protocollo disabilitato */
    int currentIdx;
};

#endif // SQUAREVOLTAGEBASEDANALYSISCONSUMER_H
