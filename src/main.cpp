#include <QApplication>
#include <QStyleFactory>
#include <QFontDatabase>

#include "maincontroller.h"
#include "globaldefines.h"
#include "qsettings.h"
#include "splashscreen/splashview.h"
#include "statisticsresult.h"
#include "plotmessage.h"
#include "resultwrapper.h"
#include "eventsdirection.h"
#include "themecontroller.h"

// Custom fonts loader
void loadFonts() {
    QFontDatabase::addApplicationFont(":/fonts/Inter.ttf");
    QFontDatabase::addApplicationFont(":/fonts/InriaSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/InriaSans-Bold.ttf");
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

    ThemeController &t = ThemeController::getInstance();
    QSettings settings;
    int savedTheme = settings.value("Preferences/UI/theme").toInt();

    if (savedTheme == Light) {
        t.applyTheme(Light);
    } else {
        t.applyTheme(Dark);
    }

    MainController c;

    // Splashscreen
    SplashView *splash = new SplashView();
    c.setSplash(splash);

    return a.exec();
}
