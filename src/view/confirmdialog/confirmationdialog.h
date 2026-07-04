#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include "messagedialog.h"

#include <QObject>

class ConfirmationDialog : public MessageDialog {
    Q_OBJECT

public:
    ConfirmationDialog(QWidget* parent = nullptr);

protected slots:
    virtual void onAccepted();
};

#endif // CONFIRMATIONDIALOG_H
