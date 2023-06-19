#include "controllerdevice.h"

ControllerDevice::ControllerDevice(ModelDevice * mDev) :
    mDev(mDev)
{
    deviceControlDockWidget = new DeviceControlDockWidget(mDev);
}

DeviceControlDockWidget * ControllerDevice::getDeviceControlDockWidget(){
    return deviceControlDockWidget;
}


// Slots (actionPerformed) for current and voltage ranges
// ADC Current Range in VC
void ControllerDevice::onVcCurrentRangeSelected(uint16_t selectedVcCurrentRangeIndex){
    std::vector <RangedMeasurement_t> ranges;
    uint16_t notUsedDefaultVcCurrRangeIdx;
    mDev->getVcCurrentRangesFeatures(ranges, notUsedDefaultVcCurrRangeIdx);

    this->mDev->setVcCurrentRange(selectedVcCurrentRangeIndex);
    this->mDev->setVcCurrentRange(ranges[selectedVcCurrentRangeIndex]);
    this->mDev->getMessageDispatcher()->setVCCurrentRange(selectedVcCurrentRangeIndex, true);

    emit sigVcCurrentRangeSelected(selectedVcCurrentRangeIndex);
}

// DAC Voltage Range in VC might be set by protocol
void ControllerDevice::onVcVoltageRangeSelected(uint16_t selectedVcVoltageRangeIndex){
    std::vector <RangedMeasurement_t> ranges;
    mDev->getVcVoltageRangesFeatures(ranges);

    this->mDev->setVcVoltageRange(selectedVcVoltageRangeIndex);
    this->mDev->setVcVoltageRange(ranges[selectedVcVoltageRangeIndex]);
    this->mDev->getMessageDispatcher()->setVCVoltageRange(selectedVcVoltageRangeIndex, true);

    emit sigVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
}

// DAC Current Range in CC might be set by protocol
void ControllerDevice::onCcCurrentRangeSelected(uint16_t selectedCcCurrentRangeIndex){
    std::vector <RangedMeasurement_t> ranges;
    mDev->getCcCurrentRangesFeatures(ranges);

    this->mDev->setCcCurrentRange(selectedCcCurrentRangeIndex);
    this->mDev->setCcCurrentRange(ranges[selectedCcCurrentRangeIndex]);
    this->mDev->getMessageDispatcher()->setCCCurrentRange(selectedCcCurrentRangeIndex, true);

    emit sigCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
}

// ADC Voltage Range in CC
void ControllerDevice::onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex){
    std::vector <RangedMeasurement_t> ranges;
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
    std::vector <Measurement_t> filters;
    mDev->getVoltageStimulusLpfsFeatures(filters);

    this->mDev->setVcVoltageFilter(selectedVcVoltageFilterIndex);
    this->mDev->setVcVoltageFilter(filters[selectedVcVoltageFilterIndex]);
    this->mDev->getMessageDispatcher()->setVoltageStimulusLpf(selectedVcVoltageFilterIndex, true);

    emit sigVcVoltageFilterSelected(selectedVcVoltageFilterIndex);
}

// DAC Current Filter in CC
void ControllerDevice::onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex){
    std::vector <Measurement_t> filters;
    mDev->getCurrentStimulusLpfsFeatures(filters);

    this->mDev->setCcCurrentFilter(selectedCcCurrentFilterIndex);
    this->mDev->setCcCurrentFilter(filters[selectedCcCurrentFilterIndex]);
    this->mDev->getMessageDispatcher()->setCurrentStimulusLpf(selectedCcCurrentFilterIndex, true);

    emit sigCcCurrentFilterSelected(selectedCcCurrentFilterIndex);
}

// Sampling rate
void ControllerDevice::onSamplingRateSelected(uint16_t selectedSamplingRateIndex){
    std::vector <Measurement_t> samplingRates;
    mDev->getSamplingRatesFeatures(samplingRates);

    this->mDev->setSamplingRate(selectedSamplingRateIndex);
    this->mDev->setSamplingRate(samplingRates[selectedSamplingRateIndex]);
    this->mDev->getMessageDispatcher()->setSamplingRate(selectedSamplingRateIndex, true);

    emit sigSamplingRateSelected(selectedSamplingRateIndex);
}
// ADC Voltage Filter in CC set by Sampling rate

void ControllerDevice::onClampingModalitySelected(uint16_t selectedClampingModalityIndex){
    std::vector <int> clampingModalities;
    mDev->getClampingModalitiesFeatures(clampingModalities);

    this->mDev->setOngoingClampingModalityIdx(selectedClampingModalityIndex);
    this->mDev->setOngoingClampingModality(clampingModalities[selectedClampingModalityIndex]);

    int voltageChannelsNum;
    int currentChannelsNum;
    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    std::vector <bool> allTrue(currentChannelsNum, true);
    std::vector <bool> allFalse(currentChannelsNum, false);

    if (clampingModalities[selectedClampingModalityIndex] == ClampingModality_t::VOLTAGE_CLAMP) {
        mDev->getMessageDispatcher()->enableCcCompensations(false);
        mDev->getMessageDispatcher()->turnCurrentStimulusOn(false, false);
        mDev->getMessageDispatcher()->turnVoltageReaderOn(false, false);
        mDev->getMessageDispatcher()->turnCurrentReaderOn(true, false);
        mDev->getMessageDispatcher()->turnVoltageStimulusOn(true, false);
        mDev->getMessageDispatcher()->enableVcCompensations(true);

        mDev->getMessageDispatcher()->setSourceForVoltageChannel(0, false);
        mDev->getMessageDispatcher()->setSourceForCurrentChannel(0, false);

        mDev->getMessageDispatcher()->setDebugBit(0, 7, false);
        this->onVcCurrentRangeSelected(mDev->getVcCurrentRangeIdx());
        this->onVcVoltageRangeSelected(mDev->getVcVoltageRangeIdx());

    } else {
        mDev->getMessageDispatcher()->enableVcCompensations(false);
        mDev->getMessageDispatcher()->turnVoltageStimulusOn(false, false);
        mDev->getMessageDispatcher()->turnCurrentReaderOn(false, false);
        mDev->getMessageDispatcher()->turnVoltageReaderOn(true, false);
        mDev->getMessageDispatcher()->turnCurrentStimulusOn(true, false);
        mDev->getMessageDispatcher()->enableCcCompensations(true);

        mDev->getMessageDispatcher()->setSourceForVoltageChannel(1, false);
        mDev->getMessageDispatcher()->setSourceForCurrentChannel(1, false);

        mDev->getMessageDispatcher()->setDebugBit(0, 7, true);
        this->onCcCurrentRangeSelected(mDev->getCcCurrentRangeIdx());
        this->onCcVoltageRangeSelected(mDev->getCcVoltageRangeIdx());
    }

    emit sigClampingModalitySelected(selectedClampingModalityIndex);
}
