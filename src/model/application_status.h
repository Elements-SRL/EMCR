#ifndef APPLICATION_STATUS_H
#define APPLICATION_STATUS_H

#include <channel_mapping.h>
#include "messagedispatcher.h"
#include "channel_and_name.h"

class ApplicationStatus {
private:
    std::vector<YAML::ChannelMapping> channelMappings;
    int voltageChannelsNum;
    int currentChannelsNum;
    std::vector<ChannelAndName> channelsAndNames;
    MessageDispatcher * msgDisp;

public:
    ApplicationStatus(MessageDispatcher * msgDisp, std::string filepath = "");
    ~ApplicationStatus();
    void loadChannelMappingFromYaml(std::string pathTofile);
    void printEverything();
    int getVoltageChannelsNum();
    int getCurrentChannelsNum();
    std::vector <ChannelModel *> getChannels();
    std::vector <bool> getSelectedChannels();
    std::vector <uint16_t> getSelectedChannelsIndexes();
};


#endif // APPLICATION_STATUS_H
