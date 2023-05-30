#ifndef STATE_H
#define STATE_H

#include "yaml.h"

namespace YAML {
typedef struct State {
    bool activeTimeout;
    double timeout;
    int timeoutState;
    bool activeTrigger;
    bool delta;
    double minTrigLevel;
    double maxTrigLevel;
    int triggerState;
    std::string triggerType;
    double voltage;
} State_t;

template<>
struct convert<State>{
    static Node encode(const State& rhs) {
        Node node;
        node["activeTimeout"] = rhs.activeTimeout;
        node["timeout"] = rhs.timeout;
        node["timeoutState"] = rhs.timeoutState;
        node["activeTrigger"] = rhs.activeTrigger;
        node["delta"] = rhs.delta;
        node["minTrigLevel"] = rhs.minTrigLevel;
        node["maxTrigLevel"] = rhs.maxTrigLevel;
        node["triggerState"] = rhs.triggerState;
        node["triggerType"] = rhs.triggerType;
        node["voltage"] = rhs.voltage;
        return node;
    }

    static bool decode(const Node& node, State& rhs) {
        if(node.size() != 10) {
            return false;
        }
        rhs.activeTimeout = node["activeTimeout"].as<bool>();
        rhs.timeout = node["activeTimeout"].as<double>();
        rhs.timeoutState = node["timeoutState"].as<int>();
        rhs.activeTrigger = node["activeTrigger"].as<bool>();
        rhs.delta = node["delta"].as<bool>();
        rhs.minTrigLevel = node["minTrigLevel"].as<double>();
        rhs.maxTrigLevel = node["maxTrigLevel"].as<double>();
        rhs.triggerState = node["triggerState"].as<int>();
        rhs.triggerType = node["triggerType"].as<std::string>();
        rhs.voltage = node["voltage"].as<double>();
        return true;
    }
};
}
#endif // STATE_H
