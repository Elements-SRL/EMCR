#include "modeldevice.h"

ModelDevice::ModelDevice() {

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
