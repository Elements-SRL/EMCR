#include "modelboard.h"

ModelBoard::ModelBoard()
{

}

uint16_t ModelBoard::getId(){
    return this->id;
}

vector<ModelChannel> ModelBoard::getChannelsOnBoard(){
    return this->channelsOnBoard;
}

Measurement_t ModelBoard::getGateVoltage(){
    return this->gateVoltage;
}

Measurement_t ModelBoard::getSourceVoltage(){
    return this->sourceVoltage;
}


void ModelBoard::setId(uint16_t id){
    this->id = id;
}

void ModelBoard::setChannelsOnBoard (vector<ModelChannel> channelsOnBoard){
    this->channelsOnBoard = channelsOnBoard;
}

void ModelBoard::setGateVoltage(Measurement_t gateVoltage){
    this->gateVoltage = gateVoltage;
}

void ModelBoard::setSourceVoltage(Measurement_t sourceVoltage){
    this->sourceVoltage = sourceVoltage;
}


void ModelBoard::fillChannelList(uint16_t numChannelsOnBoard){
    this->channelsOnBoard.resize(numChannelsOnBoard);
    for(uint16_t i = 0; i< numChannelsOnBoard; i++ ){
        ModelChannel channel;
        channel.setId(i);
        this->channelsOnBoard.push_back(channel);
    }
}
