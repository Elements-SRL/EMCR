#include "deviceinfodialog.h"

#include <QBoxLayout>
#include <QSpacerItem>
#include <QDialogButtonBox>
#include <QApplication>
#include <QClipboard>

#include "globaldefines.h"

DeviceInfoDialog::DeviceInfoDialog(MessageDispatcher * md, QString deviceId, QWidget* parent) :
    MessageDialog("Device Info", true, parent) {

    deviceIdLbl = new QLabel;
    deviceIdLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(deviceIdLbl);

    deviceVerLbl = new QLabel;
    deviceVerLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(deviceVerLbl);

    deviceSubverLbl = new QLabel;
    deviceSubverLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(deviceSubverLbl);

    fwVerLbl = new QLabel;
    fwVerLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(fwVerLbl);

    mainVl->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));

    QHBoxLayout* btnHl = new QHBoxLayout;
    mainVl->setContentsMargins(6, 6, 6, 6);
    mainVl->setSpacing(6);

    copyToClipboardBtn = new QPushButton("copy information to clipboard");
    connect(copyToClipboardBtn, &QPushButton::clicked, this, &DeviceInfoDialog::onCopyToClipboard);

    btnHl->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    btnHl->addWidget(copyToClipboardBtn);
    btnHl->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

    mainVl->addLayout(btnHl);

    uint32_t deviceVer;
    uint32_t deviceSubver;
    uint32_t fwMajor;
    uint32_t fwMinor;
    uint32_t fwPatch;
    bool connected;
    ErrorCodes_t ret;

    if (md == nullptr) {
        connected = false;
        ret = MessageDispatcher::getDeviceInfo(deviceId.toStdString(), deviceVer, deviceSubver, fwMajor, fwMinor, fwPatch);
    }
    else {
        connected = true;
        ret = md->getDeviceInfo(deviceVer, deviceSubver, fwMajor, fwMinor, fwPatch);
    }

    deviceIdLbl->setText("Device ID: " + deviceId + (connected ? " (connected)" : " (not connected)"));
    switch (ret) {
    case Success:
        deviceVerLbl->setText("Device version: " + QString::number(deviceVer));
        deviceVerLbl->setVisible(true);
        deviceSubverLbl->setText("Device subversion: " + QString::number(deviceSubver));
        deviceVerLbl->setVisible(true);
        fwVerLbl->setText("Firmware version: " + QString("%1.%2.%3").arg(fwMajor).arg(fwMinor).arg(fwPatch));
        deviceVerLbl->setVisible(true);
        copyToClipboardBtn->setVisible(true);
        break;

    case ErrorDeviceTypeNotRecognized:
        deviceVerLbl->setText("Device not recognized");
        deviceVerLbl->setVisible(true);
        deviceVerLbl->setVisible(false);
        deviceVerLbl->setVisible(false);
        copyToClipboardBtn->setVisible(true);
        break;
    }
    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}

void DeviceInfoDialog::onCopyToClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(deviceIdLbl->text() + "\n" +
        deviceVerLbl->text() + "\n" +
        deviceSubverLbl->text() + "\n" +
        fwVerLbl->text());
}
