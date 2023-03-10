#include "plotconsumer.h"

#include <QTime>

PlotConsumer::PlotConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer) {

    voltageRange.prefix = UnitPfxNone;
    currentRange.prefix = UnitPfxNone;

    /*! Allocate buffer max size once and for all, so we avoid real time memory reallocations */
    buffer.reserve(DDP_DATA_PACKETS_BUFFER_LEN*totalChannelsNum);
}

PlotConsumer::~PlotConsumer() {

}

void PlotConsumer::forceAxisUpdate() {
    /*! This function is to be used only during class initialization, not when a consuming process is already running */
    this->updateTimeAxis();
    this->updateRangeAxis();
}

void PlotConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        this->start();
    }
}

void PlotConsumer::onStopConsuming() {
    if (this->isRunning()) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = true;
        while (!exitedDataConsumingLoop) {
            exitedDataConsumingLoopCv.wait(&consumptionMtx, 100);
        }
        consumptionLock.unlock();
    }

    if (hook != nullptr) {
        delete hook;
        hook = nullptr;
    }
}

void PlotConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&timeAxisMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRate = samplingRate.value;
    pushedSamplingRateFlag = true;
}

void PlotConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangeAxisMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void PlotConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangeAxisMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}

void PlotConsumer::onDurationChanged(Measurement_t duration) {
    QMutexLocker locker(&timeAxisMtx);
    duration.convertValue(UnitPfxNone);
    pushedDuration = duration.value;
    pushedDurationFlag = true;
}

void PlotConsumer::updateTimeAxis() {
    QMutexLocker locker(&timeAxisMtx);
    if (pushedDurationFlag || pushedSamplingRateFlag) {
        if (pushedDurationFlag) {
            pushedDurationFlag = false;
            sweepDuration = pushedDuration;
        }

        if (pushedSamplingRateFlag) {
            pushedSamplingRateFlag = false;
            sweepSamplingRate = pushedSamplingRate;
        }

        locker.unlock();
        this->computeTimeAxis();
        emit durationUpdated(sweepDuration);
    }
}

void PlotConsumer::computeTimeAxis() {
    dataSize = qRound(sweepSamplingRate*sweepDuration);
    minDataBatchSize = qRound(sweepSamplingRate*PCS_MIN_DATA_BATCH_DURATION_S);

    subSamplingRatio = (dataSize-1)/PCS_MAX_SAMPLES_PER_PLOT+1;
    dataSize /= subSamplingRatio;

    subSamplingIdx = 0;

    double dt = ((double)subSamplingRatio)/sweepSamplingRate;
    for (int idx = 0; idx < dataSize; idx++) {
        timeValues[idx] = dt*(double)idx;
    }

    this->emitPlotData();

    pushedDuration = sweepDuration;
}

void PlotConsumer::updateRangeAxis() {
    QMutexLocker locker(&rangeAxisMtx);
    if (pushedVoltageRangeFlag) {
        pushedVoltageRangeFlag = false;
        voltageRange.max = 1.0;
        voltageRange.convertValues(pushedVoltageRange.prefix);
        double coeff = voltageRange.max;
        for (int voltageChannelIdx = 0; voltageChannelIdx < voltageChannelsNum; voltageChannelIdx++) {
            for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                voltageValues[voltageChannelIdx][sampleIdx] *= coeff;
            }
        }
        voltageRange = pushedVoltageRange;

        emit voltageRangeUpdated(voltageRange);
    }

    if (pushedCurrentRangeFlag) {
        pushedCurrentRangeFlag = false;
        double coeff;
        currentRange.max = 1.0;
        currentRange.convertValues(pushedCurrentRange.prefix);
        coeff = currentRange.max;
        for (int currentChannelIdx = 0; currentChannelIdx < currentChannelsNum; currentChannelIdx++) {
            for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                currentValues[currentChannelIdx][sampleIdx] *= coeff;
            }
        }
        currentRange = pushedCurrentRange;

        emit currentRangeUpdated(currentRange);
    }
}

GapFreePlotConsumer::GapFreePlotConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    PlotConsumer(mDev, producer) {

    for (int idx = 0; idx < this->voltageChannelsNum; idx++) {
        voltageValues.append(new double[PCS_MAX_SAMPLES_PER_PLOT]());
    }

    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
        currentValues.append(new double[PCS_MAX_SAMPLES_PER_PLOT]());
    }

    timeValues = new double[PCS_MAX_SAMPLES_PER_PLOT];

    this->updateTimeAxis();
}

GapFreePlotConsumer::~GapFreePlotConsumer() {
    this->onStopConsuming();

    this->clearData();
}

void GapFreePlotConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;

    int bufferIdx;
    int bufferLen = 0;
    int voltageChannelIdx;
    int currentChannelIdx;

    QTime updateDataTimer = QTime::currentTime();
    updateDataTimer.start();

    int lastUpdateTimeMs = updateDataTimer.elapsed();
    int currentTimeMs;

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        if (hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
            this->updateTimeAxis();
            this->updateRangeAxis();

            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                for (voltageChannelIdx = 0; voltageChannelIdx < voltageChannelsNum; voltageChannelIdx++) {
                    voltageValues[voltageChannelIdx][gapFreeTimeIdx] = buffer[bufferIdx++];
                }

                for (currentChannelIdx = 0; currentChannelIdx < currentChannelsNum; currentChannelIdx++) {
                    currentValues[currentChannelIdx][gapFreeTimeIdx] = buffer[bufferIdx++];
                }

                gapFreeTimeIdx++;
                if (gapFreeTimeIdx >= dataSize) {
                    gapFreeTimeIdx = 0;
                }
            }

            currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs-lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS) {
                emit plotDataUpdated();
                lastUpdateTimeMs = currentTimeMs;
            }

        } else {
            /*! \todo FCON al momento questa cosa non accade mai, getDataChunk ritorna sempre true */
            break;
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void GapFreePlotConsumer::clearData() {
    for (int idx = 0; idx < voltageChannelsNum; idx++) {
        delete [] voltageValues[idx];
    }

    for (int idx = 0; idx < currentChannelsNum; idx++) {
        delete [] currentValues[idx];
    }

    if (timeValues != nullptr) {
        delete [] timeValues;
        timeValues = nullptr;
    }
}

void GapFreePlotConsumer::emitPlotData() {
    emit setPlotData(timeValues, &voltageValues, &currentValues, dataSize);
}
