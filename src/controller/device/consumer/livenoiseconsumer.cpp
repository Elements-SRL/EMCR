#include "livenoiseconsumer.h"

#include <qmath.h>
#include <QFile>
#include <QDir>
#include <QTextStream>

LiveNoiseConsumer::LiveNoiseConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer) {

}

LiveNoiseConsumer::~LiveNoiseConsumer() {

}

void LiveNoiseConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        this->start();
    }
}

void LiveNoiseConsumer::onStopConsuming() {
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

void LiveNoiseConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&samplingRateMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRateHz = samplingRate.value;
    pushedSamplingRateFlag = true;
}

void LiveNoiseConsumer::onDownsamplingRatioChanged(unsigned int ratio) {
    QMutexLocker locker(&samplingRateMtx);
    pushedDownsamplingRatio = ratio;
    pushedDownsamplingRatioFlag = true;
}

void LiveNoiseConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void LiveNoiseConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}

void LiveNoiseConsumer::onExportLiveNoiseEstimates() {
    QString filename = "noise";
    QString filedir = QDir::currentPath() + "/";
    QString filepath = filedir + filename + ".csv";
    while (QFile::exists(filepath)) {
        filename += "_";
        filepath = filedir + filename + ".csv";
    }

    QFile file(filepath);
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
    for (auto noise : res.stdCurrent) {
        stream << noise << "\n";
    }
    file.close();
}

void LiveNoiseConsumer::run() {
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

        if (hook->getDataChunk(buffer, 1, minDataBatchSize)) {
            analysisBuffer.append(buffer);

            this->performAnalysis();
        }
    }

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void LiveNoiseConsumer::initAnalysis() {
    res.meanVoltage.resize(voltageChannelsNum);
    res.stdVoltage.resize(voltageChannelsNum);
    voltageSum.resize(voltageChannelsNum);
    voltageSum2.resize(voltageChannelsNum);
    res.meanCurrent.resize(currentChannelsNum);
    res.stdCurrent.resize(currentChannelsNum);
    currentSum.resize(currentChannelsNum);
    currentSum2.resize(currentChannelsNum);

    this->lockAndResetAnalysis(currentChannelsNum);
}

void LiveNoiseConsumer::lockAndResetAnalysis(int currentChannelIdx) {
    QMutexLocker locker(&mutex);
    this->resetAnalysis(currentChannelIdx);
}

void LiveNoiseConsumer::resetAnalysis(int) {
    analysisBuffer.clear();

    minSamples = qRound(sweepSamplingRate*LNC_MIN_INTERVAL_S);
}

void LiveNoiseConsumer::performAnalysis() {
    QMutexLocker locker(&mutex);
    bufferSize = analysisBuffer.size();
    analysisSamples = bufferSize/totalChannelsNum;
    int channelIdx;

    if (analysisSamples >= minSamples) {
        voltageSum.fill(0.0);
        voltageSum2.fill(0.0);
        currentSum.fill(0.0);
        currentSum2.fill(0.0);

        for (analysisIdx = 0; analysisIdx < bufferSize; analysisIdx += totalChannelsNum) {
            for (voltageIdx = 0; voltageIdx < voltageChannelsNum; voltageIdx++) {
                channelIdx = analysisIdx+voltageIdx;
                voltageSum[voltageIdx] += analysisBuffer[channelIdx];
                voltageSum2[voltageIdx] += analysisBuffer[channelIdx]*analysisBuffer[channelIdx];
            }

            for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
                channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
                currentSum[currentIdx] += analysisBuffer[channelIdx];
                currentSum2[currentIdx] += analysisBuffer[channelIdx]*analysisBuffer[channelIdx];
            }
        }

        analysisBuffer.clear();

        for (voltageIdx = 0; voltageIdx < voltageChannelsNum; voltageIdx++) {
            res.meanVoltage[voltageIdx] = voltageSum[voltageIdx]/((double)analysisSamples);
            res.stdVoltage[voltageIdx] = qSqrt((voltageSum2[voltageIdx]-voltageSum[voltageIdx]*res.meanVoltage[voltageIdx])/((double)analysisSamples))*voltageMultiplier;
            res.meanVoltage[voltageIdx] *= voltageMultiplier;
        }

        for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
            res.meanCurrent[currentIdx] = currentSum[currentIdx]/((double)analysisSamples);
            res.stdCurrent[currentIdx] = qSqrt((currentSum2[currentIdx]-currentSum[currentIdx]*res.meanCurrent[currentIdx])/((double)analysisSamples))*currentMultiplier;
            res.meanCurrent[currentIdx] *= currentMultiplier;
        }

        emit sigResult(res);
    }
}

void LiveNoiseConsumer::updateSamplingRate() {
    QMutexLocker locker(&samplingRateMtx);
    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        sweepSamplingRate = pushedSamplingRate/(double)pushedDownsamplingRatio;
        minDataBatchSize = qRound(sweepSamplingRate*0.05);
        this->lockAndResetAnalysis(currentChannelsNum);
    }
}

void LiveNoiseConsumer::updateRanges() {
    QMutexLocker locker(&rangesMtx);
    if (pushedVoltageRangeFlag) {
        pushedVoltageRangeFlag = false;
        voltageRange = pushedVoltageRange;
        voltageMultiplier = voltageRange.multiplier();
        this->lockAndResetAnalysis(currentChannelsNum);
    }

    if (pushedCurrentRangeFlag) {
        pushedCurrentRangeFlag = false;
        currentRange = pushedCurrentRange;
        currentMultiplier = currentRange.multiplier();
        this->lockAndResetAnalysis(currentChannelsNum);
    }
}
