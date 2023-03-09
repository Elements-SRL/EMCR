#include "devicedataconsumer.h"

DeviceDataConsumer::DeviceDataConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    QThread(),
    mDev(mDev),
    producer(producer) {

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);
    totalChannelsNum = voltageChannelsNum+currentChannelsNum;
}

DeviceDataConsumer::~DeviceDataConsumer() {
    this->wait();
}

void DeviceDataConsumer::onResetConsuming() {
    this->onStopConsuming();
    this->onStartConsuming();
}
