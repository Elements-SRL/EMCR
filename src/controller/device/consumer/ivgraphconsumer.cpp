#include "ivgraphconsumer.h"
#include <QTime>

IvGraphConsumer::IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer):
    PlotConsumer(appStatus, producer) {
    /*! \todo FCON update binSize from user input? */
}

IvGraphConsumer::~IvGraphConsumer() {
    this->onStopConsuming();
    this->clearData();
}

void IvGraphConsumer::forceAxisUpdate() {
    pushedCurrentRangeFlag = true;
    updateRangeAxis();
    emitPlotData();
}

void IvGraphConsumer::onVoltageRangeChanged(){
    PlotConsumer::onVoltageRangeChanged();
    allocateData();
    emitPlotData();
}

void IvGraphConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    emitPlotData();
    int bufferIdx;
    int bufferLen = 0;
    QElapsedTimer updateDataTimer;
    updateDataTimer.start();

    int lastUpdateTimeMs = updateDataTimer.elapsed();
    int currentTimeMs;

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            consumptionLock.unlock();
            break;
        }
        consumptionLock.unlock();
        if (hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
            this->updateRangeAxis();
            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx + voltageChannelsNum < bufferLen) {
                for (auto channelIdx : expandedChannels) {
                    auto voltage = buffer[bufferIdx+channelIdx];
                    //                      use the voltage value to index the currents
                    auto binIndex = scaleToBins(channelIdx, voltage);
                    auto currentValue = buffer[bufferIdx+channelIdx + voltageChannelsNum];
                    if (ivChannels[channelIdx] != nullptr) {
                        ivChannels[channelIdx]->pushValue(binIndex, currentValue);
                    }
                }
                bufferIdx += totalChannelsNum;
            }
            currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs-lastUpdateTimeMs > IVC_MIN_UPDATE_PLOT_TIME_MS) {
                for (int i=0; i<currentChannelsNum; i++) {
                    auto currents = ivChannels[i]->getCurrents();
                    int counterOfSomeVariant = 0;
                    for(int c_idx = 0; c_idx<currents.size(); c_idx++) {
                        const auto current = currents[c_idx];
                        if(current.has_value()){
                            voltageData[i][counterOfSomeVariant] = voltageBins[c_idx];
                            currentValues[i][counterOfSomeVariant] = current.value();
                            counterOfSomeVariant++;
                        }
                    }
                    dataSize[i] = counterOfSomeVariant;
                }
                bool isDataChanged = false;
                for (int i=0; i<currentChannelsNum; i++) {
                    if (dataSize [i] != officialDataSize[i]) {
                        officialDataSize[i] = dataSize [i];
                        isDataChanged = true;
                    }
                }
                if (isDataChanged) {
                    emitPlotData();
                }
                emit plotDataUpdated();
                lastUpdateTimeMs = currentTimeMs;
            }
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

int IvGraphConsumer::scaleToBins(int channelIdx, double value) {
    value = (value > maxVoltageRange.max ? maxVoltageRange.max : value < maxVoltageRange.min ? maxVoltageRange.min : value);
    return static_cast<int>((value - maxVoltageRange.min) / binSize);
}

void IvGraphConsumer::allocateData() {
    bool wasThisRunning = isRunning();
    onStopConsuming();
    clearData();
    ivChannels.resize(currentChannelsNum);

    voltageData.resize(currentChannelsNum);
    for (int i=0; i< currentChannelsNum; i++) {
        voltageData[i] = new double[nBins];
    }
    dataSize.resize(currentChannelsNum);
    officialDataSize.resize(currentChannelsNum);
    voltageBins.resize(nBins);
    for (int i = 0; i<nBins; i++){
        voltageBins[i] = ((double) i) * binSize + maxVoltageRange.min;
    }
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentValues.push_back(new double[nBins]);
//        todo maybe get the n_bins from some type of configuration file
        ivChannels[idx] = new IvChannel(nBins, binSize);
    }
    if (wasThisRunning) {
        onStartConsuming();
    }
}

void IvGraphConsumer::calculateBinSize() {
    binSize = maxVoltageRange.delta() / ((double) (nBins - 1));
}

void IvGraphConsumer::emitPlotData() {
    IvMessage message = {voltageData, currentValues, dataSize};
    emit setPlotData(message);
}

void IvGraphConsumer::updateRangeAxis() {
    PlotConsumer::updateRangeAxis();
    this->calculateBinSize();
}

void IvGraphConsumer::clearData(){
    for (int i = 0; i < ivChannels.size(); i++) {
        delete ivChannels[i];
    }
    ivChannels.clear();
    for (int i = 0; i < currentValues.size(); i++) {
        delete [] currentValues[i];
    }
    currentValues.clear();
    for (int i = 0; i < voltageData.size(); i++) {
        delete [] voltageData[i];
    }
    voltageData.clear();
    voltageBins.clear();
    dataSize.clear();
    officialDataSize.clear();
}
