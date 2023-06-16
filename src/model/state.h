#ifndef STATE_H
#define STATE_H

#include "yaml.h"

#include <QString>



namespace YAML {

typedef enum TriggerType {
    CURRENT,
    CONDUCTANCE,
} TriggerType_t;

static std::vector <std::string> triggerTypeStrings = {"Current(pA)", "Conductance(pS)"};

typedef struct State {
    bool activeTimeout = false;
    double timeout = 0.0;
    int timeoutState = 0;
    bool activeTrigger = false;
    bool delta = false;
    double minTrigLevel = 0.0;
    double maxTrigLevel = 0.0;
    int triggerState = 0;
    TriggerType triggerType = TriggerType::CURRENT;
    double voltage = 0.0;
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
        node["triggerType"] = triggerTypeStrings[rhs.triggerType];
        node["voltage"] = rhs.voltage;
        return node;
    }

    static bool decode(const Node& node, State& rhs) {
        if(node.size() != 10) {
            return false;
        }
        rhs.activeTimeout = node["activeTimeout"].as<bool>();
        rhs.timeout = node["timeout"].as<double>();
        rhs.timeoutState = node["timeoutState"].as<int>();
        rhs.activeTrigger = node["activeTrigger"].as<bool>();
        rhs.delta = node["delta"].as<bool>();
        rhs.minTrigLevel = node["minTrigLevel"].as<double>();
        rhs.maxTrigLevel = node["maxTrigLevel"].as<double>();
        rhs.triggerState = node["triggerState"].as<int>();
        rhs.triggerType = (TriggerType)(std::find(triggerTypeStrings.begin(), triggerTypeStrings.end(), node["triggerType"].as<std::string>())-triggerTypeStrings.begin());
        rhs.voltage = node["voltage"].as<double>();
        return true;
    }
};
}

QStringList getListOfTriggerStates(QStringList *);
YAML::TriggerType getTriggerTypeFromString(std::string);

#endif // STATE_H
