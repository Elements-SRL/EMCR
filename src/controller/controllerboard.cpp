#include "controllerboard.h"

ControllerBoard::ControllerBoard(ModelDevice * mDev) :
    mDev(mDev)
{

}

void ControllerBoard::setModelDevice(ModelDevice * mDev){
    this->mDev = mDev;
}

void ControllerBoard::onGateSourceVoltagesApplied(vector<uint16_t> gateVoltageBoardIndexes, vector<Measurement_t> gateVoltages, vector<uint16_t> sourceVoltageBoardIndexes, vector<Measurement_t> sourceVoltages){
    this->mDev->getMessageDispatcher()->setGateVoltagesTuner(gateVoltageBoardIndexes, gateVoltages, true);
    this->mDev->getMessageDispatcher()->setSourceVoltagesTuner(sourceVoltageBoardIndexes, sourceVoltages, true);
}
