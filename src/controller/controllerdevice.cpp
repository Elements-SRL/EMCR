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
    std::vector <RangedMeasurement_t> ranges;
    uint16_t notUsedDefaultVcCurrRangeIdx;
    mDev->getVcCurrentRangesFeatures(ranges, notUsedDefaultVcCurrRangeIdx);

    this->mDev->setVcCurrentRange(ranges[selectedVcCurrentRangeIndex]);
    this->mDev->getMessageDispatcher()->setVCCurrentRange(selectedVcCurrentRangeIndex, true);

    emit sigVcCurrentRangeSelected(selectedVcCurrentRangeIndex);
}

// DAC Voltage Range in VC might be set by protocol
void ControllerDevice::onVcVoltageRangeSelected(uint16_t selectedVcVoltageRangeIndex){
    std::vector <RangedMeasurement_t> ranges;
    mDev->getVcVoltageRangesFeatures(ranges);

    this->mDev->setVcVoltageRange(ranges[selectedVcVoltageRangeIndex]);
    this->mDev->getMessageDispatcher()->setVCVoltageRange(selectedVcVoltageRangeIndex, true);

    emit sigVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
}

// DAC Current Range in CC might be set by protocol
void ControllerDevice::onCcCurrentRangeSelected(uint16_t selectedCcCurrentRangeIndex){
    std::vector <RangedMeasurement_t> ranges;
    mDev->getCcCurrentRangesFeatures(ranges);

    this->mDev->setCcCurrentRange(ranges[selectedCcCurrentRangeIndex]);
    this->mDev->getMessageDispatcher()->setCCCurrentRange(selectedCcCurrentRangeIndex, true);

    emit sigCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
}

// ADC Voltage Range in CC
void ControllerDevice::onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex){
    std::vector <RangedMeasurement_t> ranges;
    mDev->getCcVoltageRangesFeatures(ranges);

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

    this->mDev->setVcVoltageFilter(filters[selectedVcVoltageFilterIndex]);
    this->mDev->getMessageDispatcher()->setVoltageStimulusLpf(selectedVcVoltageFilterIndex, true);

    emit sigVcVoltageFilterSelected(selectedVcVoltageFilterIndex);
}

// DAC Current Filter in CC
void ControllerDevice::onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex){
    std::vector <Measurement_t> filters;
    mDev->getCurrentStimulusLpfsFeatures(filters);

    this->mDev->setCcCurrentFilter(filters[selectedCcCurrentFilterIndex]);
    this->mDev->getMessageDispatcher()->setCurrentStimulusLpf(selectedCcCurrentFilterIndex, true);

    emit sigCcCurrentFilterSelected(selectedCcCurrentFilterIndex);
}

// Sampling rate
void ControllerDevice::onSamplingRateSelected(uint16_t selectedSamplingRateIndex){
    std::vector <Measurement_t> samplingRates;
    mDev->getSamplingRatesFeatures(samplingRates);

    this->mDev->setSamplingRate(samplingRates[selectedSamplingRateIndex]);
    this->mDev->getMessageDispatcher()->setSamplingRate(selectedSamplingRateIndex, true);

    emit sigSamplingRateSelected(selectedSamplingRateIndex);
}
// ADC Voltage Filter in CC set by Sampling rate
