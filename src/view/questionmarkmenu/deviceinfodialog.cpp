#include "deviceinfodialog.h"

#include <QBoxLayout>
#include <QSpacerItem>
#include <QDialogButtonBox>
#include <QApplication>
#include <QClipboard>

#include "globaldefines.h"

DeviceInfoDialog::DeviceInfoDialog(bool connected, QString deviceId, QWidget* parent) :
    MessageDialog("Device Info", true, parent) {

    deviceIdLbl = new QLabel;
    deviceIdLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(deviceIdLbl);

    //deviceVerLbl = new QLabel;
    //deviceVerLbl->setAlignment(Qt::AlignCenter);
    //mainVl->addWidget(deviceVerLbl);

    //deviceSubverLbl = new QLabel;
    //deviceSubverLbl->setAlignment(Qt::AlignCenter);
    //mainVl->addWidget(deviceSubverLbl);

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

    if (connected) {
        deviceIdLbl->setText("Device ID: " + deviceId + " (connected)");
        //deviceVerLbl->setText("Device version: " + QString::number(deviceVer));
        //deviceSubverLbl->setText("Device subversion: " + QString::number(deviceSubver));
        //fwVerLbl->setText("Firmware version: " + QString::number(fwVer));
        copyToClipboardBtn->setVisible(true);
    } else {
        deviceIdLbl->setText("No device connected");
        //deviceVerLbl->setText("");
        //deviceSubverLbl->setText("Please plug a device in to get its information");
        //fwVerLbl->setText("Please plug a device in to get its information");
        copyToClipboardBtn->setVisible(false);
    }

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}

DeviceInfoDialog::~DeviceInfoDialog() {

}

void DeviceInfoDialog::onCopyToClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(deviceIdLbl->text() + "\n" +
        //deviceVerLbl->text() + "\n" +
        //deviceSubverLbl->text() + "\n" +
        fwVerLbl->text());
}