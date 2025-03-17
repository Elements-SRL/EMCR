#include "analysisconsumer.h"


AnalysisConsumer::AnalysisConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    DeviceDataConsumer(appStatus, producer) {
    buffer.reserve(producer->getDataPacketsBufferLen()*totalChannelsNum); /*! \todo FCON magari renderlo comportamento di default della classe base */
}

void AnalysisConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = false;
        exitedDataConsumingLoop = false;

        this->start();
    }
}

void AnalysisConsumer::onStopConsuming() {
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

void AnalysisConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&samplingRateMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRateHz = samplingRate.value;
    pushedSamplingRateFlag = true;
}

void AnalysisConsumer::onDownsamplingRatioChanged(unsigned int ratio) {
    QMutexLocker locker(&samplingRateMtx);
    pushedDownsamplingRatio = ratio;
    pushedDownsamplingRatioFlag = true;
}

void AnalysisConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void AnalysisConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}

void AnalysisConsumer::run() {
    this->initAnalysis();

    QMutexLocker consumptionLock(&consumptionMtx);
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
            if (flushAfterAnalysisFlag) {
                hook->flush();
            }
        }
    }

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void AnalysisConsumer::lockAndResetAnalysis() {
    QMutexLocker locker(&mutex);
    this->resetAnalysis();
}

void AnalysisConsumer::updateSamplingRate() {
    QMutexLocker locker(&samplingRateMtx);
    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;
        this->lockAndResetAnalysis();
    }
}

void AnalysisConsumer::updateRanges() {
    QMutexLocker locker(&rangesMtx);
    if (pushedVoltageRangeFlag) {
        pushedVoltageRangeFlag = false;
        voltageRange = pushedVoltageRange;
        this->lockAndResetAnalysis();
    }

    if (pushedCurrentRangeFlag) {
        pushedCurrentRangeFlag = false;
        currentRange = pushedCurrentRange;
        this->lockAndResetAnalysis();
    }
}
