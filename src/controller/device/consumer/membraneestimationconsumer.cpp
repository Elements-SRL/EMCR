#include "membraneestimationconsumer.h"

MembraneEstimationConsumer::MembraneEstimationConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    SquareVoltageBasedAnalysisConsumer(appStatus, producer) {

    transient1Perc = MEC_TRANSIENT1_PERC;
    transient2Perc = MEC_TRANSIENT2_PERC;
    trail1Perc = MEC_TRAIL_PERC;
    trail2Perc = MEC_TRAIL_PERC;
    minPeriods = MEC_MIN_PERIODS;
    minBatchIntervalS = MEC_MIN_BATCH_INTERVAL_S;
    initialDelayS = MEC_INITIAL_DELAY_S;
}

void MembraneEstimationConsumer::initAnalysis() {
    results.resize(currentChannelsNum);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].chIdx = channelIdx;
        results[channelIdx].membraneCapacitance.unit = "F";
        results[channelIdx].accessResistance.unit = "Ohm";
        results[channelIdx].membraneResistance.unit = "Ohm";
    }
    currentSum.resize(currentChannelsNum);
    currentSum2.resize(currentChannelsNum);
    currentTransient.resize(currentChannelsNum);
    this->lockAndResetAnalysis();
}

void MembraneEstimationConsumer::resetAnalysis() {
    SquareVoltageBasedAnalysisConsumer::resetAnalysis();
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        results[channelIdx].membraneCapacitance.prefix = currentRange.prefix / voltageRange.prefix; /*! it should be current * time / voltage, but the time is in seconds */
        results[channelIdx].accessResistance.prefix = voltageRange.prefix / currentRange.prefix;
        results[channelIdx].membraneResistance.prefix = voltageRange.prefix / currentRange.prefix;
    }
}

void MembraneEstimationConsumer::computingPeriodEnd() {
    waitForRegimeSamples = qRound(transient1Perc*(double)periodSamples);
    regimeSamples = qRound((1.0-transient1Perc-trail2Perc)*(double)periodSamples);
}

void MembraneEstimationConsumer::waitingForTransientEnd() {
    if (collectedPeriods == 0) {
        std::fill(currentSum.begin(), currentSum.end(), 0.0);
        std::fill(currentSum2.begin(), currentSum2.end(), 0.0);
        for (auto &&t : currentTransient) {
            t.resize(toBeCollectedSamples2);
            std::fill(t.begin(), t.end(), 0.0);
        }
        voltageSum = buffer[analysisIdx+voltageIdx];
    }
}

void MembraneEstimationConsumer::collectingDataExe() {
    for (int currentIdx : channelsToBeAnalyzed) {
        int channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
        currentSum[currentIdx] += buffer[channelIdx];
    }
}

void MembraneEstimationConsumer::waitingForTransient2End() {
    if (collectedPeriods == 0) {
        voltageSum2 = buffer[analysisIdx+voltageIdx];
    }
    secondPulseIdx = 0;
}

void MembraneEstimationConsumer::collectingData2Exe() {
    for (int currentIdx : channelsToBeAnalyzed) {
        int channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
        currentTransient[currentIdx][secondPulseIdx] += buffer[channelIdx];
        if (secondPulseIdx >= waitForRegimeSamples) {
            currentSum2[currentIdx] += buffer[channelIdx];
        }
    }
    secondPulseIdx++;
}

void MembraneEstimationConsumer::computeResults() {
    for (int currentIdx : channelsToBeAnalyzed) {
        currentSum[currentIdx] /= (double)(toBeCollectedSamples*collectedPeriods);
        currentSum2[currentIdx] /= (double)(regimeSamples*collectedPeriods);
        double deltaVoltage = voltageSum2-voltageSum;
        double deltaCurrent = currentSum2[currentIdx]-currentSum[currentIdx];
        if (deltaVoltage*deltaCurrent <= 0.0) {
            continue;
        }
        double totalR = deltaVoltage/deltaCurrent;
        double currentIntegral = 0.0;
        for (int idx = 0; idx < toBeCollectedSamples2; idx++) {
            currentTransient[currentIdx][idx] = currentTransient[currentIdx][idx]/(double)(collectedPeriods)-currentSum2[currentIdx];
            currentIntegral += currentTransient[currentIdx][idx];
        }

        double rxx0 = 0.0;
        double rxx1 = 0.0;

        double peakValue = 0.0;
        bool peakJustFound = false;

        for (int idx = 1; idx < toBeCollectedSamples2; idx++) {
            if (!peakJustFound) {
                rxx0 += currentTransient[currentIdx][idx-1]*currentTransient[currentIdx][idx-1];
                rxx1 += currentTransient[currentIdx][idx]*currentTransient[currentIdx][idx-1];
            }

            if (abs(currentTransient[currentIdx][idx]) > peakValue) {
                peakValue = abs(currentTransient[currentIdx][idx]);
                peakJustFound = true;
                rxx0 = 0.0;
                rxx1 = 0.0;
            }
            else {
                peakJustFound = false;
            }
        }
        if (rxx1*rxx0 <= 0.0) {
            continue;
        }
        double estTau = -1.0/(samplingRateHz*log(rxx1/rxx0));

        int ratio = (int)round(estTau*samplingRateHz/20.0);
        int ratioCounter = 0;
        int oldRatio = -2;

        bool exitLoop = false;
        while (abs(ratio-oldRatio) > 3 && ratioCounter++ < 5) { /*! with oldRatio = -2 the first check verifies that ratio is bigger than 1, on following iterations it checks for convergence */
            oldRatio = ratio;
            rxx0 = 0.0;
            rxx1 = 0.0;

            peakValue = 0.0;
            peakJustFound = false;

            for (int idx = 1; idx < toBeCollectedSamples2; idx++) {
                if (!peakJustFound) {
                    if (idx > ratio) {
                        rxx0 += currentTransient[currentIdx][idx-ratio]*currentTransient[currentIdx][idx-ratio];
                        rxx1 += currentTransient[currentIdx][idx]*currentTransient[currentIdx][idx-ratio];
                    }
                }

                if (abs(currentTransient[currentIdx][idx]) > peakValue) {
                    peakValue = abs(currentTransient[currentIdx][idx]);
                    peakJustFound = true;
                    rxx0 = 0.0;
                    rxx1 = 0.0;
                }
                else {
                    peakJustFound = false;
                }
            }
            if (rxx1*rxx0 <= 0.0) {
                exitLoop = true;
                break;
            }
            estTau = -1.0/(samplingRateHz/(double)ratio*log(rxx1/rxx0));
        }

        if (exitLoop) {
            continue;
        }

        double charge = currentIntegral/samplingRateHz+deltaCurrent*estTau; /*!< The second addend is the correction charge */
        results[currentIdx].membraneCapacitance.value = charge/deltaVoltage;
        results[currentIdx].accessResistance.value = 0.5*totalR-sqrt(totalR*(0.25*totalR-estTau/results[currentIdx].membraneCapacitance.value));
        results[currentIdx].membraneResistance.value = totalR-results[currentIdx].accessResistance.value;
    }
    MembraneResultWrapper_t w = {results};
    emit sigResult(w);
}
