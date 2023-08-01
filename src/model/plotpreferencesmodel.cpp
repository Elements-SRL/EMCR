#include "plotpreferencesmodel.h"

#include <QSettings>

#include "globaldefines.h"

PlotPreferencesModel::PlotPreferencesModel(int channelsNum) :
    channelsNum(channelsNum) {

    this->loadSettings();
}

void PlotPreferencesModel::setColor(PlotPreferencesDialog::SettingType_t type, int channelIdx, QColor color) {
    QSettings settings;
    switch (type) {
    case PlotPreferencesDialog::CurrentColor:
        selectedCurrentColors[channelIdx] = color;
        settings.setValue(this->tagName(PlotPreferencesDialog::CurrentColor, channelIdx), color);
        break;

    case PlotPreferencesDialog::VoltageColor:
    case PlotPreferencesDialog::DarkMode:
        break;
    }
}

QColor PlotPreferencesModel::getColor(int idx) {
    return selectedCurrentColors[idx];
}

QString PlotPreferencesModel::tagName(PlotPreferencesDialog::SettingType_t type, int channelIdx) {
    QString ret;
    switch (type) {
    case PlotPreferencesDialog::CurrentColor:
        ret = QString(GLB_PREFERENCES_CURRENT_CHANNEL_X_COLOR_TAG).arg(channelIdx, 3, 10, QChar('0'));
        break;

    case PlotPreferencesDialog::VoltageColor:
        ret = QString(GLB_PREFERENCES_VOLTAGE_CHANNEL_X_COLOR_TAG).arg(channelIdx, 3, 10, QChar('0'));
        break;

    case PlotPreferencesDialog::DarkMode:
        ret = QString(GLB_PREFERENCES_DARK_MODE_TAG);
        break;
    }
    return ret;
}

void PlotPreferencesModel::loadSettings() {
    QColor someColors[16] = {
        QColor(Qt::blue),
        QColor(Qt::red),
        QColor(Qt::green),
        QColor(0xCC6600),
        QColor(Qt::gray),
        QColor(Qt::cyan),
        QColor(Qt::magenta),
        QColor(Qt::yellow),
        QColor(Qt::darkBlue),
        QColor(Qt::darkRed),
        QColor(Qt::darkGreen),
        QColor(Qt::darkMagenta),
        QColor(Qt::darkCyan),
        QColor(Qt::darkYellow),
        QColor(Qt::darkGray),
        QColor(0x00CC66)
    };

    defaultCurrentColors.resize(channelsNum);
    for (int idx = 0; idx < channelsNum; idx++) {
        defaultCurrentColors[idx] = someColors[idx & 0xF];
    }

    QSettings settings;
    QString keyName;

    for (int idx = 0; idx < channelsNum; idx++) {
        keyName = this->tagName(PlotPreferencesDialog::CurrentColor, idx);
        selectedCurrentColors.push_back(settings.value(keyName, defaultCurrentColors[idx]).value <QColor> ());
    }

    darkModeFlag = settings.value(this->tagName(PlotPreferencesDialog::DarkMode, 0), false).toBool();
}
