#include "devicedataconsumer.h"

DeviceDataConsumer::DeviceDataConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    QThread(),
    appStatus(appStatus),
    producer(producer) {
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    totalChannelsNum = voltageChannelsNum+currentChannelsNum;
}

DeviceDataConsumer::~DeviceDataConsumer() {
    this->wait();
}

void DeviceDataConsumer::onResetConsuming() {
    this->onStopConsuming();
    this->onStartConsuming();
}
