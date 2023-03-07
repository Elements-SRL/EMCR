#ifndef MODELCHANNEL_H
#define MODELCHANNEL_H

#include "e384commlib_global.h"
#include "e384commlib_global_addendum.h"

using namespace e384CommLib;

class ModelChannel
{
public:
    ModelChannel();

    uint16_t getId();
    bool isOn();
    bool isCompensating();
    bool isInStimActive();
    bool isSelected();
    Measurement_t getVhold();
    Measurement_t getVdoc();

    void setId(uint16_t id);
    void setOn(bool on);
    void setCompensating(bool compensating);
    void setInStimActive(bool active);
    void setSelected(bool selected);
    void setVhold(Measurement_t vHold);
    void setVdoc(Measurement_t vDoc);


private:
    uint16_t id = 65535;                            // Channel ID
    bool on = false;                                // Channel is ON (true) or off (false)
    bool compensating = false;                      // The channel is currently compensating
    bool inStimActive = false;                      // The channel currently has active stimulus (true)
    bool selected = false;                          // The channel is selected in the GUI
    Measurement_t vHold = {0.0, UnitPfxMilli, "V"}; // holding voltage
    Measurement_t vDoc = {0.0, UnitPfxMilli, "V"};  // digital offset compensation voltage

};

#endif // MODELCHANNEL_H
