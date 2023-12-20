#ifndef STATEARRAY_H
#define STATEARRAY_H

#include "model/state.h"

namespace YAML {
typedef struct StateArray {
    int initialState = 0;
    double reactionTimeUs = 0.0;
    std::vector<YAML::State> states = {{}};
} StateArray_t;

template<>
struct convert<StateArray>{
    static Node encode(const StateArray& rhs) {
        Node node;
        node["initialState"] = rhs.initialState;
        node["reactionTimeUs"] = rhs.reactionTimeUs;
        node["states"] = rhs.states;
        return node;
    }

    static bool decode(const Node& node, StateArray& rhs) {
        if(node.size() != 3) {
            return false;
        }
        rhs.initialState = node["initialState"].as<int>();
        rhs.reactionTimeUs = node["reactionTimeUs"].as<double>();
        rhs.states = node["states"].as<std::vector<YAML::State>>();
        return true;
    }
};
}

void writeStateArrayToFile(YAML::StateArray, std::string);
YAML::StateArray readStateArrayFromFile(std::string fname);

#endif // STATEARRAY_H
