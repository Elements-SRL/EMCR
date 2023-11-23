#ifndef CHANNEL_MAPPING_H
#define CHANNEL_MAPPING_H

#include <string>
#include <yaml.h>

namespace YAML {
typedef struct ChannelMapping {
    int index;
    std::string name;
    double x;
    double y;
    bool visible;
} ChannelMapping_t;

template<>
struct convert<ChannelMapping>{
    static Node encode(const ChannelMapping& rhs) {
        Node node;
        node["index"] = rhs.index;
        node["name"] = rhs.name;
        node["x"] = rhs.x;
        node["y"] = rhs.y;
        node["visible"] = rhs.visible;
        return node;
    }

    static bool decode(const Node& node, ChannelMapping& rhs) {
        if(node.size() != 5) {
            return false;
        }
        rhs.index = node["index"].as<int>();
        rhs.name = node["name"].as<std::string>();
        rhs.x = node["x"].IsNull()?NULL:node["x"].as<double>();
        rhs.y = node["y"].IsNull()?NULL:node["x"].as<double>();
        rhs.visible = node["visible"].as<bool>();
        return true;
    }
};
}

#endif // CHANNEL_MAPPING_H
