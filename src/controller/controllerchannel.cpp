#include "controllerchannel.h"

ControllerChannel::ControllerChannel()
{

}

void ControllerChannel::setModelDevice(ModelDevice * mDev){
    this->mDev = mDev;
}

void ControllerChannel::onSingleChannelsClicked(uint16_t changedChannelIndexes, bool newChannelState){
    this->mDev->getChannels()[changedChannelIndexes]->setSelected(newChannelState);
    emit sigUpdateChannelControlDockWidget();
}

void ControllerChannel::onOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState){
    ModelBoard* boardToUpdate = this->mDev->getBoards()[changedBoardIndex];
    uint16_t numOfChannelsToUpadate = boardToUpdate->getChannelsOnBoard().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        boardToUpdate->getChannelsOnBoard()[i]->setSelected(newChannelState);
    }
    emit sigUpdateChannelControlDockWidget();
}

void ControllerChannel::onOneRowClicked(uint16_t changedRowIndex, bool newChannelState){
    uint16_t numOfBoardsToUpadate = this->mDev->getBoards().size();
    for(uint16_t i = 0; i < numOfBoardsToUpadate; i++){
        this->mDev->getBoards()[i]->getChannelsOnBoard()[changedRowIndex]->setSelected(newChannelState);
    }
    emit sigUpdateChannelControlDockWidget();
}

void ControllerChannel::onAllChannelsClicked(bool newChannelState){
    uint16_t numOfChannelsToUpadate = this->mDev->getChannels().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        this->mDev->getChannels()[i]->setSelected(newChannelState);
    }
    emit sigUpdateChannelControlDockWidget();
}

void ControllerChannel::onApplyTurnChannelOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues, bool applyFlag){
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, applyFlag);
}

void ControllerChannel::onApplyTurnDocOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues, bool applyFlag){
    this->mDev->getMessageDispatcher()->digitalOffsetCompensation(channelIndexes, onValues, applyFlag);
}

void ControllerChannel::onApplyVoltageHoldValues(vector<uint16_t> channelIndexes, vector<Measurement_t> voltages, bool applyFlag){
    this->mDev->getMessageDispatcher()->setVoltageHoldTuner(channelIndexes, voltages, applyFlag);
}
