#ifndef MULTIPLECHANNELMODEL_H
#define MULTIPLECHANNELMODEL_H

#include "application_status.h"
#include "multiplechannelcontroldockwidget.h"

class MultipleChannelModel {
public:
    MultipleChannelModel(ApplicationStatus * appStatus, MultipleChannelControlDockWidget * multipleChannelControlsDw);
    ~MultipleChannelModel();

    void turnChannelsAuto(bool flag);

    bool getChannelsAuto();

    void saveSettings();
    void loadSettings();

private:
    ApplicationStatus * appStatus = nullptr;
    MultipleChannelControlDockWidget * view = nullptr;

    bool channelsAuto = false;
    bool channelsAutoSet = false;
};

#endif // MULTIPLECHANNELMODEL_H
