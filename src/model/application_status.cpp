#include "application_status.h"
#include <iostream>

void ApplicationStatus::loadChannelMappingFromYaml(std::string pathTofile) {
    // Parse the YAML file
    YAML::Node yamlNode = YAML::LoadFile(pathTofile);
    if (yamlNode.IsSequence()){
        std::cout << yamlNode << std::endl;
    }
    channelMappings = yamlNode.as<std::vector<YAML::ChannelMapping>>();
}
