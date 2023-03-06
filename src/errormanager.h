#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <QMessageBox>

#include "e384commlib_errorcodes.h"

namespace e384cl = e384CommLib;

class ErrorManager : QMessageBox {
    Q_OBJECT

public:
    ErrorManager(QString error, QString info = "");
    ErrorManager(e384cl::ErrorCodes_t errorCode, QString info);
    ErrorManager(e384cl::ErrorCodes_t errorCode);
};

#endif // ERRORMANAGER_H
