#include "confirmationdialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

ConfirmationDialog::ConfirmationDialog(QWidget* parent):
    MessageDialog("Disconnect device", false, parent) {

    this->setMinimumSize(280, 140);
    mainVl->setContentsMargins(20, 25, 20, 15);
    mainVl->setSpacing(15);

    QLabel* exitLbl = new QLabel("Are you sure you want\nto disconnect the device?");
    exitLbl->setObjectName("dlgExitMsg");

    exitLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(exitLbl);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("dlgCancelBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);

    QPushButton* confirmBtn = new QPushButton("Disconnect");
    confirmBtn->setObjectName("dlgDisconnectBtn");
    confirmBtn->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(confirmBtn);
    mainVl->addLayout(buttonLayout);

    connect(confirmBtn, &QPushButton::clicked, this, &ConfirmationDialog::onAccepted);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    this->centerOnParent(parent);
}

void ConfirmationDialog::onAccepted() {
    this->accept();
}
