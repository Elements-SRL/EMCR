#ifndef MULTIPLECHANNELMODEL_H
#define MULTIPLECHANNELMODEL_H

#include "application_status.h"
#include "multiplechannelcontroldockwidget.h"

class MultipleChannelModel {
public:
    MultipleChannelModel(ApplicationStatus * appStatus, MultipleChannelControlDockWidget * multipleChannelControlsDw);
    ~MultipleChannelModel();

    void turnChannelsAuto(bool flag);
    void turnStimulusAuto(bool flag);
    void turnExpandAuto(bool flag);
    void turnPlotDetailAuto(bool flag);

    bool getChannelsAuto();
    bool getStimulusAuto();
    bool getExpandAuto();

    void saveSettings();
    void loadSettings();

private:
    ApplicationStatus * appStatus = nullptr;
    MultipleChannelControlDockWidget * view = nullptr;

    bool channelsAuto = false;
    bool channelsAutoSet = false;

    bool stimulusAuto = false;
    bool stimulusAutoSet = false;

    bool expandAuto = false;
    bool expandAutoSet = false;
};

#endif // MULTIPLECHANNELMODEL_H
