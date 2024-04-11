#include "supportdialog.h"
#include <QLabel>
//#include "errorlogger.h"
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

#if defined(_WIN32)
    QLabel* bugReport1Lbl = new QLabel("In bug report emails please briefly describe the bug, include");
    bugReport1Lbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(bugReport1Lbl);

    QLabel* bugReport2Lbl = new QLabel("the information available in ""Device Info"" dialog and attach");
    bugReport2Lbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(bugReport2Lbl);

    //QLabel* bugReport3Lbl = new QLabel("the last error log in " + ELG_DEFAULT_LOG_PATH);
    //bugReport3Lbl->setAlignment(Qt::AlignCenter);
    //mainVl->addWidget(bugReport3Lbl);
#else
    QLabel* bugReport1Lbl = new QLabel("In bug report emails please briefly describe the bug and");
    bugReport1Lbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(bugReport1Lbl);

    QLabel* bugReport2Lbl = new QLabel("include the information available in ""Device Info"" dialog.");
    bugReport2Lbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(bugReport2Lbl);
#endif

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}

SupportDialog::~SupportDialog() {

}
