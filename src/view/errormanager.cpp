#include "errormanager.h"
#include "globaldefines.h"

QString commLibCode2error(ErrorCodes_t errorCode) {
    QString error;

    switch (errorCode) {
    case Success:
        error = "NONE";
        break;

    case ErrorNoDeviceFound:
        error = "No devices detected";
        break;

    case ErrorListDeviceFailed:
        error = "Devices detection failed";
        break;

    case ErrorEepromAlreadyConnected:
        error = "Connection to device EEPROM failed";
        break;

    case ErrorEepromConnectionFailed:
        error = "Connection to device EEPROM failed";
        break;

    case ErrorEepromDisconnectionFailed:
        error = "Disconnection from device EEPROM failed";
        break;

    case ErrorEepromNotConnected:
        error = "Not connected to device EEPROM";
        break;

    case ErrorEepromReadFailed:
        error = "Read from device EEPROM failed";
        break;

    case ErrorEepromNotRecognized:
        error = "Device EEPROM not recognized";
        break;

    case ErrorDeviceTypeNotRecognized:
        error = "Device type not controllable with EDR4";
        break;

    case ErrorDeviceAlreadyConnected:
        error = "Device already connected";
        break;

    case ErrorDeviceNotConnected:
        error = "Device not connected";
        break;

    case ErrorDeviceConnectionFailed:
        error = "Connection to device failed";
        break;

    case ErrorFtdiConfigurationFailed:
        error = "Ftdi configuration failed";
        break;

    case ErrorDeviceDisconnectionFailed:
        error = "Disconnection from device failed";
        break;

    case ErrorSendMessageFailed:
        error = "Failed to send message to the device";
        break;

    case ErrorCommandNotImplemented:
        error = "Command not implemented";
        break;

    case ErrorValueOutOfRange:
        error = "Value out of range for the device";
        break;

    case ErrorNoDataAvailable:
        error = "No data available";
        break;

    case ErrorFeatureNotImplemented:
        error = "This feature is not implemented for the current device";
        break;

    case ErrorUpgradesNotAvailable:
        error = "No upgrades available for the current device";
        break;

    case ErrorExpiredDevice:
        error = "Expired device";
        break;

    case ErrorUnknown:
        error = "Unknown error";
        break;
    }
    return error;
}

QString commLibCode2info(ErrorCodes_t errorCode) {
    QString info;

    switch (errorCode) {
    case Success:
        info = "";
        break;

    case ErrorNoDeviceFound:
        info = "";
        break;

    case ErrorListDeviceFailed:
        info = "If any device is connected try restarting EDR4.";
        break;

    case ErrorEepromAlreadyConnected:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorEepromConnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorEepromDisconnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorEepromNotConnected:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorEepromReadFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorEepromNotRecognized:
        info = "This device probably is not usable with EDR4.";
        break;

    case ErrorDeviceTypeNotRecognized:
        info = "Check the device info in \"?\" menu and\n"
               "contact support@elements-ic.com to know\n"
               "which software you should use with your device.";
        break;

    case ErrorDeviceAlreadyConnected:
        info = "";
        break;

    case ErrorDeviceNotConnected:
        info = "";
        break;

    case ErrorDeviceConnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorFtdiConfigurationFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorDeviceDisconnectionFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorSendMessageFailed:
        info = "Try to unplug and replug the device and restart EDR4.";
        break;

    case ErrorCommandNotImplemented:
        info = "";
        break;

    case ErrorValueOutOfRange:
        info = "";
        break;

    case ErrorNoDataAvailable:
        info = "";
        break;

    case ErrorFeatureNotImplemented:
        info = "";
        break;

    case ErrorUpgradesNotAvailable:
        info = "";
        break;

    case ErrorExpiredDevice:
        info = "Contact support@elements-ic.com to renew your device.";
        break;

    case ErrorUnknown:
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

ErrorManager::ErrorManager(ErrorCodes_t errorCode, QString info) :
    ErrorManager(commLibCode2error(errorCode), info) {

}

ErrorManager::ErrorManager(ErrorCodes_t errorCode) :
    ErrorManager(errorCode, commLibCode2info(errorCode)) {

}
