#include "spectrumconsumer.h"

SpectrumConsumer::SpectrumConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    PlotConsumer(appStatus, producer) {

    maxSamples = 2048;
    maxSamples2 = maxSamples/2+1;
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
    pushedVoltageRangeFlag = true;
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
        integrationWindowS = pushedIntegrationWindowS;

        if (!pushedSamplingRateFlag && !pushedDownsamplingRatioFlag) { // if any of these is true the locker is still needed and the computeFrequencyAxis method is performed later
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
    n2Bins = nBins/2; // floor rounding: if nBins is even the DC frequency is removed, but SR/2 is included, if nBins is odd DC included, SR/2 does not exist
    if (n2Bins == 0) {
        return;
    }

    integrationRoundIdx = 0;
    binIndex = 0;

    df = samplingRateHz/(double)nBins;
    // avoid DC component
    for (int binIdx = 1; binIdx < n2Bins+1; binIdx++) {
        frequencyValues[binIdx-1] = df*(double)binIdx;
    }

    shape = {static_cast <size_t> (nBins)};
    /*! This ensures that integrating the power spectrum (int{S*dF}) returns the signal variance
     *  2 because the spectrum is mono lateral
     *  samplingRateHz to give information about the x axis (it will cancel out with dF = Fs/nBins in the integral)
     *  nBins to take into account that the fft is computed from the sum of nBins elements */
    normalizationFactor = 2.0/(samplingRateHz*(double)(integrationRounds*nBins-1));

    this->emitPlotData();
    emit sigRangeUpdate({frequencyValues[0], frequencyValues[n2Bins-1], df, UnitPfxNone, "Hz"});
}

void SpectrumConsumer::updateRangeAxis() {
    QMutexLocker locker(&rangeAxisMtx);
    bool anythingChanged = false;
    if (pushedCurrentRangeFlag) {
        anythingChanged = true;
        pushedCurrentRangeFlag = false;
        currentRange = pushedCurrentRange;
    }
    if (pushedVoltageRangeFlag) {
        anythingChanged = true;
        pushedVoltageRangeFlag = false;
        voltageRange = pushedVoltageRange;
    }
    if (anythingChanged) {
        ClampingModality_t mode;
        appStatus->getMessageDispatcher()->getClampingModality(mode);
        switch (mode) {
        case e384CommLib::VOLTAGE_CLAMP:
            channelsOffset = voltageChannelsNum;
            break;

        case e384CommLib::CURRENT_CLAMP:
        case e384CommLib::ZERO_CURRENT_CLAMP:
            channelsOffset = 0;
            break;
        }
    }
}

void SpectrumConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    emitPlotData();
    int bufferIdx;
    int bufferLen = 0;
    bool emitFlag = false;

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
            this->updateFrequencyAxis();
            this->updateRangeAxis();
            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in buffers for FFT evaluation */
            while (bufferIdx + channelsOffset < bufferLen) {
                for (auto channelIdx : channels) {
                    auto currentValue = buffer[bufferIdx + channelIdx + channelsOffset];
                    fftIn[channelIdx][binIndex] = currentValue;
                }
                bufferIdx += totalChannelsNum;
                binIndex++;

                /*! Enough data to compute FFT */
                if (binIndex == nBins) {
                    if (integrationRoundIdx == 0) {
                        for (auto channelIdx : channels) {
                            pocketfft::r2c(shape, stride_in, stride_out, axes, pocketfft::FORWARD, fftIn[channelIdx], fftOut[channelIdx], 1.0);
                            for (binIndex = 1; binIndex < n2Bins+1; binIndex++) {
                                currentValues[channelIdx][binIndex-1] = std::norm(fftOut[channelIdx][binIndex]);
                            }
                        }
                    }
                    else {
                        for (auto channelIdx : channels) {
                            pocketfft::r2c(shape, stride_in, stride_out, axes, pocketfft::FORWARD, fftIn[channelIdx], fftOut[channelIdx], 1.0);
                            for (binIndex = 1; binIndex < n2Bins+1; binIndex++) {
                                currentValues[channelIdx][binIndex-1] += std::norm(fftOut[channelIdx][binIndex]);
                            }
                        }
                    }

                    /*! Enough FFTs to estimate spectrum */
                    if (++integrationRoundIdx == integrationRounds) {
                        for (auto channelIdx : channels) {
                            for (binIndex = 0; binIndex < n2Bins; binIndex++) {
                                currentSpectrumValues[channelIdx][binIndex] = currentValues[channelIdx][binIndex]*normalizationFactor;
                                if (binIndex == 0) {
                                    irmsValues[channelIdx][binIndex] = currentSpectrumValues[channelIdx][binIndex]*df;
                                }
                                else {
                                    irmsValues[channelIdx][binIndex] = irmsValues[channelIdx][binIndex-1]+currentSpectrumValues[channelIdx][binIndex]*df;
                                }
                            }
                            for (binIndex = 0; binIndex < n2Bins; binIndex++) {
                                irmsValues[channelIdx][binIndex] = sqrt(irmsValues[channelIdx][binIndex]);
                            }
                        }
                        integrationRoundIdx = 0;
                        emitFlag = true;
                    }
                    binIndex = 0;
                }
            }
            if (emitFlag) {
                emit plotDataUpdated();
                emitFlag = false;
            }
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void SpectrumConsumer::allocateData() {
    frequencyValues = new double[maxSamples2];

    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentValues.push_back(new double[maxSamples2]);
        currentSpectrumValues.push_back(new double[maxSamples2]);
        irmsValues.push_back(new double[maxSamples2]);
        fftIn.push_back(new double[maxSamples]);
        fftOut.push_back(new std::complex <double> [maxSamples2]);
    }
}

void SpectrumConsumer::clearData() {
    for (int i = 0; i < currentValues.size(); i++) {
        delete [] currentValues[i];
    }
    currentValues.clear();

    for (int i = 0; i < currentSpectrumValues.size(); i++) {
        delete [] currentSpectrumValues[i];
    }
    currentSpectrumValues.clear();

    for (int i = 0; i < irmsValues.size(); i++) {
        delete [] irmsValues[i];
    }
    irmsValues.clear();

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
}

void SpectrumConsumer::emitPlotData() {
    SpectrumMessage message = {frequencyValues, currentSpectrumValues, irmsValues, n2Bins};
    emit setPlotData(message);
};
