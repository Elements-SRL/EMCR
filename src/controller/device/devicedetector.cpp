#include "devicedetector.h"

#include <algorithm>

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
            bool anyChanges = false;
            for (auto oldDevice : devicesList) {
                if (!anyChanges && std::find(detectedList.begin(), detectedList.end(), oldDevice) == detectedList.end()) {
                    anyChanges = true;
                }
            }

            for (auto newDevice : detectedList) {
                if (!anyChanges && std::find(devicesList.begin(), devicesList.end(), newDevice) == devicesList.end()) {
                    anyChanges = true;
                }
            }

            if (anyChanges) {
                devicesList = detectedList;
                emit devicesListChanged(devicesList);
            }
        }
    }
}
