#include "ccresistanceestimationconsumer.h"

#define REC_TRANSIENT_PERC 0.5
#define REC_TRAIL_PERC 0.25
#define REC_MIN_BATCH_INTERVAL_S 0.001
#define REC_MIN_PERIODS 5
#define REC_INITIAL_DELAY_S 0.5

CcResistanceEstimationConsumer::CcResistanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    SquareCurrentBasedAnalysisConsumer(appStatus, producer) {

    transient1Perc = REC_TRANSIENT_PERC;
    transient2Perc = REC_TRANSIENT_PERC;
    trail1Perc = REC_TRAIL_PERC;
    trail2Perc = REC_TRAIL_PERC;
    minPeriods = REC_MIN_PERIODS;
    minBatchIntervalS = REC_MIN_BATCH_INTERVAL_S;
    initialDelayS = REC_INITIAL_DELAY_S;
}

void CcResistanceEstimationConsumer::initAnalysis() {
    results.resize(voltageChannelsNum);
    for (int channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
        results[channelIdx].chIdx = channelIdx;
        results[channelIdx].meas.unit = "Ohm";
    }
    voltageSum.resize(voltageChannelsNum);
    voltageSum2.resize(voltageChannelsNum);
    this->lockAndResetAnalysis();
}

void CcResistanceEstimationConsumer::resetAnalysis() {
    SquareCurrentBasedAnalysisConsumer::resetAnalysis();
    for (int channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
        results[channelIdx].meas.value = 0.0;
        results[channelIdx].meas.prefix = voltageRange[channelIdx].prefix / currentRange[channelIdx].prefix;
    }
}

void CcResistanceEstimationConsumer::waitingForTransientEnd() {
    std::fill(voltageSum.begin(), voltageSum.end(), 0.0);
    std::fill(voltageSum2.begin(), voltageSum2.end(), 0.0);
    currentSum = buffer[analysisIdx+voltageChannelsNum+currentIdx];
}

void CcResistanceEstimationConsumer::collectingDataExe() {
    for (int voltageIdx : channelsToBeAnalyzed) {
        int channelIdx = analysisIdx+voltageIdx;
        voltageSum[voltageIdx] += buffer[channelIdx];
    }
}

void CcResistanceEstimationConsumer::collectingDataEnd() {
    for (int voltageIdx : channelsToBeAnalyzed) {
        voltageSum[voltageIdx] /= (double)toBeCollectedSamples;
    }
}

void CcResistanceEstimationConsumer::waitingForTransient2End() {
    currentSum2 = buffer[analysisIdx+voltageChannelsNum+currentIdx];
}

void CcResistanceEstimationConsumer::collectingData2Exe() {
    for (int voltageIdx : channelsToBeAnalyzed) {
        int channelIdx = analysisIdx+voltageIdx;
        voltageSum2[voltageIdx] += buffer[channelIdx];
    }
}

void CcResistanceEstimationConsumer::collectingData2End() {
    for (int voltageIdx : channelsToBeAnalyzed) {
        voltageSum2[voltageIdx] /= (double)toBeCollectedSamples2;
        results[voltageIdx].meas.value += (voltageSum[voltageIdx]-voltageSum2[voltageIdx])/(currentSum-currentSum2);
    }
}

void CcResistanceEstimationConsumer::computeResults() {
    for (int voltageIdx : channelsToBeAnalyzed) {
        results[voltageIdx].meas.value /= (double)collectedPeriods;
    }
    SingleMeasResultWrapper_t w = {results};
    emit sigResult(w);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].meas.value = 0.0;
    }
}

