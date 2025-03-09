#include "resistanceestimationconsumer.h"

ResistanceEstimationConsumer::ResistanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    SquareVoltageBasedAnalysisConsumer(appStatus, producer) {

    transient1Perc = REC_TRANSIENT_PERC;
    transient2Perc = REC_TRANSIENT_PERC;
    trail1Perc = REC_TRAIL_PERC;
    trail2Perc = REC_TRAIL_PERC;
    minPeriods = REC_MIN_PERIODS;
    minBatchIntervalS = REC_MIN_BATCH_INTERVAL_S;
    initialDelayS = REC_INITIAL_DELAY_S;
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
    SquareVoltageBasedAnalysisConsumer::resetAnalysis();
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].meas.value = 0.0;
        results[channelIdx].meas.prefix = voltageRange.prefix / currentRange.prefix;
    }
}

void ResistanceEstimationConsumer::waitingForTransientEnd() {
    std::fill(currentSum.begin(), currentSum.end(), 0.0);
    std::fill(currentSum2.begin(), currentSum2.end(), 0.0);
    voltageSum = buffer[analysisIdx+voltageIdx];
}

void ResistanceEstimationConsumer::collectingDataExe() {
    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
        int channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
        currentSum[currentIdx] += buffer[channelIdx];
    }
}

void ResistanceEstimationConsumer::collectingDataEnd() {
    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
        currentSum[currentIdx] /= (double)toBeCollectedSamples;
    }
}

void ResistanceEstimationConsumer::waitingForTransient2End() {
    voltageSum2 = buffer[analysisIdx+voltageIdx];
}

void ResistanceEstimationConsumer::collectingData2Exe() {
    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
        int channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
        currentSum2[currentIdx] += buffer[channelIdx];
    }
}

void ResistanceEstimationConsumer::collectingData2End() {
    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
        currentSum2[currentIdx] /= (double)toBeCollectedSamples2;
        results[currentIdx].meas.value += (voltageSum-voltageSum2)/(currentSum[currentIdx]-currentSum2[currentIdx]);
    }
}

void ResistanceEstimationConsumer::computeResults() {
    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
        results[currentIdx].meas.value /= (double)collectedPeriods;
    }
    SingleMeasResultWrapper_t w = {results};
    emit sigResult(w);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].meas.value = 0.0;
    }
}
