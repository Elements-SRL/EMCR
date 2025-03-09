#include "squarevoltagebasedanalysisconsumer.h"

SquareVoltageBasedAnalysisConsumer::SquareVoltageBasedAnalysisConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    AnalysisConsumer(appStatus, producer) {

}

void SquareVoltageBasedAnalysisConsumer::resetAnalysis() {
    minDataBatchSize = qRound(samplingRateHz*minBatchIntervalS);
    initialDelaySamples = qRound(samplingRateHz*initialDelayS);
    status = WaitingForInitialDelay;
    collectedPeriods = 0;
    initialDelaySamplesPassed = 0;
    periodSamples = 0;
    waitingSamples = 0;
    collectingSamples = 0;
}

void SquareVoltageBasedAnalysisConsumer::performAnalysis() {
    QMutexLocker locker(&mutex);
    int bufferLen = buffer.size();
    double bufferedValue;

    for (analysisIdx = 0; analysisIdx < bufferLen; analysisIdx += totalChannelsNum) {
        switch (status) {
        case WaitingForInitialDelay:
            if (++initialDelaySamplesPassed > initialDelaySamples) {
                prevVoltage = buffer[analysisIdx+voltageIdx];
                status = WaitingForFirstEdge;
                this->waitingForInitialDelayEnd();
            }
            else {
                this->waitingForInitialDelayExe();
            }
            break;

        case WaitingForFirstEdge:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                status = ComputingPeriod;
                this->waitingForFirstEdgeEnd();
            }
            else {
                this->waitingForFirstEdgeExe();
            }
            break;

        case ComputingPeriod:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                transientSamples = qRound(transient1Perc*(double)periodSamples);
                transientSamples2 = qRound(transient2Perc*(double)periodSamples);
                toBeCollectedSamples = qRound((1.0-transient1Perc-trail1Perc)*(double)periodSamples);
                toBeCollectedSamples2 = qRound((1.0-transient2Perc-trail2Perc)*(double)periodSamples);
                periodSamples = 0;
                status = WaitingForTransient;
                this->computingPeriodEnd();
            }
            else {
                periodSamples++;
                this->computingPeriodExe();
            }
            break;

        case WaitingForTransient:
            if (waitingSamples++ > transientSamples) {
                waitingSamples = 0;
                status = CollectingData;
                this->waitingForTransientEnd();
            }
            else {
                this->waitingForTransientExe();
            }
            break;

        case CollectingData:
            if (collectingSamples++ < toBeCollectedSamples) {
                this->collectingDataExe();
            }
            else {
                collectingSamples = 0;
                status = WaitingForEdge;
                this->collectingDataEnd();
            }
            break;

        case WaitingForEdge:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                status = WaitingForTransient2;
                this->waitingForEdgeEnd();
            }
            else {
                this->waitingForEdgeExe();
            }
            break;

        case WaitingForTransient2:
            if (waitingSamples++ > transientSamples2) {
                waitingSamples = 0;
                status = CollectingData2;
                this->waitingForTransient2End();
            }
            else {
                this->waitingForTransient2Exe();
            }
            break;

        case CollectingData2:
            if (collectingSamples++ < toBeCollectedSamples2) {
                this->collectingData2Exe();
            }
            else {
                collectingSamples = 0;
                status = WaitingForEdge2;
                this->collectingData2End();
            }
            break;

        case WaitingForEdge2:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                status = WaitingForFirstEdge;
                this->waitingForEdge2End();
            }
            else {
                this->waitingForEdge2Exe();
            }
            break;
        }
    }
}
