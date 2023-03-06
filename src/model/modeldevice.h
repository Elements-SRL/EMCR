#ifndef MODELDEVICE_H
#define MODELDEVICE_H

#include <QString>

#include "messagedispatcher.h"

class ModelDevice {
public:
    ModelDevice();

    QString getSerialNumber();
    bool isConnected();

    void setSerialNumber(QString serial);
    void setConnected(bool flag);

    /*! This is public just to avoid reproducing all the get methods of the messageDispatcher in the ModelDevice class */
    MessageDispatcher * messageDispatcher = nullptr;

private:
    QString serialNumber;
    bool connected;

};

#endif // MODELDEVICE_H
