#include "errormanager.h"
#include "globaldefines.h"

QString commLibCode2error(e384cl::ErrorCodes_t errorCode) {
    QString error;

    switch (errorCode) {
    case e384cl::Success:
        error = "NONE";
        break;

    case e384cl::ErrorNoDeviceFound:
        error = "No devices detected";
        break;

    case e384cl::ErrorListDeviceFailed:
        error = "Devices detection failed";
        break;

    case e384cl::ErrorEepromAlreadyConnected:
        error = "Connection to device EEPROM failed";
        break;

    case e384cl::ErrorEepromConnectionFailed:
        error = "Connection to device EEPROM failed";
        break;

    case e384cl::ErrorEepromDisconnectionFailed:
        error = "Disconnection from device EEPROM failed";
        break;

    case e384cl::ErrorEepromNotConnected:
        error = "Not connected to device EEPROM";
        break;

    case e384cl::ErrorEepromReadFailed:
        error = "Read from device EEPROM failed";
        break;

    case e384cl::ErrorEepromNotRecognized:
        error = "Device EEPROM not recognized";
        break;

    case e384cl::ErrorDeviceTypeNotRecognized:
        error = "Device type not controllable with EDR4";
        break;

    case e384cl::ErrorDeviceAlreadyConnected:
        error = "Device already connected";
        break;

    case e384cl::ErrorDeviceNotConnected:
        error = "Device not connected";
        break;

    case e384cl::ErrorDeviceConnectionFailed:
        error = "Connection to device failed";
        break;

    case e384cl::ErrorFtdiConfigurationFailed:
        error = "Ftdi configuration failed";
        break;

    case e384cl::ErrorDeviceDisconnectionFailed:
        error = "Disconnection from device failed";
        break;

    case e384cl::ErrorSendMessageFailed:
        error = "Failed to send message to the device";
        break;

    case e384cl::ErrorCommandNotImplemented:
        error = "Command not implemented";
        break;

    case e384cl::ErrorValueOutOfRange:
        error = "Value out of range for the device";
        break;

    case e384cl::ErrorNoDataAvailable:
        error = "No data available";
        break;

    case e384cl::ErrorFeatureNotImplemented:
        error = "This feature is not implemented for the current device";
        break;

    case e384cl::ErrorUpgradesNotAvailable:
        error = "No upgrades available for the current device";
        break;

    case e384cl::ErrorExpiredDevice:
        error = "Expired device";
        break;

    case e384cl::ErrorUnknown:
        error = "Unknown error";
        break;
    }
    return error;
}

QString commLibCode2info(e384cl::ErrorCodes_t errorCode) {
    QString info;

    switch (errorCode) {
    case e384cl::Success:
        info = "";
        break;

    case e384cl::ErrorNoDeviceFound:
        info = "";
        break;

    case e384cl::ErrorListDeviceFailed:
        info = "If any device is connected try restarting EDR4.";
        break;

    case e384cl::ErrorEepromAlreadyConnected:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorEepromConnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorEepromDisconnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorEepromNotConnected:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorEepromReadFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorEepromNotRecognized:
        info = "This device probably is not usable with EDR4.";
        break;

    case e384cl::ErrorDeviceTypeNotRecognized:
        info = "Check the device info in \"?\" menu and\n"
               "contact support@elements-ic.com to know\n"
               "which software you should use with your device.";
        break;

    case e384cl::ErrorDeviceAlreadyConnected:
        info = "";
        break;

    case e384cl::ErrorDeviceNotConnected:
        info = "";
        break;

    case e384cl::ErrorDeviceConnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorFtdiConfigurationFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorDeviceDisconnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorSendMessageFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case e384cl::ErrorCommandNotImplemented:
        info = "";
        break;

    case e384cl::ErrorValueOutOfRange:
        info = "";
        break;

    case e384cl::ErrorNoDataAvailable:
        info = "";
        break;

    case e384cl::ErrorFeatureNotImplemented:
        info = "";
        break;

    case e384cl::ErrorUpgradesNotAvailable:
        info = "";
        break;

    case e384cl::ErrorExpiredDevice:
        info = "Contact support@elements-ic.com to renew your device.";
        break;

    case e384cl::ErrorUnknown:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;
    }
    return info;
}

ErrorManager::ErrorManager(QString error, QString info) {
    if (error != "NONE") {
        if (info.size() > 0) {
            error += ":\n" + info;

        } else {
            error += ".";
        }
        this->warning(nullptr, GLB_SOFTWARE_NAME, error);
        this->setFixedSize(500, 200);

    } else {
        /*! \todo FCON cosa si fa se si riceve un Success? */
    }
}

ErrorManager::ErrorManager(e384cl::ErrorCodes_t errorCode, QString info) :
    ErrorManager(commLibCode2error(errorCode), info) {

}

ErrorManager::ErrorManager(e384cl::ErrorCodes_t errorCode) :
    ErrorManager(errorCode, commLibCode2info(errorCode)) {

}
