#include "squarecurrentbasedanalysisconsumer.h"

SquareCurrentBasedAnalysisConsumer::SquareCurrentBasedAnalysisConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    AnalysisConsumer(appStatus, producer) {

    flushAfterAnalysisFlag = false; /*! Protocol based analysis need to process all of the data */
}

void SquareCurrentBasedAnalysisConsumer::resetAnalysis() {
    minDataBatchSize = qRound(samplingRateHz*minBatchIntervalS);
    initialDelaySamples = qRound(samplingRateHz*initialDelayS);
    status = WaitingForInitialDelay;
    collectedPeriods = 0;
    initialDelaySamplesPassed = 0;
    periodSamples = 0;
    waitingSamples = 0;
    collectingSamples = 0;
    channelsToBeAnalyzed = appStatus->getExpandedAndStimActiveChannelsIndexes();
    if (channelsToBeAnalyzed.empty()) {
        channelsToBeAnalyzed = {0};
        emit sigNoChannelsForAnalysis();
    }
    currentIdx = channelsToBeAnalyzed[0];
}

void SquareCurrentBasedAnalysisConsumer::performAnalysis() {
    QMutexLocker locker(&mutex);
    int bufferLen = buffer.size();
    double bufferedValue;

    for (analysisIdx = 0; analysisIdx < bufferLen; analysisIdx += totalChannelsNum) {
        switch (status) {
        case WaitingForInitialDelay:
            if (++initialDelaySamplesPassed < initialDelaySamples) {
                this->waitingForInitialDelayExe();
            }
            else {
                prevCurrent = buffer[analysisIdx+voltageChannelsNum+currentIdx];
                status = WaitingForFirstEdge;
                this->waitingForInitialDelayEnd();
            }
            break;

        case WaitingForFirstEdge:
            bufferedValue = buffer[analysisIdx+voltageChannelsNum+currentIdx];
            if (bufferedValue == prevCurrent) {
                this->waitingForFirstEdgeExe();
            }
            else {
                prevCurrent = bufferedValue;
                status = ComputingPeriod;
                this->waitingForFirstEdgeEnd();
                periodSamples = 0;
            }
            break;

        case ComputingPeriod:
            bufferedValue = buffer[analysisIdx+voltageChannelsNum+currentIdx];
            if (bufferedValue == prevCurrent) {
                periodSamples++;
                this->computingPeriodExe();
            }
            else {
                prevCurrent = bufferedValue;
                transientSamples = qRound(transient1Perc*(double)periodSamples);
                transientSamples2 = qRound(transient2Perc*(double)periodSamples);
                toBeCollectedSamples = qRound((1.0-transient1Perc-trail1Perc)*(double)periodSamples);
                toBeCollectedSamples2 = qRound((1.0-transient2Perc-trail2Perc)*(double)periodSamples);
                status = WaitingForTransient;
                this->computingPeriodEnd();
                waitingSamples = 0;
            }
            break;

        case WaitingForTransient:
            if (waitingSamples++ < transientSamples) {
                this->waitingForTransientExe();
            }
            else {
                status = CollectingData;
                this->waitingForTransientEnd();
                collectingSamples = 0;
            }
            break;

        case CollectingData:
            if (collectingSamples++ < toBeCollectedSamples) {
                this->collectingDataExe();
            }
            else {
                status = WaitingForEdge;
                this->collectingDataEnd();
            }
            break;

        case WaitingForEdge:
            bufferedValue = buffer[analysisIdx+voltageChannelsNum+currentIdx];
            if (bufferedValue == prevCurrent) {
                this->waitingForEdgeExe();
            }
            else {
                prevCurrent = bufferedValue;
                status = WaitingForTransient2;
                this->waitingForEdgeEnd();
                waitingSamples = 0;
            }
            break;

        case WaitingForTransient2:
            if (waitingSamples++ < transientSamples2) {
                this->waitingForTransient2Exe();
            }
            else {
                status = CollectingData2;
                this->waitingForTransient2End();
                collectingSamples = 0;
            }
            break;

        case CollectingData2:
            if (collectingSamples++ < toBeCollectedSamples2) {
                this->collectingData2Exe();
            }
            else {
                status = WaitingForEdge2;
                this->collectingData2End();

                if (++collectedPeriods >= minPeriods) {
                    this->computeResults();
                    collectedPeriods = 0;
                }
            }
            break;

        case WaitingForEdge2:
            bufferedValue = buffer[analysisIdx+voltageChannelsNum+currentIdx];
            if (bufferedValue == prevCurrent) {
                this->waitingForEdge2Exe();
            }
            else {
                prevCurrent = bufferedValue;
                status = WaitingForTransient;
                this->waitingForEdge2End();
                waitingSamples = 0;
            }
            break;
        }
    }
}
