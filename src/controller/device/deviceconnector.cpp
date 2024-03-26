#include "deviceconnector.h"

using namespace e384CommLib;

DeviceConnector::DeviceConnector() :
    QThread() {

}

DeviceConnector::~DeviceConnector() {
    if (msgDisp != nullptr) {
        delete msgDisp;
        msgDisp = nullptr;
    }
}

void DeviceConnector::setDeviceId(QString deviceId) {
    this->deviceId = deviceId;
}

MessageDispatcher * DeviceConnector::getMessageDispatcher() {
    return msgDisp;
}

void DeviceConnector::run() {
    ErrorCodes_t ret = MessageDispatcher::connectDevice(deviceId.toStdString(), msgDisp);
    if (ret != Success) {
        msgDisp = nullptr;
    }
    emit deviceConnected(ret);
}

