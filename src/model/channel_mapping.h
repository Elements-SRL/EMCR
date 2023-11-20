#ifndef CHANNEL_MAPPING_H
#define CHANNEL_MAPPING_H

#include <string>
#include <yaml.h>

namespace YAML {
typedef struct ChannelMapping {
    int originalChannelIndex;
    std::string originalChannelName;
    int newChannelIndex;
    std::string newChannelName;
    double x;
    double y;
    bool visible;
} ChannelMapping_t;

template<>
struct convert<ChannelMapping>{
    static Node encode(const ChannelMapping& rhs) {
        Node node;
        node["original_ch_index"] = rhs.originalChannelIndex;
        node["original_ch_name"] = rhs.originalChannelName;
        node["new_ch_index"] = rhs.newChannelIndex;
        node["new_ch_name"] = rhs.newChannelName;
        node["x"] = rhs.x;
        node["y"] = rhs.y;
        node["visible"] = rhs.visible;
        return node;
    }

    static bool decode(const Node& node, ChannelMapping& rhs) {
        if(node.size() != 7) {
            return false;
        }
        rhs.originalChannelIndex = node["original_ch_index"].as<int>();
        rhs.originalChannelName = node["original_ch_name"].as<std::string>();
        rhs.newChannelIndex = node["new_ch_index"].as<int>();
        rhs.newChannelName = node["new_ch_name"].as<std::string>();
        rhs.x = node["x"].IsNull()?NULL:node["x"].as<double>();
        rhs.y = node["y"].IsNull()?NULL:node["x"].as<double>();
        rhs.visible = node["visible"].as<bool>();
        return true;
    }
};
}

#endif // CHANNEL_MAPPING_H
