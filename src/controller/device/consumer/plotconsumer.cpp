#include "plotconsumer.h"

#include <QTime>

PlotConsumer::PlotConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer) {

    voltageRange.prefix = UnitPfxNone;
    currentRange.prefix = UnitPfxNone;

    /*! Allocate buffer max size once and for all, so we avoid real time memory reallocations */
    buffer.reserve(DDP_DATA_PACKETS_BUFFER_LEN*totalChannelsNum);

    selectedChannels.resize(currentChannelsNum);
    selectedChannels.fill(true);


    logFile.setFileName(QString("log%1.txt").arg((unsigned int)this));
    logFile.open(QFile::WriteOnly | QFile::Truncate);
    logStream.setDevice(&logFile);
}

PlotConsumer::~PlotConsumer() {
    logFile.close();
}

void PlotConsumer::forceAxisUpdate() {
    pushedDurationFlag = true;
    pushedVoltageRangeFlag = true;
    pushedCurrentRangeFlag = true;
    this->updateTimeAxis();
    this->updateRangeAxis();
}

void PlotConsumer::setMaxSamplesPerPlot(int samples) {
    bool wasThisRunning = this->isRunning();
    if(wasThisRunning){
        this->onStopConsuming();
    }

    this->clearData();
    maxSamples = samples;
    this->allocateData();

    if(wasThisRunning){
        this->onStartConsuming();
    }

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
    }

    if (hook != nullptr) {
        delete hook;
        hook = nullptr;
    }
}

void PlotConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&timeAxisMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRateHz = samplingRate.value;
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

void PlotConsumer::onSelectChannels(vector<uint16_t> channelIndexes, vector <bool> channels) {
    bool wasThisRunning = this->isRunning();
    if(wasThisRunning){
        this->onStopConsuming();
    }

    selectedChannels.fill(false);
    this->channelsAtTrue = 0;
    for (int i = 0; i<channels.size(); i++){
        if(channels[i]){
            selectedChannels[channelIndexes[i]] = true;
            channelsAtTrue++;
        }
    }
    forceAxisUpdate();
    if(wasThisRunning){
        this->onStartConsuming();
    }
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
            sweepSamplingRateHz = pushedSamplingRateHz;
        }

        locker.unlock();
        this->computeTimeAxis();
        emit durationUpdated(sweepDuration);
    }
}

void PlotConsumer::computeTimeAxis() {
    dataSize = qRound(sweepSamplingRateHz*sweepDuration);
    minDataBatchSize = qMin(qRound(sweepSamplingRateHz*PCS_MIN_DATA_BATCH_DURATION_S), DDP_DATA_PACKETS_BUFFER_LEN/4);

    subSamplingRatio = (dataSize-1)/maxSamples+1;
    dataSize /= subSamplingRatio;

#ifdef GLB_SHOW_DEBUG_CTRLS
    logStream << "sweepSamplingRateHz " << sweepSamplingRateHz;
    logStream << " --- sweepDuration " << sweepDuration;
    logStream << " --- initial dataSize " << qRound(sweepSamplingRateHz*sweepDuration);
    logStream << " --- final dataSize " << dataSize;
    logStream << " --- minDataBatchSize " << minDataBatchSize;
    logStream << " --- subSamplingRatio " << subSamplingRatio;
    logStream << " --- maxSamples " << maxSamples << endl;
#endif

    subSamplingIdx = 0;

    double dt = ((double)subSamplingRatio)/sweepSamplingRateHz;
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
        int counter = 0;
        for (int channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
            if (selectedChannels[channelIdx]) {
                for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                    voltageValues[counter][sampleIdx] *= coeff;
                }
                counter++;
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
        int counter = 0;
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            if (selectedChannels[channelIdx]) {
                for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                    currentValues[counter][sampleIdx] *= coeff;
                }
                counter++;
            }
        }
        currentRange = pushedCurrentRange;

        emit currentRangeUpdated(currentRange);
    }
}

GapFreePlotConsumer::GapFreePlotConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    PlotConsumer(mDev, producer) {

    this->allocateData();
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
    int channelIdx;
    int counter;

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
                counter = 0;
                for (channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
                    if (selectedChannels[channelIdx]) {
                        voltageValues[counter++][gapFreeTimeIdx] = buffer[bufferIdx];
                    }
                    bufferIdx++;
                }

                counter = 0;
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (selectedChannels[channelIdx]) {
                        currentValues[counter++][gapFreeTimeIdx] = buffer[bufferIdx];
                    }
                    bufferIdx++;
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
            break;
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void GapFreePlotConsumer::allocateData() {
    for (int idx = 0; idx < this->voltageChannelsNum; idx++) {
        voltageValues.append(new double[maxSamples]);
    }

    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
        currentValues.append(new double[maxSamples]);
    }

    timeValues = new double[maxSamples];
    forceAxisUpdate();
}

void GapFreePlotConsumer::clearData() {
    for (int idx = 0; idx < voltageChannelsNum; idx++) {
        delete [] voltageValues[idx];
    }
    voltageValues.clear();

    for (int idx = 0; idx < currentChannelsNum; idx++) {
        delete [] currentValues[idx];
    }
    currentValues.clear();

    if (timeValues != nullptr) {
        delete [] timeValues;
        timeValues = nullptr;
    }
}

void GapFreePlotConsumer::emitPlotData() {
    emit setPlotData(timeValues, &voltageValues, &currentValues, dataSize, this->channelsAtTrue);
}
