#include "application_status.h"
#include <filesystem>
#include <iostream>

ApplicationStatus::ApplicationStatus(MessageDispatcher * msgDisp, std::string filepath):
    msgDisp(msgDisp) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    for (uint16_t i = 0; i < currentChannelsNum; i++) {
        channelStates.push_back(new ChannelState(i));
    }
    if(!filepath.empty() && std::filesystem::exists(filepath)) {
//        if there's a mapping file read from it
        loadChannelMappingFromYaml(filepath);
    } else {
//        else populate it with default values from the commlib
        std::vector <ChannelModel *> channels;
        msgDisp->getChannels(channels);
        for (auto ch: channels){
            auto chIdx = ch->getId();
            mappings.push_back({chIdx, std::to_string(chIdx + 1), NULL, NULL, true});
        }
    }
}

void ApplicationStatus::loadChannelMappingFromYaml(std::string pathTofile) {
    YAML::Node yamlNode = YAML::LoadFile(pathTofile);
    mappings = yamlNode.as<std::vector<YAML::ChannelMapping>>();
}

void ApplicationStatus::setAllChannelsSelected(bool state){
    for (auto &ch: channelStates) {
        ch->setSelected(state);
    }
}

int ApplicationStatus::getVoltageChannelsNum(){
    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    return voltageChannelsNum;
};

int ApplicationStatus::getCurrentChannelsNum(){
    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    return currentChannelsNum;
};

int ApplicationStatus::getBoardsNum(){
    msgDisp->getBoardsNumberFeatures(boardsNum);
    return boardsNum;
};

int ApplicationStatus::getTemperatureChannelsNum(){
    std::vector <std::string> names;
    std::vector <RangedMeasurement_t> ranges;

    if (msgDisp->getTemperatureChannelsFeatures(names, ranges) != Success) {
        return 0;
    }
    return names.size();
};

std::vector <ChannelModel *> ApplicationStatus::getChannels(){
    std::vector <ChannelModel *> channels;
    msgDisp->getChannels(channels);
    return channels;
}

std::vector <bool> ApplicationStatus::getSelectedChannels(){
    std::vector <bool> selectedChannels;
    for (auto ch: channelStates) {
        selectedChannels.push_back(ch->isSelected());
    }
    return selectedChannels;

};

std::vector <uint16_t> ApplicationStatus::getSelectedChannelsIndexes(){
    std::vector <uint16_t> selectedChannels;
    for (auto ch: channelStates) {
        if (ch->isSelected()) {
            selectedChannels.push_back(ch->getIndex());
        }
    }
    return selectedChannels;
};

std::vector <uint16_t> ApplicationStatus::getExpandedChannelsIndexes() {
    auto expChannels = this->getExpandedTraces();
    std::vector<uint16_t> keys;
    for (const auto& pair : expChannels) {
        if (pair.second) {
            keys.push_back(pair.first);
        }
    }
    return keys;
};

std::vector <uint16_t> ApplicationStatus::getStimActiveChannelsIndexes() {
    std::vector <uint16_t> stimActiveChannels;
    for (const auto& ch : getChannels()) {
        if (ch->isStimActive()) {
            stimActiveChannels.push_back(ch->getId());
        }
    }
    return stimActiveChannels;
};

std::vector <uint16_t> ApplicationStatus::getExpandedAndStimActiveChannelsIndexes() {
    auto expandedChannelsIndexes = this->getExpandedChannelsIndexes();
    std::vector <uint16_t> stimActiveChannels;
    for (const auto& ch : getChannels()) {
        if (ch->isStimActive() && std::find(expandedChannelsIndexes.begin(), expandedChannelsIndexes.end(), ch->getId()) != expandedChannelsIndexes.end()) {
            stimActiveChannels.push_back(ch->getId());
        }
    }
    return stimActiveChannels;
};

std::vector <uint16_t> ApplicationStatus::getOffsetRecalibratingChannelsIndexes() {
    std::vector <uint16_t> recalibratingChannels;
    for (const auto& ch : getChannels()) {
        if (ch->isRecalibratingReadoutOffset()) {
            recalibratingChannels.push_back(ch->getId());
        }
    }
    return recalibratingChannels;
};

