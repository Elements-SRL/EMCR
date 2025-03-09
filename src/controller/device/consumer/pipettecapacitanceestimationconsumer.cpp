#include "pipettecapacitanceestimationconsumer.h"

PipetteCapacitanceEstimationConsumer::PipetteCapacitanceEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    SquareVoltageBasedAnalysisConsumer(appStatus, producer) {

    transient1Perc = PEC_TRANSIENT1_PERC;
    transient2Perc = PEC_TRANSIENT2_PERC;
    trail1Perc = PEC_TRAIL_PERC;
    trail2Perc = PEC_TRAIL_PERC;
    minPeriods = PEC_MIN_PERIODS;
    minBatchIntervalS = PEC_MIN_BATCH_INTERVAL_S;
    initialDelayS = PEC_INITIAL_DELAY_S;
}

void PipetteCapacitanceEstimationConsumer::initAnalysis() {
    results.resize(currentChannelsNum);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].chIdx = channelIdx;
        results[channelIdx].meas.unit = "F";
    }
    currentSum2.resize(currentChannelsNum);
    currentTransient.resize(currentChannelsNum);
    this->lockAndResetAnalysis();
}

void PipetteCapacitanceEstimationConsumer::resetAnalysis() {
    SquareVoltageBasedAnalysisConsumer::resetAnalysis();
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].meas.value = 0.0;
        results[channelIdx].meas.prefix = currentRange.prefix / voltageRange.prefix; /*! it should be current * time / voltage, but the time is in seconds */
    }
}

void PipetteCapacitanceEstimationConsumer::computingPeriodEnd() {
    waitForRegimeSamples = qRound(transient1Perc*(double)periodSamples);
    regimeSamples = qRound((1.0-transient1Perc-trail2Perc)*(double)periodSamples);
}

void PipetteCapacitanceEstimationConsumer::waitingForTransientEnd() {
    if (collectedPeriods == 0) {
        std::fill(currentSum2.begin(), currentSum2.end(), 0.0);
        for (auto &&t : currentTransient) {
            t.resize(toBeCollectedSamples2);
            std::fill(t.begin(), t.end(), 0.0);
        }
        voltageSum = buffer[analysisIdx+voltageIdx];
    }
}

void PipetteCapacitanceEstimationConsumer::waitingForTransient2End() {
    if (collectedPeriods == 0) {
        voltageSum2 = buffer[analysisIdx+voltageIdx];
    }
    secondPulseIdx = 0;
}

void PipetteCapacitanceEstimationConsumer::collectingData2Exe() {
    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
        int channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
        currentTransient[currentIdx][secondPulseIdx] = buffer[channelIdx];
        if (secondPulseIdx >= waitForRegimeSamples) {
            currentSum2[currentIdx] += buffer[channelIdx];
        }
    }
    secondPulseIdx++;
}

void PipetteCapacitanceEstimationConsumer::computeResults() {
    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
        currentSum2[currentIdx] /= (double)(regimeSamples*collectedPeriods);
        double currentIntegral = -currentSum2[currentIdx]*(double)(collectedPeriods); /*! currentSum2 is the regime value, so it should be subtraced from each sample in current transient,
                                                                                            so we subtract it once here  */
        for (int idx = 0; idx < toBeCollectedSamples2; idx++) {
            currentIntegral += currentTransient[currentIdx][idx];
        }
        currentIntegral /= (double)(collectedPeriods);
        double charge = currentIntegral/samplingRateHz; /*! Se non indispensabile non calcolo la tau per la pipetta */ //+deltaCurrent*estTau; /*!< The second addend is the correction charge */
        results[currentIdx].meas.value = charge/(voltageSum-voltageSum2);
    }
    SingleMeasResultWrapper_t w = {results};
    emit sigResult(w);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].meas.value = 0.0;
    }
}
