#include <QApplication>
#include <QStyleFactory>
#include <QFontDatabase>

#include "maincontroller.h"
#include "globaldefines.h"
#include "statisticsresult.h"
#include "plotmessage.h"
#include "resultwrapper.h"
#include "eventsdirection.h"

// Custom fonts loader
void loadFonts() {
    QFontDatabase::addApplicationFont(":/fonts/Inter.ttf");
    QFontDatabase::addApplicationFont(":/fonts/InriaSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/InriaSans-Bold.ttf");
}

// Applies a custom QSS style file to the whole app
void applyCustomStyle(QApplication &app) {
    QFile file(":/styles/emcr_dark.qss");
    if(file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
        file.close();
    }
}

int main(int argc, char *argv[]) {
    QApplication::setDesktopSettingsAware(false);
    QApplication a(argc, argv);

    qRegisterMetaType <std::vector <std::string>> ("std::vector <std::string>");
    qRegisterMetaType <ErrorCodes_t> ("ErrorCodes_t");
    qRegisterMetaType <StatisticsResult *> ("StatisticsResult *");
    qRegisterMetaType <QVector <Measurement_t>> ("QVector <Measurement_t>");
    qRegisterMetaType <RangedMeasurement_t> ("RangedMeasurement_t");
    qRegisterMetaType <RecordSettingsDialog::RecordSettings_t> ("RecordSettingsDialog::RecordSettings_t");
    qRegisterMetaType <e384CommLib::ClampingModality_t> ("e384CommLib::ClampingModality_t");
    qRegisterMetaType <OffsetCorrectionController::OffsetCorrectionCheck_t> ("OffsetCorrectionController::OffsetCorrectionCheck_t");
    qRegisterMetaType <ProtocolWidget *> ("ProtocolWidget *");
    qRegisterMetaType <PlotMessage>("PlotMessage");
    qRegisterMetaType <SingleMeasResultWrapper_t>("SingleMeasResultWrapper_t");
    qRegisterMetaType <StatisticsResultWrapper_t>("StatisticsResultWrapper_t");
    qRegisterMetaType <MembraneResultWrapper_t>("MembraneResultWrapper_t");
    qRegisterMetaType <EventsDirection>("EventsDirection");

    QCoreApplication::setOrganizationName("Elements S.R.L.");
    QCoreApplication::setOrganizationDomain("elements-ic.com");
    QCoreApplication::setApplicationName(GLB_SOFTWARE_NAME);

    QDir().mkpath(PSD_DEFAULT_RECORD_PATH);
    QDir().mkpath(YAML_DEFAULT_FOLDER);

    loadFonts();

    QFont defaultFont("Inter");
    defaultFont.setPixelSize(12);
    a.setFont(defaultFont);

    // QSS Custom style
    // TODO - Read preference and load dark/light based on that
    applyCustomStyle(a);

    MainController c;
    return a.exec();
}
