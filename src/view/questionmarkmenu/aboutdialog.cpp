#include "aboutdialog.h"
#include <QLabel>
#include "globaldefines.h"

AboutDialog::AboutDialog(QWidget* parent) :
    MessageDialog(QString("About ") + GLB_SOFTWARE_NAME, true, parent) {

    QLabel* versionLbl = new QLabel(QString("Version ") + GLB_SOFTWARE_VERSION_NUMBER);
    versionLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(versionLbl);

    QLabel* copyrightLbl = new QLabel("Copyright (c) 2015-2024 by Elements s.r.l.");
    copyrightLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(copyrightLbl);

    QLabel* emailLbl = new QLabel("info@elements-ic.com");
    emailLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(emailLbl);

    QLabel* websiteLbl = new QLabel("www.elements-ic.com");
    websiteLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(websiteLbl);

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}

AboutDialog::~AboutDialog() {

}
