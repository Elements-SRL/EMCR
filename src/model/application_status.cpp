#include "application_status.h"
#include <iostream>
#include <filesystem>

ApplicationStatus::ApplicationStatus(MessageDispatcher * msgDisp, std::string filepath):
    msgDisp(msgDisp) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
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

ApplicationStatus:: ~ApplicationStatus() {
    if (msgDisp != nullptr) {
        delete msgDisp;
        msgDisp = nullptr;
    }
}

void ApplicationStatus::loadChannelMappingFromYaml(std::string pathTofile) {
    YAML::Node yamlNode = YAML::LoadFile(pathTofile);
    mappings = yamlNode.as<std::vector<YAML::ChannelMapping>>();
}

void ApplicationStatus::setAllChannelsSelected(bool state){
    msgDisp->setAllChannelsSelected(state);
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

std::vector <ChannelModel *> ApplicationStatus::getChannels(){
    std::vector <ChannelModel *> channels;
    msgDisp->getChannels(channels);
    return channels;
}

std::vector <bool> ApplicationStatus::getSelectedChannels(){
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    return selectedChannels;

};

std::vector <uint16_t> ApplicationStatus::getSelectedChannelsIndexes(){
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    return selectedChannels;
};

std::vector <uint16_t> ApplicationStatus::getExpandedChannelsIndexes() {
    std::vector <uint16_t> expandedChannels;
    for (const auto& ch : getChannels()) {
        if (ch->isExpanded()) {
            expandedChannels.push_back(ch->getId());
        }
    }
    return expandedChannels;
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
        msgDisp->setChannelSelected(ch.first, ch.second);
    }
}

MessageDispatcher * ApplicationStatus::getMessageDispatcher(){
    return msgDisp;
}

std::vector<std::string> ApplicationStatus::getNames(){
    std::vector<std::string> names;
    for(auto m: getMappings()){
        names.push_back(m.name);
    }
    return names;
}

Measurement ApplicationStatus::getSamplingRate() {
    Measurement_t sr;
    auto err = msgDisp->getSamplingRate(sr);
    return sr;
}