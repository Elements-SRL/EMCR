#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <QMessageBox>

#include "e384commlib_errorcodes.h"

using namespace e384CommLib;

class ErrorManager : QMessageBox {
    Q_OBJECT

public:
    ErrorManager(QString error, QString info = "");
    ErrorManager(ErrorCodes_t errorCode, QString info);
    ErrorManager(ErrorCodes_t errorCode);
};

#endif // ERRORMANAGER_H
