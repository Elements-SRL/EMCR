#include "plotpreferencesmodel.h"

#include <QSettings>

#include "globaldefines.h"
#include "themecontroller.h"

namespace {
// Old default palette, the same for both themes. Only used for the migration:
// a saved color equal to this one (or to its inverse, if the old dark mode was
// active) is not a user choice but a saved default.
const QColor kLegacyDefaultColors[16] = {
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

// Default palette per theme: same hues in the same order as the old one
// (channel 1 blue, 2 red, 3 green, ...), with contrast >= 4:1 against the
// plot background (#F3F3F5 light, #1A202C dark).
const QRgb kLightDefaultColors[16] = {
    0x1D4ED8, 0xDC2626, 0x15803D, 0xC2410C, 0x6B7280, 0x0E7490, 0xC026D3, 0xA16207,
    0x1E3A8A, 0x7F1D1D, 0x14532D, 0x6B21A8, 0x155E75, 0x713F12, 0x374151, 0x047857
};
const QRgb kDarkDefaultColors[16] = {
    0x60A5FA, 0xF87171, 0x4ADE80, 0xFB923C, 0xD1D5DB, 0x22D3EE, 0xE879F9, 0xFACC15,
    0x3B82F6, 0xEF4444, 0x22C55E, 0xA855F7, 0x06B6D4, 0xCA8A04, 0x9CA3AF, 0x34D399
};

QColor invertedColor(const QColor &color) {
    return QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());
}
}

PlotPreferencesModel::PlotPreferencesModel(int channelsNum) :
    channelsNum(channelsNum) {

    this->loadSettings();
}

void PlotPreferencesModel::setColor(PlotPreferencesDialog::SettingType_t type, int channelIdx, QColor color) {
    QSettings settings;
    switch (type) {
    case PlotPreferencesDialog::CurrentColor:
        selectedCurrentColors[channelIdx] = color;
        customColors[channelIdx] = true;
        settings.setValue(this->tagName(PlotPreferencesDialog::CurrentColor, channelIdx), color);
        break;

    case PlotPreferencesDialog::VoltageColor:
        break;
    }
}

QColor PlotPreferencesModel::getColor(int idx) {
    return selectedCurrentColors[idx];
}

QVector <QColor> PlotPreferencesModel::getColors() {
    return selectedCurrentColors;
}

void PlotPreferencesModel::restoreDefaultColors() {
    // Defaults are no longer written to QSettings: removing the keys makes
    // the channels follow the theme palette again
    QSettings settings;
    for (int idx = 0; idx < channelsNum; idx++) {
        settings.remove(this->tagName(PlotPreferencesDialog::CurrentColor, idx));
        customColors[idx] = false;
        selectedCurrentColors[idx] = themeDefaultColor(idx);
    }
}

bool PlotPreferencesModel::refreshDefaultColors() {
    bool changed = false;
    for (int idx = 0; idx < channelsNum; idx++) {
        if (customColors[idx]) {
            continue;
        }
        const QColor color = themeDefaultColor(idx);
        if (color != selectedCurrentColors[idx]) {
            selectedCurrentColors[idx] = color;
            changed = true;
        }
    }
    return changed;
}

QColor PlotPreferencesModel::themeDefaultColor(int channelIdx) {
    const QRgb *palette = (ThemeController::theme() == Light) ? kLightDefaultColors : kDarkDefaultColors;
    return QColor(palette[channelIdx & 0xF]);
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
    }
    return ret;
}

void PlotPreferencesModel::loadSettings() {
    QSettings settings;
    // The old plot "Dark mode" (curve colors inverted, now replaced by the
    // application themes) is only read to migrate the saved colors, then removed
    const bool legacyDarkMode = settings.value(GLB_PREFERENCES_DARK_MODE_TAG, false).toBool();
    settings.remove(GLB_PREFERENCES_DARK_MODE_TAG);

    selectedCurrentColors.resize(channelsNum);
    customColors.fill(false, channelsNum);

    for (int idx = 0; idx < channelsNum; idx++) {
        const QString keyName = this->tagName(PlotPreferencesDialog::CurrentColor, idx);
        selectedCurrentColors[idx] = themeDefaultColor(idx);
        if (!settings.contains(keyName)) {
            continue;
        }

        const QColor saved = settings.value(keyName).value <QColor> ();
        const QColor legacy = kLegacyDefaultColors[idx & 0xF];
        const bool isLegacyDefault = saved.isValid() &&
                (saved.rgb() == legacy.rgb() || (legacyDarkMode && saved.rgb() == invertedColor(legacy).rgb()));

        if (!saved.isValid() || isLegacyDefault) {
            // Migration: it was the old saved default (e.g. from "Restore defaults"),
            // not a user choice -> the channel follows the theme palette
            settings.remove(keyName);
        } else {
            selectedCurrentColors[idx] = saved;
            customColors[idx] = true;
        }
    }
}
