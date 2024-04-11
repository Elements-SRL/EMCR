#ifndef DEVICEINFODIALOG_H
#define DEVICEINFODIALOG_H

#include <QLabel>
#include <QPushButton>

#include "messagedialog.h"
#include "e384commlib_errorcodes.h"

class DeviceInfoDialog : public MessageDialog {
    Q_OBJECT

public:
    DeviceInfoDialog(bool connected, QString deviceId, QWidget* parent = nullptr);
    ~DeviceInfoDialog();

private slots:
    void onCopyToClipboard();

private:
    QLabel* deviceIdLbl;
    QLabel* deviceVerLbl;
    QLabel* deviceSubverLbl;
    QLabel* fwVerLbl;
    QPushButton* copyToClipboardBtn;
};

#endif // DEVICEINFODIALOG_H