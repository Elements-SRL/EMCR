#include "ivgraphconsumer.h"
#include <QTime>
#include <iostream>

IvGraphConsumer::IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer):
    PlotConsumer(appStatus, producer) {
//    todo read from file this value?
    this->nBins = 1600;
    calculateBinSize();
}

IvGraphConsumer::~IvGraphConsumer() {
    this->onStopConsuming();
    this->clearData();
}

void IvGraphConsumer::forceAxisUpdate() {
    pushedCurrentRangeFlag = true;
    updateRangeAxis();
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
            while (bufferIdx < bufferLen) {
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        auto voltage = buffer[bufferIdx];
//                      use the voltage value to index the currents
                        auto binIndex = scaleToBins(voltage);
                        auto it = std::find(indexes.begin(), indexes.end(), binIndex);
                        if (it == indexes.end()) {
                            indexes.push_back(binIndex);
                            std::sort(indexes.begin(), indexes.end());
                        }
                        bufferIdx++;
                        auto currentValue = buffer[bufferIdx];
                        ivChannels[channelIdx]->pushValue(binIndex, currentValue);
                        bufferIdx++;
                    } else {
                        bufferIdx+=2;
                    }
                }
            }
            currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs-lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS) {
                for (int i=0; i<currentChannelsNum; i++) {
                    auto currents = ivChannels[i]->getCurrents();
                    for (int j=0; j< nBins; j++){
                        currentValues[i][j] = currents[j];
                        std::cout << " " << currentValues[i][j];
                    }
                }
                std::cout << std::endl;
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
    ivChannels.reserve(currentChannelsNum);
    voltageData.resize(nBins);
    for (int i = 0; i<nBins; i++){
        voltageData[i] = ((double) i) * binSize + pushedVoltageRange.min;
    }
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentValues.push_back(new double[nBins]);
//        todo maybe get the n_bins from some type of configuration file
        ivChannels.push_back(new IvChannel(nBins, binSize));
    }
}

//todo call this method when bin size changes or when voltage range changes
void IvGraphConsumer::calculateBinSize(){
    // Calculate the size of each bin
    binSize = pushedVoltageRange.delta() / ((double) nBins);
}

void IvGraphConsumer::emitPlotData() {
    IvMessage message = {voltageData, currentValues, nBins};
    emit setPlotData(message);
}

void IvGraphConsumer::clearData(){
    ivChannels.clear();
    currentValues.clear();
    voltageData.clear();
}
