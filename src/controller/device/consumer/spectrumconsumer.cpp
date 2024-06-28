#include "spectrumconsumer.h"

SpectrumConsumer::SpectrumConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    PlotConsumer(appStatus, producer) {

    maxSamples = 2048;
    this->allocateData();
    this->updateFrequencyAxis();
}

SpectrumConsumer::~SpectrumConsumer() {
    this->onStopConsuming();

    this->clearData();
}

void SpectrumConsumer::forceAxisUpdate() {
    pushedIntegrationWindowFlag = true;
    pushedCurrentRangeFlag = true;
    this->updateFrequencyAxis();
    this->updateRangeAxis();
}

void SpectrumConsumer::onIntegrationWindowChanged(Measurement_t window) {
    QMutexLocker locker(&timeAxisMtx);
    pushedIntegrationWindowS = window.getNoPrefixValue();
    pushedIntegrationWindowFlag = true;
}

void SpectrumConsumer::updateFrequencyAxis() {
    QMutexLocker locker(&timeAxisMtx);
    if (pushedIntegrationWindowFlag) {
        pushedIntegrationWindowFlag = false;
        xAxisDuration = pushedIntegrationWindowS;

        if (!pushedSamplingRateFlag && !pushedDownsamplingRatioFlag) { // if any of these is true the locker is still needed and the computeTimeAxisMethod is performed later
            locker.unlock();
            this->computeFrequencyAxis();
        }
    }

    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;

        locker.unlock();
        this->computeFrequencyAxis();
    }
}

void SpectrumConsumer::computeFrequencyAxis() {
    nBins = qRound(integrationWindowS*samplingRateHz);
    integrationRounds = (nBins-1)/maxSamples+1;
    nBins /= integrationRounds;
    n2Bins = nBins/2; // floor rounding: if nBins is even the DC frequeny is removed, but SR/2 is included, if nBins is odd DC included, SR/2 does not exist

    integrationRoundIdx = 0;

    double df = samplingRateHz/(double)nBins;
    for (int binIdx = 0; binIdx < n2Bins; binIdx++) {
        frequencyValues[binIdx] = df*(double)(binIdx+1);
    }

    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        fftw_plan_dft_r2c_1d(nBins, fftIn[channelIdx], reinterpret_cast <fftw_complex *> (fftOut[channelIdx]), FFTW_ESTIMATE);
        // va fatto anche il destroy dei plan? ha impatto? dove andrebbe fatto e come si verifica se il piano esiste?
    }

    this->emitPlotData();
}

void SpectrumConsumer::updateRangeAxis() {
    QMutexLocker locker(&rangeAxisMtx);
    if (pushedCurrentRangeFlag) {
        pushedCurrentRangeFlag = false;
        currentRange = pushedCurrentRange;

        emit currentRangeUpdated(currentRange);
    }
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
            this->updateFrequencyAxis();
            this->updateRangeAxis();
            bufferIdx = voltageChannelsNum;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        auto currentValue = buffer[bufferIdx + voltageChannelsNum];
                        fftIn[channelIdx][binIndex] = currentValue;
                    }
                    bufferIdx++;
                }
                bufferIdx += voltageChannelsNum;
                binIndex++;

                /*! Enough data to compute FFT */
                if (binIndex == nBins) {
                    if (integrationRoundIdx == 0) {
                        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                            if (plottedChannels[channelIdx]) {
                                fftw_execute(fftwPlans[channelIdx]);
                                for (binIndex = 0; binIndex < n2Bins; binIndex++) {
                                    currentValues[channelIdx][binIndex] = std::norm(fftOut[channelIdx][binIndex+1]);
                                }
                            }
                        }

                    } else {
                        if (plottedChannels[channelIdx]) {
                            fftw_execute(fftwPlans[channelIdx]);
                            for (binIndex = 0; binIndex < n2Bins; binIndex++) {
                                currentValues[channelIdx][binIndex] += std::norm(fftOut[channelIdx][binIndex+1]);
                            }
                        }
                    }

                    /*! Enough FFTs to estimate spectrum */
                    if (++integrationRoundIdx == integrationRounds) {
                        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                            if (plottedChannels[channelIdx]) {
                                for (binIndex = 0; binIndex < n2Bins; binIndex++) {
                                    currentValues[channelIdx][binIndex] *= normalizationFactor;
                                }
                            }
                        }
                        emitPlotData();
                        emit plotDataUpdated();
                        integrationRoundIdx = 0;
                    }
                    binIndex = 0;

                    // salvare i dati in eccesso nel buffer successivo
                }
            }

        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void SpectrumConsumer::allocateData() {
    frequencyValues = new double[maxSamples/2];

    dataSize.resize(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentValues.push_back(new double[maxSamples]);
    }
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        fftIn.push_back(new double[nBins]);
        fftOut.push_back(new std::complex <double> [maxSamples]);
    }
    fftwPlans.resize(currentChannelsNum);
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
