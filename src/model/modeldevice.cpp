#include "modeldevice.h"

ModelDevice::ModelDevice() {

}

ModelDevice::~ModelDevice() {

}

MessageDispatcher * ModelDevice::getMessageDispatcher() {
    return messageDispatcher;
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

vector<ModelBoard*> ModelDevice::getBoards(){
    return this->myBoards;
}

vector<ModelChannel*> ModelDevice::getChannels(){
    return this->myChannels;
}

Measurement_t ModelDevice::getSamplingRate(){
    return this->samplingRate;
}

RangedMeasurement_t ModelDevice::getVcCurrentRange(){
    return this->vcCurrentRange;
}

RangedMeasurement_t ModelDevice::getVcVoltageRange(){
    return this->vcVoltageRange;
}

RangedMeasurement_t ModelDevice::getCcCurrentRange(){
    return this->ccCurrentRange;
}

RangedMeasurement_t ModelDevice::getCcVoltageRange(){
    return this->ccVoltageRange;
}

Measurement_t ModelDevice::getVcCurrentFilter(){
    return this->vcCurrentFilter;
}

Measurement_t ModelDevice::getVcVoltageFilter(){
    return this->vcVoltageFilter;
}

Measurement_t ModelDevice::getCcCurrentFilter(){
    return this->ccCurrentFilter;
}

Measurement_t ModelDevice::getCcVoltageFilter(){
    return this->ccVoltageFilter;
}


void ModelDevice::setMessageDispatcher(MessageDispatcher * messageDispatcher) {
    this->messageDispatcher = messageDispatcher;
}

void ModelDevice::setBoards(vector<ModelBoard*> boards){
    this->myBoards = boards;
}

void ModelDevice::setChannels(vector<ModelChannel*> channels){
    this->myChannels = channels;
}

void ModelDevice::setSamplingRate(Measurement_t samplingRate){
    this->samplingRate = samplingRate;
}

void ModelDevice::setVcCurrentRange(RangedMeasurement_t vcCurrentRange){
    this->vcCurrentRange = vcCurrentRange;
}

void ModelDevice::setVcVoltageRange(RangedMeasurement_t vcVoltageRange){
    this->vcVoltageRange = vcVoltageRange;
}

void ModelDevice::setCcCurrentRange(RangedMeasurement_t ccCurrentRange){
    this->ccCurrentRange = ccCurrentRange;
}

void ModelDevice::setCcVoltageRange(RangedMeasurement_t ccVoltageRange){
    this->ccVoltageRange = ccVoltageRange;
}

void ModelDevice::setVcCurrentFilter(Measurement_t vcCurrentFilter){
    this->vcCurrentFilter = vcCurrentFilter;
}

void ModelDevice::setVcVoltageFilter(Measurement_t vcVoltageFilter){
    this->vcVoltageFilter = vcVoltageFilter;
}

void ModelDevice::setCcCurrentFilter(Measurement_t ccCurrentFilter){
    this->ccCurrentFilter = ccCurrentFilter;
}

void ModelDevice::setCcVoltageFilter(Measurement_t ccVoltageFilter){
    this->ccVoltageFilter = ccVoltageFilter;
}


void ModelDevice::fillBoardList(uint16_t numOfBoards, uint16_t numOfChannelsOnBoard){
    this->myBoards.resize(numOfBoards);
    for(uint16_t i = 0; i< numOfBoards; i++ ){
        ModelBoard* board = new ModelBoard;
        board->setId(i);
        board->fillChannelList(numOfChannelsOnBoard);
        this->myBoards[i] = board;
    }
}

void ModelDevice::fillChannelList(uint16_t numOfBoards, uint16_t numOfChannelsOnBoard){
    if(this->myBoards.size() == 0){
        this->fillBoardList(numOfBoards, numOfChannelsOnBoard);
    }
    uint16_t newChannelId = 0;
    myChannels.resize(numOfChannelsOnBoard*numOfBoards);
    for(uint16_t i = 0; i< numOfBoards; i++ ){
        for(uint16_t j = 0; j< numOfChannelsOnBoard; j++ ){
            this->myChannels[newChannelId] = this->myBoards[i]->getChannelsOnBoard()[j];
            newChannelId++;
        }
    }
}

void ModelDevice::flushBoardList() {
    int numOfBoards = this->myBoards.size();
    for(uint16_t i = 0; i< numOfBoards; i++ ){
        if (this->myBoards[i] != nullptr) {
            delete this->myBoards[i];
        }
    }
    myBoards.clear();
    myChannels.clear();
}

// wrappers for MessageDispatcher get features
e384cl::ErrorCodes_t ModelDevice::getVoltageHoldTunerFeatures(RangedMeasurement_t &voltageHoldTunerFeatures){
    return this->messageDispatcher->getVoltageHoldTunerFeatures(voltageHoldTunerFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getCalibVcCurrentGainFeatures(RangedMeasurement_t &calibVcCurrentGainFeatures){
    return this->messageDispatcher->getCalibVcCurrentGainFeatures(calibVcCurrentGainFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getCalibVcCurrentOffsetFeatures(vector <RangedMeasurement_t> &calibVcCurrentOffsetFeatures){
    return this->messageDispatcher->getCalibVcCurrentOffsetFeatures(calibVcCurrentOffsetFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getCalibCcVoltageGainFeatures(RangedMeasurement_t &calibCcVoltageGainFeatures){
    return this->messageDispatcher->getCalibCcVoltageGainFeatures(calibCcVoltageGainFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getCalibCcVoltageOffsetFeatures(vector <RangedMeasurement_t> &calibCcVoltageOffsetFeatures){
    return this->messageDispatcher->getCalibCcVoltageOffsetFeatures(calibCcVoltageOffsetFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getGateVoltagesTunerFeatures(RangedMeasurement_t &gateVoltagesTunerFeatures){
    return this->messageDispatcher->getGateVoltagesTunerFeatures(gateVoltagesTunerFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getSourceVoltagesTunerFeatures(RangedMeasurement_t &sourceVoltagesTunerFeatures){
    return this->messageDispatcher->getSourceVoltagesTunerFeatures(sourceVoltagesTunerFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getChannelsNumberFeatures(int &voltageChannelsNum, int &currentChannelsNum) {
    uint16_t vNum;
    uint16_t cNum;
    e384cl::ErrorCodes_t ret = this->messageDispatcher->getChannelNumberFeatures(vNum, cNum);
    if (ret == e384cl::Success) {
        voltageChannelsNum = (int)vNum;
        currentChannelsNum = (int)cNum;
    }
    return ret;
}

e384cl::ErrorCodes_t ModelDevice::getBoardsNumberFeatures(int &boardNum) {
    uint16_t bNum;
    e384cl::ErrorCodes_t ret = this->messageDispatcher->getBoardsNumberFeatures(bNum);
    if (ret == e384cl::Success) {
        boardNum = (int)bNum;
    }
    return ret;
}

e384cl::ErrorCodes_t ModelDevice::getVcCurrentRangesFeatures(vector <e384cl::RangedMeasurement_t> &vcCurrentRangesFeatures) {
    return this->messageDispatcher->getVCCurrentRanges(vcCurrentRangesFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getVcVoltageRangesFeatures(vector <RangedMeasurement_t> &vcVoltageRangesFeatures){
    return this->messageDispatcher->getVCVoltageRanges(vcVoltageRangesFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getCcCurrentRangesFeatures(vector <RangedMeasurement_t> &ccCurrentRangesFeatures){
    return this->messageDispatcher->getCCCurrentRanges(ccCurrentRangesFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getCcVoltageRangesFeatures(vector <RangedMeasurement_t> &ccVoltageRangesFeatures){
    return this->messageDispatcher->getCCVoltageRanges(ccVoltageRangesFeatures);
}

e384cl::ErrorCodes_t ModelDevice::getVoltageStimulusLpfsFeatures(vector <string> &voltageFilterOptions){
    return this->messageDispatcher->getVoltageStimulusLpfs(voltageFilterOptions);
}

e384cl::ErrorCodes_t ModelDevice::getCurrentStimulusLpfsFeatures(vector <string> &currentFilterOptions){
    return this->messageDispatcher->getCurrentStimulusLpfs(currentFilterOptions);
}
//---------------------------------------------/
