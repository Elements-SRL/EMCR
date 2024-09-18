#include "supportdialog.h"
#include <QLabel>
#include "globaldefines.h"

SupportDialog::SupportDialog(QWidget* parent) :
    MessageDialog("Support", true, parent) {

    QLabel* supportLbl = new QLabel("For support or bug reporting please contact the email address");
    supportLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(supportLbl);

    QLabel* supportEmailLbl = new QLabel("support@elements-ic.com");
    supportEmailLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(supportEmailLbl);

    mainVl->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));

    QLabel* bugReport1Lbl = new QLabel("In bug report emails please describe the bug, add the software version");
    bugReport1Lbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(bugReport1Lbl);

    QLabel* bugReport2Lbl = new QLabel("and the information available in ""Device Info"" dialog.");
    bugReport2Lbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(bugReport2Lbl);

    //QLabel* bugReport3Lbl = new QLabel("Please also attach the last error log in " + ELG_DEFAULT_LOG_PATH);
    //bugReport3Lbl->setAlignment(Qt::AlignCenter);
    //mainVl->addWidget(bugReport3Lbl);

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}
