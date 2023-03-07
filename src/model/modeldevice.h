#ifndef MODELDEVICE_H
#define MODELDEVICE_H

#include <QString>

#include "messagedispatcher.h"

namespace e384cl = e384CommLib;

class ModelDevice {
public:
    ModelDevice();

    MessageDispatcher * getMessageDispatcher();
    void setMessageDispatcher(MessageDispatcher * messageDispatcher);

    QString getSerialNumber();
    bool isConnected();

    void setSerialNumber(QString serial);
    void setConnected(bool flag);

    e384cl::ErrorCodes_t getChannelsNumber(int &voltageChannelNum, int &CurrentChannelNum);

private:
    MessageDispatcher * messageDispatcher = nullptr;

    QString serialNumber;
    bool connected = false;
};

#endif // MODELDEVICE_H
