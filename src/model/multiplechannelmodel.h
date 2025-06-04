#ifndef MULTIPLECHANNELMODEL_H
#define MULTIPLECHANNELMODEL_H

#include "application_status.h"
#include "multiplechannelcontroldockwidget.h"

class MultipleChannelModel {
public:
    MultipleChannelModel(ApplicationStatus * appStatus, MultipleChannelControlDockWidget * multipleChannelControlsDw);
    ~MultipleChannelModel();

    void saveSettings();
    void loadSettings();

private:
    ApplicationStatus * appStatus = nullptr;
    MultipleChannelControlDockWidget * view = nullptr;
};

#endif // MULTIPLECHANNELMODEL_H
