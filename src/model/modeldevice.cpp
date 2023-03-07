#include "modeldevice.h"

ModelDevice::ModelDevice() {

}

QString ModelDevice::getSerialNumber() {
    return this->serialNumber;
}

bool ModelDevice::isConnected() {
    return this->connected;
}

void ModelDevice::setSerialNumber(QString serial) {
    this->serialNumber = serial;
}

void ModelDevice::setConnected(bool flag) {
    this->connected = flag;
}

vector<ModelBoard> ModelDevice::getBoards(){
    return this->myBoards;
}

vector<ModelChannel> ModelDevice::getChannels(){
    return this->myChannels;
}

Measurement_t ModelDevice::getSamplingRate(){
    return this->samplingRate;
}

RangedMeasurement_t ModelDevice::getVcCurrentRange(){
    return this->vCcurrentRange;
}

RangedMeasurement_t ModelDevice::getVcVoltageRange(){
    return this->vCvoltageRange;
}

RangedMeasurement_t ModelDevice::getCcCurrentRange(){
    return this->cCcurrentRange;
}

RangedMeasurement_t ModelDevice::getCcVoltageRange(){
    return this->cCvoltageRange;
}

Measurement_t ModelDevice::getVcCurrentFilter(){
    return this->vCcurrentFilter;
}

Measurement_t ModelDevice::getVcVoltageFilter(){
    return this->vCvoltageFilter;
}

Measurement_t ModelDevice::getCcCurrentFilter(){
    return this->cCcurrentFilter
}

Measurement_t ModelDevice::getCcVoltageFilter(){
    return this->cCvoltageFilter;
}



void ModelDevice::setBoards(vector<ModelBoard> boards){
    this->myBoards = boards;
}

void ModelDevice::setChannels(vector<ModelChannel> channels){
    this->myChannels = channels;
}

void ModelDevice::setSamplingRate(Measurement_t samplingRate){
    this->samplingRate = samplingRate;
}

void ModelDevice::setVcCurrentRange(RangedMeasurement_t vCcurrentRange){
    this->vCcurrentRange = vCcurrentRange;
}

void ModelDevice::setVcVoltageRange(RangedMeasurement_t vCvoltageRange){
    this->vCvoltageRange = vCvoltageRange;
}

void ModelDevice::setCcCurrentRange(RangedMeasurement_t cCcurrentRange){
    this->cCcurrentRange = cCcurrentRange;
}

void ModelDevice::setCcVoltageRange(RangedMeasurement_t cCvoltageRange){
    this->cCvoltageRange = cCvoltageRange;
}

void ModelDevice::setVcCurrentFilter(Measurement_t vCcurrentFilter){
    this->vCcurrentFilter = vCcurrentFilter;
}

void ModelDevice::setVcVoltageFilter(Measurement_t vCvoltageFilter){
    this->vCvoltageFilter = vCvoltageFilter;
}

void ModelDevice::setCcCurrentFilter(Measurement_t cCcurrentFilter){
    this->cCcurrentFilter = cCcurrentFilter;
}

void ModelDevice::setCcVoltageFilter(Measurement_t cCvoltageFilter){
    this->cCvoltageFilter = cCvoltageFilter;
}


void ModelDevice::fillBoardList(uint16_t numOfBoards, uint16_t numOfChannelsOnBoard){
    this->myBoards.resize(numOfBoards);
    for(uint16_t i = 0; i< numOfBoards; i++ ){
        ModelBoard board;
        board.setId(i);
        board.fillChannelList(numOfChannelsOnBoard);
        this->myBoards.push_back(board);
    }
}

void ModelDevice::fillChannelList(uint16_t numOfChannels){
    this->myChannels.resize(numOfChannels);
    for(uint16_t i = 0; i< numOfChannels; i++ ){
        ModelChannel channel;
        channel.setId(i);
        this->myChannels.push_back(channel);
    }
}
