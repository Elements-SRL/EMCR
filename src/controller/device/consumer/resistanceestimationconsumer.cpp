#include "resistanceestimationconsumer.h"

ResistanceEstimationConsumer::ResistanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    AnalysisConsumer(appStatus, producer) {

}

void ResistanceEstimationConsumer::initAnalysis() {
    results.resize(currentChannelsNum);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].chIdx = channelIdx;
        results[channelIdx].meas.unit = "Ohm";
    }
    currentSum.resize(currentChannelsNum);
    currentSum2.resize(currentChannelsNum);
    this->lockAndResetAnalysis();
}

void ResistanceEstimationConsumer::resetAnalysis() {
    minDataBatchSize = qRound(samplingRateHz*REC_MIN_BATCH_INTERVAL_S);
    initialDelaySamples = qRound(samplingRateHz*REC_INITIAL_DELAY_S);
    status = WaitingForInitialDelay;
    collectedPeriods = 0;
    initialDelaySamplesPassed = 0;
    periodSamples = 0;
    waitingSamples = 0;
    toBeCollectedSamples = 0;
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].meas.unit = 0.0;
        results[channelIdx].meas.prefix = voltageRange.prefix / currentRange.prefix;
    }
}

void ResistanceEstimationConsumer::performAnalysis() {
    QMutexLocker locker(&mutex);
    int bufferLen = buffer.size();
    double bufferedValue;
    int voltageIdx = 0; /*! \todo FCON sarebbe da assegnare il primo canale di tensione che non ha il protocollo disabilitato */
    int currentIdx;
    int channelIdx;

    for (int analysisIdx = 0; analysisIdx < bufferLen; analysisIdx += totalChannelsNum) {
        switch (status) {
        case WaitingForInitialDelay:
            if (++initialDelaySamplesPassed > initialDelaySamples) {
                status = WaitingForFirstEdge;
            }
            break;

        case WaitingForFirstEdge:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                status = ComputingPeriod;
            }
            break;

        case ComputingPeriod:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                waitingSamples = qRound(REC_TRANSIENT_PERC*(double)periodSamples);
                toBeCollectedSamples = qRound((1.0-REC_TRANSIENT_PERC-REC_TRAIL_PERC)*(double)periodSamples);
                status = JustStarted;
            }
            else {
                periodSamples++;
            }
            break;

        case JustStarted:
            std::fill(currentSum.begin(), currentSum.end(), 0.0);
            std::fill(currentSum2.begin(), currentSum2.end(), 0.0);
            prevVoltage = buffer[analysisIdx+voltageIdx];
            status = WaitingForFirstEdge;
            break;

        case WaitingForTransient:
            if (waitingSamples++ > transientSamples) {
                waitingSamples = 0;
                voltageSum = buffer[analysisIdx+voltageIdx];
                status = CollectingData;
            }
            break;

        case CollectingData:
            if (collectingSamples++ < toBeCollectedSamples) {
                voltageIdx = 0;
                channelIdx = analysisIdx+voltageIdx;

                for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
                    channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
                    currentSum[currentIdx] += buffer[channelIdx];
                }
            }
            else {
                for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
                    currentSum[currentIdx] /= (double)toBeCollectedSamples;
                }
                collectingSamples = 0;
                status = WaitingForEdge;
            }
            break;

        case WaitingForEdge:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                status = WaitingForTransient2;
            }
            break;

        case WaitingForTransient2:
            if (waitingSamples++ > transientSamples) {
                waitingSamples = 0;
                voltageSum2 = buffer[analysisIdx+voltageIdx];
                status = CollectingData2;
            }
            break;

        case CollectingData2:
            if (collectingSamples++ < toBeCollectedSamples) {
                voltageIdx = 0;
                channelIdx = analysisIdx+voltageIdx;

                for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
                    channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
                    currentSum2[currentIdx] += buffer[channelIdx];
                }
            }
            else {
                for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
                    currentSum2[currentIdx] /= (double)toBeCollectedSamples;
                    results[currentIdx].meas.value += (voltageSum-voltageSum2)/(currentSum[currentIdx]-currentSum2[currentIdx]);
                }
                if (++collectedPeriods >= REC_MIN_PERIODS) {
                    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
                        results[currentIdx].meas.value /= (double)collectedPeriods;
                    }
                    SingleMeasResultWrapper_t w = {results};
                    emit sigResult(w);
                    collectedPeriods = 0;
                    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                        results[channelIdx].meas.unit = 0.0;
                    }
                }
                collectingSamples = 0;
                status = WaitingForEdge2;
            }
            break;

        case WaitingForEdge2:
            bufferedValue = buffer[analysisIdx+voltageIdx];
            if (bufferedValue != prevVoltage) {
                prevVoltage = bufferedValue;
                status = JustStarted;
            }
            break;
        }
    }
}