std::vector <uint16_t> ApplicationStatus::getLiquidJunctionCompensatingChannelsIndexes() {
    std::vector <uint16_t> compensatingChannels;
    for (const auto& ch : getChannels()) {
        if (ch->isCompensatingLiquidJunction()) {
            compensatingChannels.push_back(ch->getId());
        }
    }
    return compensatingChannels;
};

std::vector <YAML::ChannelMapping> ApplicationStatus::getMappings(){
    return mappings;
};

std::vector <int> ApplicationStatus::getVisibleChannelsOnBoard(int boardIdx){
    std::vector <ChannelModel *> channels;
    msgDisp->getChannelsOnBoard(boardIdx, channels);
    std::vector<int> channelIndexes;
    for (auto &ch: channels){
        channelIndexes.push_back(ch->getId());
    }
    return filterVisibleChannels(channelIndexes);
}

std::vector <int> ApplicationStatus::getVisibleChannelsOnRow(int rowIdx){
    std::vector <ChannelModel *> channels;
    msgDisp->getChannelsOnRow(rowIdx, channels);
    std::vector<int> channelIndexes;
    for (auto &ch: channels){
        channelIndexes.push_back(ch->getId());
    }
    return filterVisibleChannels(channelIndexes);
}

std::set <int> ApplicationStatus::getVisibleBoards(){
    std::set <int> visibleBoards;
    auto chPerBoard = currentChannelsNum/boardsNum;
    for (auto &m: getMappings()){
        if (m.visible){
            visibleBoards.insert(round(m.index/chPerBoard));
        }
    }
    return visibleBoards;
}

std::vector <int> ApplicationStatus::getVisibleChannels(){
    std::vector <int> visibleChannels;
    for (auto &m: getMappings()){
        if (m.visible){
            visibleChannels.push_back(m.index);
        }
    }
    return visibleChannels;
}

std::vector<int> ApplicationStatus::filterVisibleChannels(std::vector<int> channelInexes){
    std::vector<int> results;
    for(auto &i: channelInexes){
        if(mappings[i].visible){
            results.push_back(i);
        }
    }
    return results;
}

void ApplicationStatus::setSelectedChannels(std::map<int, bool> channelAndStatus){
    for(auto ch: channelAndStatus){
        channelStates[ch.first]->setSelected(ch.second);
    }
}

MessageDispatcher * ApplicationStatus::getMessageDispatcher(){
    return msgDisp;
}

std::vector<std::string> ApplicationStatus::getNames(){
    std::vector<std::string> names;
    for(auto &m: getMappings()){
        names.push_back(m.name);
    }
    return names;
}

Measurement ApplicationStatus::getSamplingRate() {
    Measurement_t sr;
    msgDisp->getSamplingRate(sr);
    return sr;
}

RangedMeasurement_t ApplicationStatus::getVcVoltageRange() {
    RangedMeasurement_t vr;
    msgDisp->getVCVoltageRange(vr);
    return vr;
}

std::vector <RangedMeasurement_t> ApplicationStatus::getVcCurrentRange() {
    std::vector <RangedMeasurement_t> cr;
    msgDisp->getVCCurrentRange(cr);
    return cr;
}

std::vector <RangedMeasurement_t> ApplicationStatus::getCcVoltageRange() {
    std::vector <RangedMeasurement_t> vr;
    msgDisp->getCCVoltageRange(vr);
    return vr;
}

RangedMeasurement_t ApplicationStatus::getCcCurrentRange() {
    RangedMeasurement_t cr;
    msgDisp->getCCCurrentRange(cr);
    return cr;
}

std::vector <RangedMeasurement_t> ApplicationStatus::getVoltageRanges() {
    std::vector <RangedMeasurement_t> vr;
    msgDisp->getVoltageRange(vr);
    return vr;
}

RangedMeasurement_t ApplicationStatus::getMaxVoltageRange() {
    auto vr = this->getVoltageRanges();
    return *std::max_element(vr.begin(), vr.end());
}

