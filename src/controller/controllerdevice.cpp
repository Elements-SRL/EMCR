#include "controllerdevice.h"

ControllerDevice::ControllerDevice(ModelDevice * mDev) :
    mDev(mDev)
{

}

void ControllerDevice::setModelDevice(ModelDevice * mDev){
    this->mDev = mDev;
}

// Slots (actionPerformed) for current and voltage ranges
// ADC Current Range in VC
void ControllerDevice::onVcCurrentRangeSelected(uint16_t selectedVcCurrentRangeIndex){
    vector <RangedMeasurement_t> ranges;
    uint16_t notUsedDefaultVcCurrRangeIdx;
    mDev->getVcCurrentRangesFeatures(ranges, notUsedDefaultVcCurrRangeIdx);

    this->mDev->setVcCurrentRange(selectedVcCurrentRangeIndex);
    this->mDev->setVcCurrentRange(ranges[selectedVcCurrentRangeIndex]);
    this->mDev->getMessageDispatcher()->setVCCurrentRange(selectedVcCurrentRangeIndex, true);

    emit sigVcCurrentRangeSelected(selectedVcCurrentRangeIndex);
}

// DAC Voltage Range in VC might be set by protocol
void ControllerDevice::onVcVoltageRangeSelected(uint16_t selectedVcVoltageRangeIndex){
    vector <RangedMeasurement_t> ranges;
    mDev->getVcVoltageRangesFeatures(ranges);

    this->mDev->setVcVoltageRange(selectedVcVoltageRangeIndex);
    this->mDev->setVcVoltageRange(ranges[selectedVcVoltageRangeIndex]);
    this->mDev->getMessageDispatcher()->setVCVoltageRange(selectedVcVoltageRangeIndex, true);

    emit sigVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
}

// DAC Current Range in CC might be set by protocol
void ControllerDevice::onCcCurrentRangeSelected(uint16_t selectedCcCurrentRangeIndex){
    vector <RangedMeasurement_t> ranges;
    mDev->getCcCurrentRangesFeatures(ranges);

    this->mDev->setCcCurrentRange(selectedCcCurrentRangeIndex);
    this->mDev->setCcCurrentRange(ranges[selectedCcCurrentRangeIndex]);
    this->mDev->getMessageDispatcher()->setCCCurrentRange(selectedCcCurrentRangeIndex, true);

    emit sigCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
}

// ADC Voltage Range in CC
void ControllerDevice::onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex){
    vector <RangedMeasurement_t> ranges;
    mDev->getCcVoltageRangesFeatures(ranges);

    this->mDev->setCcVoltageRange(selectedCcVoltageRangeIndex);
    this->mDev->setCcVoltageRange(ranges[selectedCcVoltageRangeIndex]);
    this->mDev->getMessageDispatcher()->setCCVoltageRange(selectedCcVoltageRangeIndex, true);

    emit sigCcVoltageRangeSelected(selectedCcVoltageRangeIndex);
}

// Slots (actionPerformed) for current and voltage filters
// ADC Current Filter in VC set by Sampling Rate

// DAC Voltage Filter in VC
void ControllerDevice::onVcVoltageFilterSelected(uint16_t selectedVcVoltageFilterIndex){
    vector <Measurement_t> filters;
    mDev->getVoltageStimulusLpfsFeatures(filters);

    this->mDev->setVcVoltageFilter(selectedVcVoltageFilterIndex);
    this->mDev->setVcVoltageFilter(filters[selectedVcVoltageFilterIndex]);
    this->mDev->getMessageDispatcher()->setVoltageStimulusLpf(selectedVcVoltageFilterIndex, true);

    emit sigVcVoltageFilterSelected(selectedVcVoltageFilterIndex);
}

// DAC Current Filter in CC
void ControllerDevice::onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex){
    vector <Measurement_t> filters;
    mDev->getCurrentStimulusLpfsFeatures(filters);

    this->mDev->setCcCurrentFilter(selectedCcCurrentFilterIndex);
    this->mDev->setCcCurrentFilter(filters[selectedCcCurrentFilterIndex]);
    this->mDev->getMessageDispatcher()->setCurrentStimulusLpf(selectedCcCurrentFilterIndex, true);

    emit sigCcCurrentFilterSelected(selectedCcCurrentFilterIndex);
}

// Sampling rate
void ControllerDevice::onSamplingRateSelected(uint16_t selectedSamplingRateIndex){
    vector <Measurement_t> samplingRates;
    mDev->getSamplingRatesFeatures(samplingRates);

    this->mDev->setSamplingRate(selectedSamplingRateIndex);
    this->mDev->setSamplingRate(samplingRates[selectedSamplingRateIndex]);
    this->mDev->getMessageDispatcher()->setSamplingRate(selectedSamplingRateIndex, true);

    emit sigSamplingRateSelected(selectedSamplingRateIndex);
}
// ADC Voltage Filter in CC set by Sampling rate

void ControllerDevice::onClampingModalitySelected(uint16_t selectedClampingModalityIndex){
    vector <int> clampingModalities;
    mDev->getClampingModalitiesFeatures(clampingModalities);

    this->mDev->setOngoingClampingModalityIdx(selectedClampingModalityIndex);
    this->mDev->setOngoingClampingModality(clampingModalities[selectedClampingModalityIndex]);

    int voltageChannelsNum;
    int currentChannelsNum;
    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);
    vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    vector <bool> allTrue(currentChannelsNum, true);
    vector <bool> allFalse(currentChannelsNum, false);

    if (clampingModalities[selectedClampingModalityIndex] == E384CL_VOLTAGE_CLAMP_MODE) {
        mDev->getMessageDispatcher()->turnCcSwOn(allChannels, allFalse, false);
        mDev->getMessageDispatcher()->enableCcStimulus(allChannels, allFalse, false);
        mDev->getMessageDispatcher()->turnVcSwOn(allChannels, allTrue, false);
        mDev->getMessageDispatcher()->turnVcCcSelOn(allChannels, allTrue, false);
        mDev->getMessageDispatcher()->setSourceForVoltageChannel(0, false);
        mDev->getMessageDispatcher()->setSourceForCurrentChannel(0, false);
        mDev->getMessageDispatcher()->setVCCurrentRange(mDev->getVcCurrentRangeIdx(), false);
        mDev->getMessageDispatcher()->setVCVoltageRange(mDev->getVcVoltageRangeIdx(), false);
        mDev->getMessageDispatcher()->setDebugBit(0, 7, false);

    } else {
        mDev->getMessageDispatcher()->turnVcSwOn(allChannels, allFalse, false);
        mDev->getMessageDispatcher()->turnCcSwOn(allChannels, allTrue, false);
        mDev->getMessageDispatcher()->enableCcStimulus(allChannels, allTrue, false);
        mDev->getMessageDispatcher()->turnVcCcSelOn(allChannels, allFalse, false);
        mDev->getMessageDispatcher()->setSourceForVoltageChannel(1, false);
        mDev->getMessageDispatcher()->setSourceForCurrentChannel(1, false);
        mDev->getMessageDispatcher()->setCCCurrentRange(mDev->getCcCurrentRangeIdx(), false);
        mDev->getMessageDispatcher()->setCCVoltageRange(mDev->getCcVoltageRangeIdx(), false);
        mDev->getMessageDispatcher()->setDebugBit(0, 7, true);
    }

    emit sigClampingModalitySelected(selectedClampingModalityIndex);
}
