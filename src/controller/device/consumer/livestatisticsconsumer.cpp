#include "livestatisticsconsumer.h"

#include <qmath.h>
#include <QFile>
#include <QDir>
#include <QTextStream>

LiveStatisticsConsumer::LiveStatisticsConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    DeviceDataConsumer(appStatus, producer) {
    buffer.reserve(producer->getDataPacketsBufferLen()*totalChannelsNum); /*! \todo FCON magari renderlo comportamneto di default della classe base */
}

LiveStatisticsConsumer::~LiveStatisticsConsumer() {

}

void LiveStatisticsConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        this->start();
    }
}

void LiveStatisticsConsumer::onStopConsuming() {
    if (this->isRunning()) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = true;
        while (!exitedDataConsumingLoop) {
            exitedDataConsumingLoopCv.wait(&consumptionMtx, 100);
        }
    }

    if (hook != nullptr) {
        delete hook;
        hook = nullptr;
    }
}

void LiveStatisticsConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&samplingRateMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRateHz = samplingRate.value;
    pushedSamplingRateFlag = true;
}

void LiveStatisticsConsumer::onDownsamplingRatioChanged(unsigned int ratio) {
    QMutexLocker locker(&samplingRateMtx);
    pushedDownsamplingRatio = ratio;
    pushedDownsamplingRatioFlag = true;
}

void LiveStatisticsConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void LiveStatisticsConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}

void LiveStatisticsConsumer::run() {
    this->initAnalysis();

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    consumptionLock.unlock();

    pushedVoltageRangeFlag = true;
    pushedCurrentRangeFlag = true;
    pushedSamplingRateFlag = true;
    pushedDownsamplingRatioFlag = true;

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        this->updateSamplingRate();
        this->updateRanges();

        if (hook!= nullptr && hook->getDataChunk(buffer, 1, minDataBatchSize)) {
            this->performAnalysis();
            hook->flush(); /*! Get rid of some data, these analyses will work anyway */
        }
    }

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void LiveStatisticsConsumer::initAnalysis() {
    results.resize(currentChannelsNum);
    voltageSum.resize(voltageChannelsNum);
    voltageSum2.resize(voltageChannelsNum);
    currentSum.resize(currentChannelsNum);
    currentSum2.resize(currentChannelsNum);
    this->lockAndResetAnalysis(currentChannelsNum);
}

void LiveStatisticsConsumer::lockAndResetAnalysis(int currentChannelIdx) {
    QMutexLocker locker(&mutex);
    this->resetAnalysis(currentChannelIdx);
}

void LiveStatisticsConsumer::resetAnalysis(int) {
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
        //std::fill(voltageSum2.begin(), voltageSum2.end(), 1.0);
        std::fill(currentSum.begin(), currentSum.end(), 0.0);
        std::fill(currentSum2.begin(), currentSum2.end(), 0.0);
    }

    for (analysisIdx = 0; analysisIdx < bufferLen; analysisIdx += totalChannelsNum*100) {
        for (voltageIdx = 0; voltageIdx < voltageChannelsNum; voltageIdx++) {
            channelIdx = analysisIdx+voltageIdx;
            voltageSum[voltageIdx] += buffer[channelIdx];
            // voltageSum2[voltageIdx] += analysisBuffer[channelIdx]*analysisBuffer[channelIdx];
        }

        for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
            channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
            bufferedValue = buffer[channelIdx];
            currentSum[currentIdx] += bufferedValue;
            currentSum2[currentIdx] += bufferedValue* bufferedValue;
        }
        totalAnalysisSamples++;
    }

    if (totalAnalysisSamples*100 >= minSamples) {
        for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
            const auto meanVoltage = (voltageSum[chIdx] / ((double)totalAnalysisSamples));

            // res->stdVoltage[voltageIdx] = qSqrt((voltageSum2[voltageIdx]-voltageSum[voltageIdx]*res->meanVoltage[voltageIdx])/((double)totalAnalysisSamples))*voltageMultiplier;
            const auto meanCurrent = currentSum[chIdx] / ((double)totalAnalysisSamples);
            const auto stdCurrent = qSqrt((currentSum2[chIdx] - currentSum[chIdx] * meanCurrent) / ((double)totalAnalysisSamples));
            const auto conductivity = meanVoltage * meanCurrent <= 0 ? -1.0 : meanCurrent / meanVoltage;
            const auto conductivityPfx = currentRange.prefix / voltageRange.prefix;

            const Measurement meanVoltageMeasurement = { meanVoltage , voltageRange.prefix, voltageRange.unit };
            const Measurement meanCurrentMeasurement = { meanCurrent, currentRange.prefix, currentRange.unit };
            const Measurement stdCurrentMeasurement = { stdCurrent, currentRange.prefix, currentRange.unit };
            const Measurement conductivityMeasurement = { conductivity, conductivityPfx, "S"};
            const StatisticsResult sr = { chIdx, meanVoltageMeasurement, meanCurrentMeasurement, stdCurrentMeasurement, conductivityMeasurement };
            results[chIdx] = sr;
        }
        totalAnalysisSamples = 0;
        StatisticsResultWrapper w = {results};
        emit sigResult(w);
    }
}

void LiveStatisticsConsumer::updateSamplingRate() {
    QMutexLocker locker(&samplingRateMtx);
    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;
        this->lockAndResetAnalysis(currentChannelsNum);
    }
}

void LiveStatisticsConsumer::updateRanges() {
    QMutexLocker locker(&rangesMtx);
    if (pushedVoltageRangeFlag) {
        pushedVoltageRangeFlag = false;
        voltageRange = pushedVoltageRange;
        this->lockAndResetAnalysis(currentChannelsNum);
    }

    if (pushedCurrentRangeFlag) {
        pushedCurrentRangeFlag = false;
        currentRange = pushedCurrentRange;
        this->lockAndResetAnalysis(currentChannelsNum);
    }
}

bool LiveStatisticsConsumer::isInVec(std::vector<int> vec, int elem){
    auto it = std::find(vec.begin(), vec.end(), elem);
    return it == vec.end();
}

void removeElem(std::vector<int> vec, int elem){
    auto it = std::find(vec.begin(), vec.end(), elem);
    vec.erase(it);
}