std::vector <RangedMeasurement_t> ApplicationStatus::getCurrentRanges() {
    std::vector <RangedMeasurement_t> cr;
    msgDisp->getCurrentRange(cr);
    return cr;
}

RangedMeasurement_t ApplicationStatus::getMaxCurrentRange() {
    auto cr = this->getCurrentRanges();
    return *std::max_element(cr.begin(), cr.end());
}

std::string ApplicationStatus::getSerialNumber() {
    std::string serialNumber;
    msgDisp->getSerialNumber(serialNumber);
    return serialNumber;
}

std::string ApplicationStatus::getDeviceInfoString() {
    unsigned int version, subversion, fw;
    auto serial = getSerialNumber();
    msgDisp->getDeviceInfo(serial, version, subversion, fw);
    return "version: " + std::to_string(version) + ", subversion: " + std::to_string(subversion) + ", firmware: " + std::to_string(fw);
}

ClampingModality_t ApplicationStatus::getClampingModality() {
    ClampingModality_t c;
    msgDisp->getClampingModality(c);
    return c;
}

std::map <uint16_t, bool> ApplicationStatus::getExpandedTraces() {
    std::map<uint16_t, bool> expandedTraces;
    for (auto chs: channelStates) {
        expandedTraces[chs->getIndex()] = chs->isExpanded();
    }
    return expandedTraces;
}

void ApplicationStatus::setExpandedTraces(std::map <uint16_t, bool> other){
    for(auto ch: other){
        channelStates[ch.first]->setExpanded(ch.second);
    }
}

std::vector<uint16_t> ApplicationStatus::getDetailedPlotIndexes() {
    std::vector<uint16_t> detailed;
    for (auto &p: this->channelStates) {
        if (p->isDetailed()) {
            detailed.push_back(p->getIndex());
        }
    }
    return detailed;
}

void ApplicationStatus::setDetailedPlots(std::map <uint16_t, bool> other){
    for (auto o : other) {
        channelStates[o.first]->setDetailed(o.second);
    }
}

void ApplicationStatus::clearPlotDetails(){
    for (auto &p: this->channelStates) {
        p->setDetailed(false);
    }
}

void ApplicationStatus::setPlotDetailAuto(bool flag) {
    plotDetailAuto = flag;
}

bool ApplicationStatus::isPlotDetailAuto() {
    return plotDetailAuto;
}

std::string ApplicationStatus::getClampingModalityString() {
    auto cm = getClampingModality();
    std::string cms;
    switch (cm) {
    case ClampingModality_t::VOLTAGE_CLAMP:
        cms = "Voltage clamp";
        break;
    case ClampingModality_t::CURRENT_CLAMP:
        cms = "Current clamp";
        break;
    case ClampingModality_t::ZERO_CURRENT_CLAMP:
        cms = "Zero current clamp";
        break;
    case ClampingModality_t::DYNAMIC_CLAMP:
        cms = "Dynamic clamp";
        break;
    }
    return cms;
}

bool ApplicationStatus::isEpisodic() {
    return (msgDisp->isEpisodic() == Success);
}

void ApplicationStatus::setChannelsAuto(bool flag) {
    this->channelsAuto = flag;
}

bool ApplicationStatus::isChannelsAuto() {
    return channelsAuto;
}

void ApplicationStatus::setStimulusAuto(bool flag) {
    this->stimulusAuto = flag;
}

bool ApplicationStatus::isStimulusAuto() {
    return stimulusAuto;
}

void ApplicationStatus::setExpandAuto(bool flag) {
    this->expandAuto = flag;
}

bool ApplicationStatus::isExpandAuto() {
    return expandAuto;
}

void ApplicationStatus::setChannelSelected(uint16_t idx, bool flag) {
    channelStates[idx]->setSelected(flag);
}

RangedMeasurement_t ApplicationStatus::getCurretRange() {
    RangedMeasurement r;
    msgDisp->getCurrentRange(r);
    return r;
}

RangedMeasurement_t ApplicationStatus::getVoltageRange() {
    RangedMeasurement r;
    msgDisp->getVoltageRange(r);
    return r;
}
