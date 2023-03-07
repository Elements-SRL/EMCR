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
        e384cl::ErrorCodes_t ret = MessageDispatcher::detectDevices(detectedList);

        if ((ret == e384cl::Success) || (ret == e384cl::ErrorNoDeviceFound)) {
            devicesList = detectedList;
            emit devicesListChanged(devicesList);
        }
    }
}
