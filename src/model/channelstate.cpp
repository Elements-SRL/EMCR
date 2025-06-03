#include "channelstate.h"

ChannelState::ChannelState(uint16_t index): index(index){

}

// Getter and Setter for index
uint16_t ChannelState::getIndex() const {
    return index;
}

void ChannelState::setIndex(uint16_t value) {
    index = value;
}

// Getter and Setter for expanded
bool ChannelState::isExpanded() const {
    return expanded;
}

void ChannelState::setExpanded(bool value) {
    expanded = value;
}

// Getter and Setter for detailed
bool ChannelState::isDetailed() const {
    return detailed;
}

void ChannelState::setDetailed(bool value) {
    detailed = value;
}

// Getter and Setter for selected
bool ChannelState::isSelected() const {
    return selected;
}

void ChannelState::setSelected(bool value) {
    selected = value;
}
