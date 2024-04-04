#include "upgradefwview.h"

UpgradeFwView::UpgradeFwView(QWidget * parent) :
    QDialog(parent) {

    /*! Hide the help and close button on the window bar */
    this->setWindowFlags(Qt::WindowTitleHint);

    QVBoxLayout * mainVl = new QVBoxLayout;
    this->setLayout(mainVl);

    messageLbl = new QLabel("");
    mainVl->addWidget(messageLbl);

    QHBoxLayout * buttonsHl = new QHBoxLayout;
    mainVl->addLayout(buttonsHl);

    okBtn = new QPushButton("OK");
    buttonsHl->addWidget(okBtn);

    upgradeBtn = new QPushButton("UPGRADE");
    buttonsHl->addWidget(upgradeBtn);
    upgradeBtn->setEnabled(false);

    cancelBtn = new QPushButton("Cancel");
    buttonsHl->addWidget(cancelBtn);

    connect(okBtn, &QPushButton::clicked, this, &UpgradeFwView::accept);
    connect(upgradeBtn, &QPushButton::clicked, this, &UpgradeFwView::sigUpgradeFw);
    connect(cancelBtn, &QPushButton::clicked, this, &UpgradeFwView::reject);
}

void UpgradeFwView::onUpgradeAvailable(bool flag) {
    upgradeBtn->setEnabled(flag);
}

void UpgradeFwView::onSetMessage(QString message) {
    messageLbl->setText(message);
}
