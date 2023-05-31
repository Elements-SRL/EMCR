#ifndef STATEARRAY_H
#define STATEARRAY_H

#include "model/state.h"

namespace YAML {
typedef struct StateArray {
    int initialState = 0;
    std::vector<YAML::State> states = {{}};
} StateArray_t;

template<>
struct convert<StateArray>{
    static Node encode(const StateArray& rhs) {
        Node node;
        node["initialState"] = rhs.initialState;
        node["states"] = rhs.states;
        return node;
    }

    static bool decode(const Node& node, StateArray& rhs) {
        if(node.size() != 2) {
            return false;
        }
        rhs.initialState = node["initialState"].as<int>();
        rhs.states = node["states"].as<std::vector<YAML::State>>();
        return true;
    }
};
}

void writeStateArrayToFile(YAML::StateArray, std::string);
YAML::StateArray readStateArrayFromFile(std::string fname);
//YAML::StateArray insertState(YAML::StateArray sa, YAML::State s, int idx);

#endif // STATEARRAY_H
