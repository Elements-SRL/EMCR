#ifndef CHANNELSTATE_H
#define CHANNELSTATE_H

#include <stdint.h>
class ChannelState
{

private:
    uint16_t index = 0;
    bool expanded = 0;
    bool detailed = 0;
    bool selected = 0;

public:
    ChannelState(uint16_t index);

    // Getter and Setter for index
    uint16_t getIndex() const;
    void setIndex(uint16_t value);

    // Getter and Setter for expanded
    bool isExpanded() const;
    void setExpanded(bool value);

    // Getter and Setter for detailed
    bool isDetailed() const;
    void setDetailed(bool value);

    // Getter and Setter for selected
    bool isSelected() const;
    void setSelected(bool value);
};

#endif // CHANNELSTATE_H
