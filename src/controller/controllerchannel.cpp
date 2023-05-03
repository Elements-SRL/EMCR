#include "controllerchannel.h"

ControllerChannel::ControllerChannel(ModelDevice * mDev) :
    mDev(mDev)
{

}

void ControllerChannel::setModelDevice(ModelDevice * mDev){
    this->mDev = mDev;
}

void ControllerChannel::onSingleChannelClicked(uint16_t changedChannelIndexes, bool newChannelState){
    this->mDev->getChannels()[changedChannelIndexes]->setSelected(newChannelState);
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState){
    ModelBoard* boardToUpdate = this->mDev->getBoards()[changedBoardIndex];
    uint16_t numOfChannelsToUpadate = boardToUpdate->getChannelsOnBoard().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        boardToUpdate->getChannelsOnBoard()[i]->setSelected(newChannelState);
    }
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onOneRowClicked(uint16_t changedRowIndex, bool newChannelState){
    uint16_t numOfBoardsToUpadate = this->mDev->getBoards().size();
    for(uint16_t i = 0; i < numOfBoardsToUpadate; i++){
        this->mDev->getBoards()[i]->getChannelsOnBoard()[changedRowIndex]->setSelected(newChannelState);
    }
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onAllChannelsClicked(bool newChannelState){
    uint16_t numOfChannelsToUpadate = this->mDev->getChannels().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        this->mDev->getChannels()[i]->setSelected(newChannelState);
    }
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onApplyTurnChannelOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setOn(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyTurnStimulusOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->enableStimulus(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setInStimActive(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: stimulus on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyTurnDocOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->digitalOffsetCompensation(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setCompensating(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: DOC on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyVoltageHoldValues(vector<uint16_t> channelIndexes, vector<Measurement_t> vHoldValues){
    this->mDev->getMessageDispatcher()->setVoltageHoldTuner(channelIndexes, vHoldValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setVhold(vHoldValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: vHold set:" << vHoldValues[i].value << "\n";
    }
}

void ControllerChannel::onCompensationApplied(vector<uint16_t> channelIndexes, vector<bool> cfastEn, vector<bool> cslowRsEn, vector<bool> rsCpEn, vector<bool> rsPgEn){
    this->mDev->enableCompensation(channelIndexes, MessageDispatcher::CompCfast, cfastEn);
    this->mDev->enableCompensation(channelIndexes, MessageDispatcher::CompCslow, cslowRsEn);
    this->mDev->enableCompensation(channelIndexes, MessageDispatcher::CompRsCorr, rsCpEn);
    this->mDev->enableCompensation(channelIndexes, MessageDispatcher::CompRsPred, rsPgEn);
}


