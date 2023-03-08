#ifndef CHANNELCONTROLDOCKWIDGET_H
#define CHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>

#include "modeldevice.h"

class ChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ChannelControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

public slots:
    void onUpdate();

private:
    QWidget * createTurnChannelsOnOffWidget();

    ModelDevice * mDev = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    QComboBox * operationCbx = nullptr;

    QWidget * turnChannelsOnOffWidget = nullptr;
    QVector <QCheckBox *> turnChannelsOnOffBtns;
};

#endif // CHANNELCONTROLDOCKWIDGET_H
