#include "ivgraphconsumer.h"
#include <QTime>
#include <iostream>

IvGraphConsumer::IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer):
    PlotConsumer(appStatus, producer) {
//    todo read from file this value?
    this->nBins = 3201;
    calculateBinSize();
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

void IvGraphConsumer::onVoltageRangeChanged(RangedMeasurement_t range){
    PlotConsumer::onVoltageRangeChanged(range);
    calculateBinSize();
    allocateData();
    emitPlotData();
}

void IvGraphConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    emitPlotData();
    int bufferIdx;
    int bufferLen = 0;
    int channelIdx;
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
//            this->updateTimeAxis();
//            this->updateRangeAxis();
            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx + voltageChannelsNum < bufferLen) {
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        auto voltage = buffer[bufferIdx];
//                      use the voltage value to index the currents
                        auto binIndex = scaleToBins(voltage);
                        auto currentValue = buffer[bufferIdx + voltageChannelsNum];
                        if (ivChannels[channelIdx] != NULL) {
                            ivChannels[channelIdx]->pushValue(binIndex, currentValue);
                        }    
                    }
                    bufferIdx++;
                }
                bufferIdx+=currentChannelsNum;
            }
            currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs-lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS) {
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

// Function to scale a value into a number of bins
int IvGraphConsumer::scaleToBins(double value) {
    // Calculate the adjusted value to lie within [0, 2*v]
    return static_cast<int>((value - pushedVoltageRange.min) / binSize);
}

void IvGraphConsumer::allocateData() {
    bool wasThisRunning = isRunning();
    onStopConsuming();
    clearData();
    ivChannels.resize(currentChannelsNum);

    double * precalculatedVoltages = new double[nBins];
    for (int i = 0; i<nBins; i++){
        precalculatedVoltages[i] = ((double) i) * binSize + pushedVoltageRange.min;
    }
    voltageData.resize(currentChannelsNum);
    for (int i=0; i< currentChannelsNum; i++) {
        voltageData[i] = new double[nBins];
    }
    dataSize.resize(currentChannelsNum);
    officialDataSize.resize(currentChannelsNum);
    voltageBins.resize(nBins);
    for (int i = 0; i<nBins; i++){
        voltageBins[i] = ((double) i) * binSize + pushedVoltageRange.min;
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

//todo call this method when bin size changes or when voltage range changes
void IvGraphConsumer::calculateBinSize(){
    // Calculate the size of each bin
    binSize = pushedVoltageRange.delta() / ((double) (nBins - 1));
}

void IvGraphConsumer::emitPlotData() {
    IvMessage message = {voltageData, currentValues, dataSize};
    emit setPlotData(message);
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
