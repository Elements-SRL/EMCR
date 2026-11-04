#ifndef PLOTPREFERENCESMODEL_H
#define PLOTPREFERENCESMODEL_H

#include "plotpreferencesdialog.h"

class PlotPreferencesModel {
public:
    PlotPreferencesModel(int channelsNum);

    void setColor(PlotPreferencesDialog::SettingType_t type, int channelIdx, QColor color);
    QColor getColor(int idx);
    QVector <QColor> getColors();
    void restoreDefaultColors();
    // Recomputes the theme default colors for the channels that are not
    // customized. Returns true if at least one color changed.
    bool refreshDefaultColors();

private:
    void loadSettings();
    QString tagName(PlotPreferencesDialog::SettingType_t type, int channelIdx);
    static QColor themeDefaultColor(int channelIdx);

    int channelsNum;
    QVector <QColor> selectedCurrentColors;
    QVector <bool> customColors; // true = color chosen by the user (saved in QSettings)
};

#endif // PLOTPREFERENCESMODEL_H
