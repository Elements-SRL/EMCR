#include <QApplication>

#include "controllermain.h"
#include "mainwindow.h"
#include "globaldefines.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qRegisterMetaType <vector <string>> ("vector <string>");
    qRegisterMetaType <ErrorCodes_t> ("ErrorCodes_t");
    qRegisterMetaType <RecordSettingsDialog::RecordSettings_t> ("RecordSettingsDialog::RecordSettings_t");

    QCoreApplication::setOrganizationName("Elements S.R.L.");
    QCoreApplication::setOrganizationDomain("elements-ic.com");
    QCoreApplication::setApplicationName(GLB_SOFTWARE_NAME);

    ControllerMain c;
    MainWindow w;
    c.setMainWindow(&w);
    w.show();

    return a.exec();
}
