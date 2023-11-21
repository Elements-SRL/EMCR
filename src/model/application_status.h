#ifndef APPLICATION_STATUS_H
#define APPLICATION_STATUS_H

#include <channel_mapping.h>
#include "messagedispatcher.h"
#include "channel_and_name.h"

class ApplicationStatus {
//    TODO maybe use a status and act as a state machin, if the configuration is not the default one do we need another way to get channels num and board num?
private:
    std::vector<YAML::ChannelMapping> channelMappings;
    int voltageChannelsNum;
    int currentChannelsNum;
    int boardsNum;
    std::vector<ChannelAndName> channelsAndNames;
    MessageDispatcher * msgDisp;

public:
    ApplicationStatus(MessageDispatcher * msgDisp, std::string filepath = "");
    ~ApplicationStatus();
    void loadChannelMappingFromYaml(std::string pathTofile);
    void printEverything();
    int getVoltageChannelsNum();
    int getCurrentChannelsNum();
    int getBoardsNum();
    std::vector <ChannelModel *> getChannels();
    std::vector <bool> getSelectedChannels();
    std::vector <uint16_t> getSelectedChannelsIndexes();
    std::vector <ChannelAndName> getChannelsAndNames();
};


#endif // APPLICATION_STATUS_H
