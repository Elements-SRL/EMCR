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

void MultipleChannelModel::saveSettings() {
    QSettings settings;

    settings.setValue(GLB_CONTROLS_CHANNEL_AUTO_TAG, appStatus->isChannelsAuto());
    settings.setValue(GLB_CONTROLS_STIMULUS_AUTO_TAG, appStatus->isStimulusAuto());
    settings.setValue(GLB_CONTROLS_EXPAND_AUTO_TAG,  appStatus->isExpandAuto());
    settings.setValue(GLB_CONTROLS_PLOT_DETAIL_AUTO_TAG, appStatus->isPlotDetailAuto());
}

void MultipleChannelModel::loadSettings() {
    QSettings settings;
    auto channelsAuto = settings.value(GLB_CONTROLS_CHANNEL_AUTO_TAG).toBool();
    appStatus->setStimulusAuto(channelsAuto);
    view->setChannelsAuto(channelsAuto);

    auto stimulusAuto = settings.value(GLB_CONTROLS_STIMULUS_AUTO_TAG).toBool();
    appStatus->setStimulusAuto(stimulusAuto);
    view->setStimulusAuto(stimulusAuto);

    auto expandAuto = settings.value(GLB_CONTROLS_EXPAND_AUTO_TAG).toBool();
    appStatus->setExpandAuto(expandAuto);
    view->setExpandAuto(expandAuto);

    auto plotDetailAuto = settings.value(GLB_CONTROLS_PLOT_DETAIL_AUTO_TAG).toBool();
    appStatus->setPlotDetailAuto(plotDetailAuto);
    view->setPlotDetailAuto(plotDetailAuto);
}
