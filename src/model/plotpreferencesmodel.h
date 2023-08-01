#ifndef PLOTPREFERENCESMODEL_H
#define PLOTPREFERENCESMODEL_H

#include "plotpreferencesdialog.h"

class PlotPreferencesModel {
public:
    PlotPreferencesModel(int channelsNum);

    void setColor(PlotPreferencesDialog::SettingType_t type, int channelIdx, QColor color);
    QColor getColor(int idx);

private:
    void loadSettings();
    QString tagName(PlotPreferencesDialog::SettingType_t type, int channelIdx);

    int channelsNum;
    QVector <QColor> selectedCurrentColors;
    QVector <QColor> defaultCurrentColors;
    bool darkModeFlag = false;
};

#endif // PLOTPREFERENCESMODEL_H
