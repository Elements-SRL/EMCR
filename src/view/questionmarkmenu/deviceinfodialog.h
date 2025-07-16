#ifndef DEVICEINFODIALOG_H
#define DEVICEINFODIALOG_H

#include <QLabel>
#include <QPushButton>

#include "messagedialog.h"
#include "e384commlib_errorcodes.h"
#include "messagedispatcher.h"

class DeviceInfoDialog : public MessageDialog {
    Q_OBJECT

public:
    DeviceInfoDialog(MessageDispatcher * md, QString deviceId, QWidget * parent = nullptr);

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
