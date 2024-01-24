#include "errormanager.h"
#include "globaldefines.h"

QString commLibCode2error(ErrorCodes_t errorCode) {
    QString error;

    switch (errorCode) {
    /*! \todo FCON riempire la lista messaggi di errore e di info nella funzione sotto */
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
        error = "Device type not controllable with " + GLB_SOFTWARE_NAME + ".";
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

    case ErrorDeviceFwLoadingFailed:
        error = "Failed to load amplifier's FW";
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
        info = "If any device is connected try restarting " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorEepromAlreadyConnected:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorEepromConnectionFailed:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorEepromDisconnectionFailed:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorEepromNotConnected:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorEepromReadFailed:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorEepromNotRecognized:
        info = "This device probably is not usable with " + GLB_SOFTWARE_NAME + ".";
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
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorFtdiConfigurationFailed:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorDeviceDisconnectionFailed:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
        break;

    case ErrorDeviceFwLoadingFailed:
        info = "Please, contact support@elements-ic.com for support, telling that you are using\n"
               + GLB_SOFTWARE_NAME + ", and reporting this problem together with the S/N of the maplifier.";
        break;

    case ErrorSendMessageFailed:
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
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
        info = "Try to unplug and replug the device and restart " + GLB_SOFTWARE_NAME + ".";
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

QString protocolManagerCode2error(ProtocolApplicationStatus_t errorCode) {
    QString error;

    switch (errorCode) {
    case ProtocolApplicationSuccess:
        error = "NONE";
        break;

    case ErrorNotEnoughItemsForSequence:
    case ErrorOverlappingSequences:
    case ErrorMidInfiniteSequence:
    case ErrorItemsOverflow:
    case ErrorItemsNotFound:
    case ErrorItemsOverStimulus:
    case ErrorItemsUnderStimulus:
    case ErrorItemsUnderDuration:
    case ErrorItemsNotProcessed:
        error = "Couldn't start the selected protocol";
        break;

    case ErrorProtocolInhibited:
        error = "The selected protocol is inhibited";
        break;

    }
    return error;
}

QString protocolManagerCode2info(ProtocolApplicationStatus_t errorCode) {
    QString info;

    switch (errorCode) {
    case ProtocolApplicationSuccess:
        info = "NONE";
        break;

    case ErrorNotEnoughItemsForSequence:
        info = "A sequence requires more protocol items than are available.";
        break;

    case ErrorOverlappingSequences:
        info = "Two sequences are overlapped.";
        break;

    case ErrorMidInfiniteSequence:
        info = "Protocol items found after an infinite sequence.";
        break;

    case ErrorItemsOverflow:
        info = "Too many protocol items for this device.";
        break;

    case ErrorItemsNotFound:
        info = "No protocol items found for this device.";
        break;

    case ErrorItemsOverStimulus:
        info = "The stimulus value is too high.";
        break;

    case ErrorItemsUnderStimulus:
        info = "The stimulus value is too low.";
        break;

    case ErrorItemsUnderDuration:
        info = "The duration of one protocol items is too low.";
        break;

    case ErrorItemsNotProcessed:
        info = "The protocol was not processed correctly.\n"
               "Please try again or write to support@elements-ic.com for support.";
        break;

    case ErrorProtocolInhibited:
        info = "";
        break;

    }
    return info;
}

QString protocolListCode2error(ProtocolList::ProtocolListStatus_t errorCode) {
    QString error;

    switch (errorCode) {
    case ProtocolList::Success:
        error = "NONE";
        break;

    case ProtocolList::ErrorProtocolAlreadyExists:
        error = "Couldn't create protocol";
        break;

    case ProtocolList::ErrorNoProtocolSelected:
        error = "No protocol selected";
        break;

    case ProtocolList::ErrorLoadNullProtocolsFail:
        error = "Failed to load stop protocols";
        break;

    case ProtocolList::ErrorLoadOffsetCompensationProtocolFail:
        error = "Failed to load protocol for offset compensation";
        break;

    case ProtocolList::ErrorLoadRestingPotentialProtocolFail:
        error = "Failed to load protocol for resting potential";
        break;

    case ProtocolList::ErrorLoadLastExecutedProtocolFail:
        error = "Failed to load last executed protocol";
        break;

    case ProtocolList::ErrorLoadLastExecutionProtocolsFail:
        error = "No file protocols from last " + GLB_SOFTWARE_NAME + " execution " + YAML_LAST_FULL_FILE + " found";
        break;

    case ProtocolList::ErrorLoadDefaultProtocolsFail:
        error = "Failed to load the file of deafult protocols";
        break;
    }
    return error;
}

QString protocolListCode2info(ProtocolList::ProtocolListStatus_t errorCode) {
    QString info;

    switch (errorCode) {
    case ProtocolList::Success:
        info = "NONE";
        break;

    case ProtocolList::ErrorProtocolAlreadyExists:
        info = "A protocol with the same name already exists.\n"
               "Please select unique names for new protocols.";
        break;

    case ProtocolList::ErrorNoProtocolSelected:
        info = "";
        break;

    case ProtocolList::ErrorLoadNullProtocolsFail:
        info = "Check that the file " + YAML_NULL_FULL_FILE +
                " exists.\nIf it doesn't please copy it from Protocols folder within " + GLB_SOFTWARE_NAME + " installation path.";
        break;

    case ProtocolList::ErrorLoadOffsetCompensationProtocolFail:
        info = "Check that the file " + YAML_VHOLD0_FULL_FILE +
                " exists.\nIf it doesn't please copy it from Protocols folder within " + GLB_SOFTWARE_NAME + " installation path.";
        break;

    case ProtocolList::ErrorLoadRestingPotentialProtocolFail:
        info = "Check that the file " + YAML_IHOLD0_FULL_FILE +
                " exists.\nIf it doesn't please copy it from Protocols folder within " + GLB_SOFTWARE_NAME + " installation path.";
        break;

    case ProtocolList::ErrorLoadLastExecutedProtocolFail:
        info = "It is possible that the file " + YAML_LAST_PROTOCOL_FULL_FILE +
                " was not correctly saved during the last execution.";
        break;

    case ProtocolList::ErrorLoadLastExecutionProtocolsFail:
        info = "Loading default protocols.";
        break;

    case ProtocolList::ErrorLoadDefaultProtocolsFail:
        info = "Check that the file " + YAML_DEFAULT_FULL_FILE +
                " exists.\nIf it doesn't please copy it from Protocols folder within " + GLB_SOFTWARE_NAME + " installation path.";
        break;
    }
    return info;
}

ErrorManager::ErrorManager(ProtocolApplicationStatus_t errorCode, QString info) :
    ErrorManager(protocolManagerCode2error(errorCode), info) {
}

ErrorManager::ErrorManager(ProtocolApplicationStatus_t errorCode) :
    ErrorManager(errorCode, protocolManagerCode2info(errorCode)) {
}

ErrorManager::ErrorManager(ProtocolList::ProtocolListStatus_t errorCode, QString info) :
    ErrorManager(protocolListCode2error(errorCode), info) {
}

ErrorManager::ErrorManager(ProtocolList::ProtocolListStatus_t errorCode) :
    ErrorManager(errorCode, protocolListCode2info(errorCode)) {
}
