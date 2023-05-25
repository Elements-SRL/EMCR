#include "addtagdialog.h"

#include <QBoxLayout>

AddTagDialog::AddTagDialog() :
    QDialog(nullptr) {

    this->setWindowTitle("Add tag");

    /*! Hides the help button on the window bar */
    Qt::WindowFlags flags = this->windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    this->setWindowFlags(flags);

    QVBoxLayout * mainVl = new QVBoxLayout;
    this->setLayout(mainVl);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    tagEdit = new QLineEdit;
    tagEdit->setPlaceholderText("Write tag here");
    mainVl->addWidget(tagEdit);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddTagDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddTagDialog::reject);
    mainVl->addWidget(buttonBox);
}

QString AddTagDialog::getTag() {
    return tagEdit->text();
}

void AddTagDialog::open() {
    tagEdit->setText("");
    QDialog::open();
}
