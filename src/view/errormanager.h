#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <QMessageBox>
#include "protocol/protocolmanager.h"
#include "protocol/protocollist.h"
#include "e384commlib_errorcodes.h"

using namespace e384CommLib;

class ErrorManager : QMessageBox {
    Q_OBJECT

public:
    ErrorManager(QString error, QString info = "");
    ErrorManager(ErrorCodes_t errorCode, QString info);
    ErrorManager(ErrorCodes_t errorCode);
    ErrorManager(ProtocolApplicationStatus_t errorCode, QString info);
    ErrorManager(ProtocolApplicationStatus_t errorCode);
    ErrorManager(ProtocolList::ProtocolListStatus_t errorCode, QString info);
    ErrorManager(ProtocolList::ProtocolListStatus_t errorCode);
};

#endif // ERRORMANAGER_H
