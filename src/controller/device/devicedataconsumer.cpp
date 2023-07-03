#include "devicedataconsumer.h"

DeviceDataConsumer::DeviceDataConsumer(MessageDispatcher * msgDisp, DeviceDataProducer * producer) :
    QThread(),
    msgDisp(msgDisp),
    producer(producer) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    totalChannelsNum = voltageChannelsNum+currentChannelsNum;
}

DeviceDataConsumer::~DeviceDataConsumer() {
    this->wait();
}

void DeviceDataConsumer::onResetConsuming() {
    this->onStopConsuming();
    this->onStartConsuming();
}
