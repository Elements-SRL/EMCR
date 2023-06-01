#include <QApplication>

#include "controllermain.h"
#include "mainwindow.h"
#include "globaldefines.h"
#include "controller/controllerstatearray.h"
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qRegisterMetaType <std::vector <std::string>> ("std::vector <std::string>");
    qRegisterMetaType <ErrorCodes_t> ("ErrorCodes_t");
    qRegisterMetaType <LiveNoiseConsumer::Result_t> ("LiveNoiseConsumer::Result_t");
    qRegisterMetaType <RecordSettingsDialog::RecordSettings_t> ("RecordSettingsDialog::RecordSettings_t");
    qRegisterMetaType <e384CommLib::ClampingModality_t> ("e384CommLib::ClampingModality_t");
    qRegisterMetaType <ProtocolWidget *> ("ProtocolWidget *");

    QCoreApplication::setOrganizationName("Elements S.R.L.");
    QCoreApplication::setOrganizationDomain("elements-ic.com");
    QCoreApplication::setApplicationName(GLB_SOFTWARE_NAME);

    QDir().mkpath(PSD_DEFAULT_RECORD_PATH);
    QDir().mkpath(YAML_DEFAULT_FOLDER);
    QDir().mkpath(CCS_CALIBRATION_DEFAULT_PATH);

    ControllerMain c;
    MainWindow w;
    c.setMainWindow(&w);
    w.show();

    ControllerStateArray * controllerStateArray = new ControllerStateArray();
    controllerStateArray->showWidget();
//    controllerStateArray->open("ciccia.yaml");
    return a.exec();
}
