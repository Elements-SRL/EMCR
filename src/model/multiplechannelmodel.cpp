#include "multiplechannelmodel.h"

#include <QSettings>

#include "globaldefines.h"

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

void MultipleChannelModel::turnStimulusAuto(bool flag) {
    stimulusAutoSet = true;
    stimulusAuto = flag;
}

void MultipleChannelModel::turnExpandAuto(bool flag) {
    expandAutoSet = true;
    expandAuto = flag;
}

bool MultipleChannelModel::getChannelsAuto() {
    return channelsAuto;
}

bool MultipleChannelModel::getStimulusAuto() {
    return stimulusAuto;
}

bool MultipleChannelModel::getExpandAuto() {
    return expandAuto;
}

void MultipleChannelModel::saveSettings() {
    QSettings settings;

    if (channelsAutoSet) {
        settings.setValue(GLB_CONTROLS_CHANNEL_AUTO_TAG, channelsAuto);
    }

    if (stimulusAutoSet) {
        settings.setValue(GLB_CONTROLS_STIMULUS_AUTO_TAG, stimulusAuto);
    }

    if (expandAutoSet) {
        settings.setValue(GLB_CONTROLS_EXPAND_AUTO_TAG, expandAuto);
    }

    if (appStatus->isPlotDetailAuto()) {
        settings.setValue(GLB_CONTROLS_PLOT_DETAIL_AUTO_TAG, true);
    }
}

void MultipleChannelModel::loadSettings() {
    QSettings settings;

    channelsAuto = settings.value(GLB_CONTROLS_CHANNEL_AUTO_TAG, channelsAuto).toBool();
    view->setChannelsAuto(channelsAuto);

    stimulusAuto = settings.value(GLB_CONTROLS_STIMULUS_AUTO_TAG, stimulusAuto).toBool();
    view->setStimulusAuto(stimulusAuto);

    expandAuto = settings.value(GLB_CONTROLS_EXPAND_AUTO_TAG, expandAuto).toBool();
    view->setExpandAuto(expandAuto);

    auto pda = settings.value(GLB_CONTROLS_PLOT_DETAIL_AUTO_TAG).toBool();
    appStatus->setPlotDetailAuto(pda);
    view->setPlotDetailAuto(pda);
}
