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
        channelsAndNames.clear();
        std::vector <ChannelModel *> channels;
        msgDisp->getChannels(channels);
        for (auto ch: channels){
            auto chIdx = ch->getId();
            channelsAndNames.push_back({chIdx, std::to_string(chIdx)});
        }
    }
}

void ApplicationStatus::loadChannelMappingFromYaml(std::string pathTofile) {
    YAML::Node yamlNode = YAML::LoadFile(pathTofile);
    if (yamlNode.IsSequence()){
        std::cout << yamlNode << std::endl;
    }
    channelMappings = yamlNode.as<std::vector<YAML::ChannelMapping>>();
    channelsAndNames.clear();
    for(auto &cm: channelMappings){
        channelsAndNames.push_back({cm.originalChannelIndex, cm.newChannelName});
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
