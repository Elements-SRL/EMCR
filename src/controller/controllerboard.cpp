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

    qDebug() << "numero di gate voltage cambiati: " << gateVoltageBoardIndexes.size() << "";
    for(int i = 0; i < gateVoltageBoardIndexes.size(); i++){
        qDebug() << "[Gate channel" << gateVoltageBoardIndexes[i] + 1 << "]: " << " now has value " << gateVoltages[i].value;
    }


    qDebug() << "numero di source voltage cambiati: " << sourceVoltageBoardIndexes.size() << "";
    for(int i = 0; i < sourceVoltageBoardIndexes.size(); i++){
        qDebug() << "[Source channel" << sourceVoltageBoardIndexes[i] + 1 << "]: " << " now has value " << sourceVoltages[i].value;
    }

}
