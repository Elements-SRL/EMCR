#include "livestatisticsconsumer.h"

#include <qmath.h>

LiveStatisticsConsumer::LiveStatisticsConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    AnalysisConsumer(appStatus, producer) {

}

void LiveStatisticsConsumer::initAnalysis() {
    results.resize(currentChannelsNum);
    voltageSum.resize(voltageChannelsNum);
    voltageSum2.resize(voltageChannelsNum);
    currentSum.resize(currentChannelsNum);
    currentSum2.resize(currentChannelsNum);
    this->lockAndResetAnalysis();
}

void LiveStatisticsConsumer::resetAnalysis() {
    minSamples = qRound(samplingRateHz*LSC_MIN_INTERVAL_S);
    minDataBatchSize = qRound(samplingRateHz*LSC_MIN_BATCH_INTERVAL_S);
    totalAnalysisSamples = 0;
}

void LiveStatisticsConsumer::performAnalysis() {
    QMutexLocker locker(&mutex);
    int bufferLen = buffer.size();
    analysisSamples = bufferLen/totalChannelsNum;
    int channelIdx;
    double bufferedValue;
    if (totalAnalysisSamples == 0) {
        std::fill(voltageSum.begin(), voltageSum.end(), 0.0);
        std::fill(voltageSum2.begin(), voltageSum2.end(), 0.0);
        std::fill(currentSum.begin(), currentSum.end(), 0.0);
        std::fill(currentSum2.begin(), currentSum2.end(), 0.0);
    }

    for (analysisIdx = 0; analysisIdx < bufferLen; analysisIdx += totalChannelsNum) {
        for (voltageIdx = 0; voltageIdx < voltageChannelsNum; voltageIdx++) {
            channelIdx = analysisIdx+voltageIdx;
            bufferedValue = buffer[channelIdx];
            voltageSum[voltageIdx] += buffer[channelIdx];
            voltageSum2[voltageIdx] += bufferedValue * bufferedValue;
        }

        for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
            channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
            bufferedValue = buffer[channelIdx];
            currentSum[currentIdx] += bufferedValue;
            currentSum2[currentIdx] += bufferedValue * bufferedValue;
        }
        totalAnalysisSamples++;
    }

    if (totalAnalysisSamples >= minSamples) {
        for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
            const auto meanVoltage = (voltageSum[chIdx] / ((double)totalAnalysisSamples));
            const auto stdVoltage = qSqrt((voltageSum2[chIdx] - voltageSum[chIdx] * meanVoltage) / ((double)totalAnalysisSamples));

            const auto meanCurrent = currentSum[chIdx] / ((double)totalAnalysisSamples);
            const auto stdCurrent = qSqrt((currentSum2[chIdx] - currentSum[chIdx] * meanCurrent) / ((double)totalAnalysisSamples));

            const Measurement meanVoltageMeasurement = { meanVoltage , voltageRange[chIdx].prefix, voltageRange[chIdx].unit };
            const Measurement stdVoltageMeasurement = { stdVoltage, voltageRange[chIdx].prefix, voltageRange[chIdx].unit };
            const Measurement meanCurrentMeasurement = { meanCurrent, currentRange[chIdx].prefix, currentRange[chIdx].unit };
            const Measurement stdCurrentMeasurement = { stdCurrent, currentRange[chIdx].prefix, currentRange[chIdx].unit };
            const StatisticsResult sr = { chIdx, meanVoltageMeasurement, stdVoltageMeasurement, meanCurrentMeasurement, stdCurrentMeasurement };
            results[chIdx] = sr;
        }
        totalAnalysisSamples = 0;
        StatisticsResultWrapper_t w = {results};
        emit sigResult(w);
    }
}
