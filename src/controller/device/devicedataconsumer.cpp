#include "devicedataconsumer.h"

DeviceDataConsumer::DeviceDataConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    QThread(),
    appStatus(appStatus),
    producer(producer) {
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    voltageRange.resize(voltageChannelsNum);
    RangedMeasurement_t zeroV = {0.0, 1.0, 1.0, UnitPfxNone, "V"};
    std::fill(voltageRange.begin(), voltageRange.end(), zeroV);
    currentRange.resize(currentChannelsNum);
    RangedMeasurement_t zeroA = {0.0, 1.0, 1.0, UnitPfxNone, "A"};
    std::fill(currentRange.begin(), currentRange.end(), zeroA);
}

DeviceDataConsumer::~DeviceDataConsumer() {
    this->wait();
}

void DeviceDataConsumer::onResetConsuming() {
    this->onStopConsuming();
    this->onStartConsuming();
}

ApplicationStatus * DeviceDataConsumer::getAppStatus() {
    return appStatus;
}
