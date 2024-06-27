#include "spectrumconsumer.h"

SpectrumConsumer::SpectrumConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    PlotConsumer(appStatus, producer) {

}

SpectrumConsumer::~SpectrumConsumer() {
    this->onStopConsuming();
    this->clearData();
}

void SpectrumConsumer::forceAxisUpdate() {
    pushedCurrentRangeFlag = true;
    updateRangeAxis();
    emitPlotData();
}

void SpectrumConsumer::updateFrequencyAxis() {
    rivedere tutto
//    QMutexLocker locker(&timeAxisMtx);
//    if (pushedDurationFlag) {
//        pushedDurationFlag = false;
//        xAxisDuration = pushedDuration;

//        locker.unlock();
//        this->computeTimeAxis();
//    }

//    // ottenere samplingrate
////    samplingRateHz
//    // ottenere durata
//    double integrationWindow = 1.0;
//    nBins = (int)round(integrationWindow*samplingRateHz);
//    n2Bins = nBins/2; // floor rounding: if nBins is even the DC frequeny is removed, but SR/2 is included, if nBins is odd DC included, SR/2 does not exist
//    this->allocateData();
}

void SpectrumConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    PlotConsumer::onCurrentRangeChanged(range);
//    cambiare unità
//    calculateBinSize();
//    allocateData();
//    emitPlotData();
}

void SpectrumConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    emitPlotData();
    int bufferIdx;
    int bufferLen = 0;
    int channelIdx;
    int binIndex = 0;

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
            bufferIdx = voltageChannelsNum;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx < bufferLen && binIndex < nBins) {
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        auto currentValue = buffer[bufferIdx + voltageChannelsNum];
                        fftIn[channelIdx][binIndex] = currentValue;
                    }
                    bufferIdx++;
                }
                bufferIdx += voltageChannelsNum;
                binIndex++;
            }

            if (binIndex == nBins) {
                for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        fftwPlan = fftw_plan_dft_r2c_1d(nBins, fftIn[channelIdx], reinterpret_cast <fftw_complex *> (fftOut[channelIdx]), FFTW_ESTIMATE);
                        fftw_execute(fftwPlan);
                        for (binIndex = 0; binIndex < n2Bins; binIndex++) {
                            currentValues[channelIdx][binIndex] = std::norm(fftOut[channelIdx][binIndex+1]);
                        }
                    }
                }
                emitPlotData();
                emit plotDataUpdated();

                binIndex = 0;
                // salvare i dati in eccesso nel buffer successivo
            }
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void SpectrumConsumer::allocateData() {
    bool wasThisRunning = isRunning();
    onStopConsuming();
    clearData();

    frequencyValues = new double[n2Bins];
    for (int binIdx = 0; binIdx < n2Bins; binIdx++) {
        frequencyValues[binIdx] = samplingRateHz*((double)(binIdx+1))/(double)nBins;
    }

    dataSize.resize(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentValues.push_back(new double[n2Bins]);
    }
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        fftIn.push_back(new double[nBins]);
        fftOut.push_back(new std::complex <double> [nBins]);
    }
    if (wasThisRunning) {
        onStartConsuming();
    }
}

void SpectrumConsumer::clearData() {
    for (int i = 0; i < currentValues.size(); i++) {
        delete [] currentValues[i];
    }
    currentValues.clear();
    for (int i = 0; i < fftIn.size(); i++) {
        delete [] fftIn[i];
    }
    fftIn.clear();
    for (int i = 0; i < fftOut.size(); i++) {
        delete [] fftOut[i];
    }
    fftOut.clear();
    delete [] frequencyValues;
    frequencyValues = nullptr;
    dataSize.clear();
}

void SpectrumConsumer::emitPlotData() {
    SpectrumMessage message = {frequencyValues, currentValues, dataSize};
    emit setPlotData(message);
};
