#ifndef DEVICECONNECTOR_H
#define DEVICECONNECTOR_H

#include <QThread>

#include "messagedispatcher.h"

class DeviceConnector : public QThread {
    Q_OBJECT

public:
    DeviceConnector();
    ~DeviceConnector();

    void setDeviceId(QString deviceId);
    MessageDispatcher * getMessageDispatcher();

protected:
    void run() override;

private:
    QString deviceId;
    MessageDispatcher * msgDisp = nullptr;

signals:
    void deviceConnected(ErrorCodes_t ret);
};

#endif // DEVICECONNECTOR_H
