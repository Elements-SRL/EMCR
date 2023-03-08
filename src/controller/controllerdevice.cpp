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
    this->mDev->getMessageDispatcher()->setVCCurrentRange(selectedVcCurrentRangeIndex, true);
    qDebug() <<"VcCurrentRangeSelected idx "<< selectedVcCurrentRangeIndex << "";
}

// DAC Voltage Range in VC set by protocol

// DAC Current Range in CC set by protocol

// ADC Voltage Range in CC
void ControllerDevice::onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex){
    this->mDev->getMessageDispatcher()->setCCVoltageRange(selectedCcVoltageRangeIndex, true);
}


// Slots (actionPerformed) for current and voltage filters
// ADC Current Filter in VC set by Sampling Rate

// DAC Voltage Filter in VC
void ControllerDevice::onVcVoltageFilterSelected(uint16_t selectedVcVoltageFilterIndex){
    this->mDev->getMessageDispatcher()->setVoltageStimulusLpf(selectedVcVoltageFilterIndex, true);
}

// DAC Current Filter in CC
void ControllerDevice::onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex){
    this->mDev->getMessageDispatcher()->setCurrentStimulusLpf(selectedCcCurrentFilterIndex, true);
}

// ADC Voltage Filter in CC set by Sampling rate
