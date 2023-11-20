#ifndef APPLICATION_STATUS_H
#define APPLICATION_STATUS_H

#include <channel_mapping.h>

class ApplicationStatus {
private:
    std::vector<YAML::ChannelMapping> channelMappings;

public:
    void loadChannelMappingFromYaml(std::string pathTofile);
    void printEverything();
};


#endif // APPLICATION_STATUS_H
