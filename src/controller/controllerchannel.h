#ifndef CONTROLLERCHANNEL_H
#define CONTROLLERCHANNEL_H

#include <QObject>
#include <QVector>

#include "modeldevice.h"


class ControllerChannel : public QObject {
    Q_OBJECT

public:
    ControllerChannel();

    void setModelDevice(ModelDevice * mDev);

private:
    ModelDevice * mDev = nullptr;

public slots:
    void onSingleChannelsClicked(uint16_t changedChannelIndexes, bool newChannelState);
    void onOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void onOneRowClicked(uint16_t changedRowIndexes, bool newChannelState);
    void onAllChannelsClicked(bool newChannelState);


    signals:
    void updateChannelControlDockWidget();
};

#endif // CONTROLLERCHANNEL_H
