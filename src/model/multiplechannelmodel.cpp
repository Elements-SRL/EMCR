#include "multiplechannelmodel.h"

#include <QSettings>

MultipleChannelModel::MultipleChannelModel(ApplicationStatus * appStatus, MultipleChannelControlDockWidget * multipleChannelControlsDw) :
    appStatus(appStatus),
    view(multipleChannelControlsDw) {

    this->loadSettings();
}

MultipleChannelModel::~MultipleChannelModel() {
    this->saveSettings();
}

void MultipleChannelModel::turnChannelsAuto(bool flag) {
    channelsAutoSet = true;
    channelsAuto = flag;
}

bool MultipleChannelModel::getChannelsAuto() {
    return channelsAuto;
}

void MultipleChannelModel::saveSettings() {
    QSettings settings;

    if (channelsAutoSet) {
        settings.setValue(GLB_CONTROLS_CHANNEL_AUTO_TAG, channelsAuto);
    }
}

void MultipleChannelModel::loadSettings() {
    QSettings settings;

    channelsAuto = settings.value(GLB_CONTROLS_CHANNEL_AUTO_TAG, channelsAuto).toBool();
    view->setChannelsAuto(channelsAuto);
}
