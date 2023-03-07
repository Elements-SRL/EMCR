#include "modeldevice.h"

ModelDevice::ModelDevice() {

}

MessageDispatcher * ModelDevice::getMessageDispatcher() {
    return messageDispatcher;
}

void ModelDevice::setMessageDispatcher(MessageDispatcher * messageDispatcher) {
    this->messageDispatcher = messageDispatcher;
}

QString ModelDevice::getSerialNumber() {
    return this->serialNumber;
}

bool ModelDevice::isConnected() {
    return connected;
}

void ModelDevice::setSerialNumber(QString serial) {
    serialNumber = serial;
}

void ModelDevice::setConnected(bool flag) {
    connected = flag;
}

e384cl::ErrorCodes_t ModelDevice::getChannelsNumber(int &voltageChannelsNum, int &currentChannelsNum) {
    uint16_t vNum;
    uint16_t cNum;
    e384cl::ErrorCodes_t ret = messageDispatcher->getChannelNumberFeatures(vNum, cNum);
    if (ret == e384cl::Success) {
        voltageChannelsNum = (int)vNum;
        currentChannelsNum = (int)cNum;
    }
    return ret;
}
