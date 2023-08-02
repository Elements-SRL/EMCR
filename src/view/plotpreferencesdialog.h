#ifndef PLOTPREFERENCESDIALOG_H
#define PLOTPREFERENCESDIALOG_H

#include <QDialog>

#include "colorselectionbutton.h"

class PlotPreferencesDialog : public QDialog {
    Q_OBJECT

public:
    typedef enum {
        CurrentColor,
        VoltageColor,
        DarkMode
    } SettingType_t;

    PlotPreferencesDialog(int channelsNum, int channelsPerBoard, QWidget * parent = nullptr);

    void setColor(SettingType_t type, int channelIdx, QColor color);
    void setDarkMode(bool flag);

private:
    QVector <ColorSelectionButton *> currentBtns;
    QPushButton * darkModeBtn = nullptr;

signals:
    void channelButtonClicked(int channelIdx);
    void darkModeButtonClicked(bool flag);
    void restoreDefaultButtonClicked();
};

#endif // PLOTPREFERENCESDIALOG_H
