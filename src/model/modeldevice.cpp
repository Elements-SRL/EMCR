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

int ModelDevice::getSamplingRateIdx(){
    return this->samplingRateIdx;
}

int ModelDevice::getVcCurrentRangeIdx(){
    return this->vcCurrentRangeIdx;
}

int ModelDevice::getVcVoltageRangeIdx(){
    return this->vcVoltageRangeIdx;
}

int ModelDevice::getCcCurrentRangeIdx(){
    return this->ccCurrentRangeIdx;
}

int ModelDevice::getCcVoltageRangeIdx(){
    return this->ccVoltageRangeIdx;
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

int ModelDevice::getVcCurrentFilterIdx(){
    return this->vcCurrentFilterIdx;
}

int ModelDevice::getVcVoltageFilterIdx(){
    return this->vcVoltageFilterIdx;
}

int ModelDevice::getCcCurrentFilterIdx(){
    return this->ccCurrentFilterIdx;
}

int ModelDevice::getCcVoltageFilterIdx(){
    return this->ccVoltageFilterIdx;
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

QVector <bool> ModelDevice::getSelectedChannelsIdxs() {
    QVector <bool> ret(this->myChannels.size());
    for (int idx = 0; idx < this->myChannels.size(); idx++) {
        ret[idx] = this->myChannels[idx]->isSelected();
    }
    return ret;
}

int ModelDevice::getOngoingClampingModality(){
    return ongoingClampingModality;
}

int ModelDevice::getOngoingClampingModalityIdx(){
    return ongoingClampingModalityIdx;
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

void ModelDevice::setSamplingRate(int idx){
    this->samplingRateIdx = idx;
}

void ModelDevice::setVcCurrentRange(int idx){
    this->vcCurrentRangeIdx = idx;
}

void ModelDevice::setVcVoltageRange(int idx){
    this->vcVoltageRangeIdx = idx;
}

void ModelDevice::setCcCurrentRange(int idx){
    this->ccCurrentRangeIdx = idx;
}

void ModelDevice::setCcVoltageRange(int idx){
    this->ccVoltageRangeIdx = idx;
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

void ModelDevice::setVcCurrentFilter(int idx){
    this->vcCurrentFilterIdx = idx;
}

void ModelDevice::setVcVoltageFilter(int idx){
    this->vcVoltageFilterIdx = idx;
}

void ModelDevice::setCcCurrentFilter(int idx){
    this->ccCurrentFilterIdx = idx;
}

void ModelDevice::setCcVoltageFilter(int idx){
    this->ccVoltageFilterIdx = idx;
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

void ModelDevice::setOngoingClampingModality(int mode) {
    ongoingClampingModality = mode;
}

void ModelDevice::setOngoingClampingModalityIdx(int idx) {
    ongoingClampingModalityIdx = idx;
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
ErrorCodes_t ModelDevice::getVoltageHoldTunerFeatures(std::vector <RangedMeasurement_t> &voltageHoldTunerFeatures){
    return this->messageDispatcher->getVoltageHoldTunerFeatures(voltageHoldTunerFeatures);
}

ErrorCodes_t ModelDevice::getCalibVcCurrentGainFeatures(RangedMeasurement_t &calibVcCurrentGainFeatures){
    return this->messageDispatcher->getCalibVcCurrentGainFeatures(calibVcCurrentGainFeatures);
}

ErrorCodes_t ModelDevice::getCalibVcCurrentOffsetFeatures(vector <RangedMeasurement_t> &calibVcCurrentOffsetFeatures){
    return this->messageDispatcher->getCalibVcCurrentOffsetFeatures(calibVcCurrentOffsetFeatures);
}

ErrorCodes_t ModelDevice::getCalibCcVoltageGainFeatures(RangedMeasurement_t &calibCcVoltageGainFeatures){
    return this->messageDispatcher->getCalibCcVoltageGainFeatures(calibCcVoltageGainFeatures);
}

ErrorCodes_t ModelDevice::getCalibCcVoltageOffsetFeatures(vector <RangedMeasurement_t> &calibCcVoltageOffsetFeatures){
    return this->messageDispatcher->getCalibCcVoltageOffsetFeatures(calibCcVoltageOffsetFeatures);
}

ErrorCodes_t ModelDevice::getGateVoltagesTunerFeatures(RangedMeasurement_t &gateVoltagesTunerFeatures){
    return this->messageDispatcher->getGateVoltagesTunerFeatures(gateVoltagesTunerFeatures);
}

ErrorCodes_t ModelDevice::getSourceVoltagesTunerFeatures(RangedMeasurement_t &sourceVoltagesTunerFeatures){
    return this->messageDispatcher->getSourceVoltagesTunerFeatures(sourceVoltagesTunerFeatures);
}

ErrorCodes_t ModelDevice::getChannelsNumberFeatures(int &voltageChannelsNum, int &currentChannelsNum) {
    uint16_t vNum;
    uint16_t cNum;
    ErrorCodes_t ret = this->messageDispatcher->getChannelNumberFeatures(vNum, cNum);
    if (ret == Success) {
        voltageChannelsNum = (int)vNum;
        currentChannelsNum = (int)cNum;
    }
    return ret;
}

ErrorCodes_t ModelDevice::getBoardsNumberFeatures(int &boardNum) {
    uint16_t bNum;
    ErrorCodes_t ret = this->messageDispatcher->getBoardsNumberFeatures(bNum);
    if (ret == Success) {
        boardNum = (int)bNum;
    }
    return ret;
}

ErrorCodes_t ModelDevice::getClampingModalitiesFeatures(vector<int> &clampingModalitiesFeatures){
    vector<uint16_t> tempVector;
    ErrorCodes_t ret = this->messageDispatcher->getClampingModalitiesFeatures(tempVector);
    if (ret == Success) {
        clampingModalitiesFeatures.resize(tempVector.size());
        for(int i = 0; i< tempVector.size(); i++){
            clampingModalitiesFeatures[i] = (int)tempVector[i];
        }
    }
    return ret;
}

ErrorCodes_t ModelDevice::getVcCurrentRangesFeatures(vector <RangedMeasurement_t> &vcCurrentRangesFeatures, uint16_t &defaultVcCurrRangeIdx) {
    return this->messageDispatcher->getVCCurrentRanges(vcCurrentRangesFeatures, defaultVcCurrRangeIdx);
}

ErrorCodes_t ModelDevice::getVcVoltageRangesFeatures(vector <RangedMeasurement_t> &vcVoltageRangesFeatures){
    return this->messageDispatcher->getVCVoltageRanges(vcVoltageRangesFeatures);
}

ErrorCodes_t ModelDevice::getCcCurrentRangesFeatures(vector <RangedMeasurement_t> &ccCurrentRangesFeatures){
    return this->messageDispatcher->getCCCurrentRanges(ccCurrentRangesFeatures);
}

ErrorCodes_t ModelDevice::getCcVoltageRangesFeatures(vector <RangedMeasurement_t> &ccVoltageRangesFeatures){
    return this->messageDispatcher->getCCVoltageRanges(ccVoltageRangesFeatures);
}

ErrorCodes_t ModelDevice::getSamplingRatesFeatures(vector <Measurement_t> &samplingRatesFeatures) {
    return this->messageDispatcher->getSamplingRatesFeatures(samplingRatesFeatures);
}

ErrorCodes_t ModelDevice::getVoltageStimulusLpfsFeatures(vector <Measurement_t> &voltageFilterOptions){
    return this->messageDispatcher->getVoltageStimulusLpfs(voltageFilterOptions);
}

ErrorCodes_t ModelDevice::getCurrentStimulusLpfsFeatures(vector <Measurement_t> &currentFilterOptions){
    return this->messageDispatcher->getCurrentStimulusLpfs(currentFilterOptions);
}

//ErrorCodes_t ModelDevice::getCalibVcVoltStepFeatures(vector <Measurement_t> &calibVcVoltStepsFeatures){
//    return this->messageDispatcher->getVcCalibVoltStepsFeatures(calibVcVoltStepsFeatures);
//}

//ErrorCodes_t ModelDevice::getCalibVcResFeatures(vector <Measurement_t> &calibVcResFeatures){
//    return this->messageDispatcher->getVcCalibResFeatures(calibVcResFeatures);
//}

ErrorCodes_t ModelDevice::getCalibDataFeatures(CalibrationData_t &calibData){
    return this->messageDispatcher->getCalibData(calibData);
}


ErrorCodes_t ModelDevice::getCompFeatures(uint16_t paramToExtractFeatures, vector<RangedMeasurement_t> &compensationFeatures, double &defaultParamValue){
    return this->messageDispatcher->getCompFeatures(paramToExtractFeatures, compensationFeatures, defaultParamValue);
}

ErrorCodes_t ModelDevice::getCompOptionsFeatures(MessageDispatcher::CompensationTypes type ,std::vector <std::string> &compOptionsArray){
    return this->messageDispatcher->getCompOptionsFeatures(type, compOptionsArray);
}

//---------------------------------------------/
