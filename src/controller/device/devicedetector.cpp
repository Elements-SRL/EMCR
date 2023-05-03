#include "devicedetector.h"

#include "globaldefines.h"

DeviceDetector::DeviceDetector() {
    devicesList.clear();
    detectedList.clear();

    detectDevicesTmr = new QTimer();
    detectDevicesTmr->setInterval(2000);
    detectDevicesTmr->setSingleShot(false);

    connect(detectDevicesTmr, &QTimer::timeout, this, &DeviceDetector::detectDevices);

    detectDevicesTmr->start();
}

DeviceDetector::~DeviceDetector() {
    if (detectDevicesTmr != nullptr) {
        detectDevicesTmr->stop();
    }
}

void DeviceDetector::onStartDetecting() {
    detectFlag = true;
}

void DeviceDetector::onStopDetecting() {
    detectFlag = false;
}

void DeviceDetector::detectDevices() {
    if (detectFlag) {
        ErrorCodes_t ret = MessageDispatcher::detectDevices(detectedList);

        if ((ret == Success) || (ret == ErrorNoDeviceFound)) {
            if(devicesList.size() != detectedList.size()){
                devicesList = detectedList;
                emit devicesListChanged(devicesList);
            } else {
                if (!std::equal(detectedList.begin(), detectedList.end(), devicesList.begin())){
                    devicesList = detectedList;
                    emit devicesListChanged(devicesList);
                }
            }
        }
    }
}
