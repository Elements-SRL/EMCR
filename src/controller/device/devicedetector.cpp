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

        /*! An FTDI device is plugged in but its library cannot be loaded: the device cannot be listed.
         *  Tell the user once, until the library is found or the device is unplugged */
        std::string ftdiDetails;
        if (MessageDispatcher::getFtdiDriverStatus(ftdiDetails) == ErrorFtdiDriverNotFound) {
            if (!ftdiDriverErrorReported) {
                ftdiDriverErrorReported = true;
                emit ftdiDriverError(QString::fromStdString(ftdiDetails));
            }
        } else {
            ftdiDriverErrorReported = false;
        }

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
