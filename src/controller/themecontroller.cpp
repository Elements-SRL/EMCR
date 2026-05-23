#include "themecontroller.h"
#include <QFile>
#include <QApplication>
#include <QMap>

ThemeController::ThemeController() {}
ThemeController::~ThemeController() {}

// Singleton
ThemeController& ThemeController::getInstance() {
    static ThemeController instance;
    return instance;
}

void ThemeController::applyTheme(Theme theme) {
    // TODO Work in progress QSS
    QString qssPath = ":/styles/emcr_base_test.qss";
    QString processedQss = loadAndProcessQss(qssPath, theme);

    qApp->setStyleSheet(processedQss);
}

QString ThemeController::loadAndProcessQss(const QString &path, Theme theme) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) return "";

    QString qss = QLatin1String(file.readAll());
    QMap<QString, QString> colors;

    colors = getPalette(theme);
    QStringList keys = colors.keys();

    // Sorting color keys to avoid bad sostitutions
    std::sort(keys.begin(), keys.end(), [](const QString &a, const QString &b) {
        return a.length() > b.length();
    });

    // Filling the base template with colors
    for (const QString &key : keys) {
        qss.replace(key, colors.value(key), Qt::CaseSensitive);
    }

    return qss;
}

// DARK PALETTE
// Palette is monochrome with different numeric shades (tonality):
// 900 darker - 100 almost white
static QMap<QString, QString> darkPalette = {
    {"900", "#1A202C"},
    {"800", "#2D3648"},
    {"700", "#4A5468"},
    {"600", "#717D96"},
    {"500", "#A0ABC0"},
    {"400", "#CBD2E0"},
    {"300", "#E2E7F0"},
    {"200", "#D9D9D9"},
    {"100", "#F7F9FC"},
    {"White", "#FFFFFF"}
};

static QMap<QString, QString> darkAccentsPalette = {
    {"Yellow", "#FFD43A"},
    {"Orange", "#FE9D35"},
    {"Green", "#2CD949"},
    {"Blue", "#00A2FF"},
    {"Purple", "#F134F7"},
    {"Red", "#FF5252"},

    {"Yellow Light", "#FFEDAD"},
    {"Orange Light", "#FED7AD"},
    {"Blue Light", "#B0DCFF"},
    {"Purple Light", "#F6D1FF"},
    {"Red Light", "#FFB8B8"},

    {"Selection Blue", "#3a6a9a"},
    {"Selection Green", "#68FF83"},

    {"White 20", "rgba(255, 255, 255, 20)"},
    {"White 16", "rgba(255, 255, 255, 16)"},
    {"White 13", "rgba(255, 255, 255, 13)"},
    {"White 10", "rgba(255, 255, 255, 10)"},
    {"White 8", "rgba(255, 255, 255, 8)"}

};

// LIGHT PALETTE
static QMap<QString, QString> lightPalette = {
    {"900", "#E2E8F0"},
    {"800", "#F8FAFC"},
    {"700", "#F1F5F9"},
    {"600", "#CBD5E1"},
    {"500", "#94A3B8"},
    {"400", "#64748B"},
    {"300", "#334155"},
    {"200", "#1E293B"},
    {"100", "#0F172A"},
    {"White", "#0F172A"}
};

static QMap<QString, QString> lightAccentsPalette = {
    {"Yellow", "#B45309"},
    {"Orange", "#D97706"},
    {"Green", "#16A34A"},
    {"Blue", "#0066CC"},
    {"Purple", "#9333EA"},
    {"Red", "#DC2626"},

    {"Yellow Light", "#FEF3C7"},
    {"Orange Light", "#FFEDD5"},
    {"Blue Light", "#DBEAFE"},
    {"Purple Light", "#F3E8FF"},
    {"Red Light", "#FEE2E2"},

    {"Selection Blue", "#B0DCFF"},
    {"Selection Green", "#BBF7D0"},

    {"White 20", "rgba(0, 0, 0, 0.20)"},
    {"White 16", "rgba(0, 0, 0, 0.16)"},
    {"White 13", "rgba(0, 0, 0, 0.13)"},
    {"White 10", "rgba(0, 0, 0, 0.10)"},
    {"White 8", "rgba(0, 0, 0, 0.08)"}
};

QMap<QString, QString> ThemeController::getPalette(Theme theme) {

    const QMap<QString, QString>* selectedPalette = nullptr;
    const QMap<QString, QString>* selectedAccents = nullptr;
    QString iconPrefix = ":/icons/";

    switch (theme){
    case Dark:
        selectedPalette = &darkPalette;
        selectedAccents = & darkAccentsPalette;
        iconPrefix += "dark";
        break;
    case Light:
        selectedPalette = &lightPalette;
        selectedAccents = &lightAccentsPalette;
        iconPrefix += "light";
        break;
    }

    const QMap<QString, QString>& palette = *selectedPalette;
    const QMap<QString, QString>& accents = *selectedAccents;

    return {
        // --- Panel layers ---
        {"@BG_APP",        palette["900"]},
        {"@BG_PANEL",      palette["800"]},
        {"@BG_HEADER",     palette["700"]},
        {"@BG_INPUT",      palette["900"]},
        {"@BG_BTN",        palette["700"]},
        {"@BG_BTN_HOVER",  palette["600"]},
        {"@BG_BTN_PRESS",  palette["900"]},
        {"@BG_SCROLL",     palette["800"]},

        // --- Borders ---
        {"@BORDER_STD",    palette["800"]},
        {"@BORDER_PANEL",  palette["700"]},
        {"@BORDER_INPUT",  palette["800"]},
        {"@BORDER_FOCUS",  palette["600"]},
        {"@BORDER_LIGHT",  accents["White 8"]},

        // --- Text ---
        {"@TEXT_MAIN",       palette["200"]},
        {"@TEXT_SECONDARY",  palette["400"]},
        {"@TEXT_LIGHT",      palette["100"]},
        {"@TEXT_MUTED",      palette["500"]},

        // --- Items ---
        {"@ITEM_SELECTED",  accents["White 20"]},
        {"@ITEM_PRESS",     accents["White 10"]},
        {"@ITEM_DISABLED",  accents["White 20"]},  

        // --- Accents ---
        {"@ACCENT_RED",       accents["Red"]},
        {"@ACCENT_ORANGE",    accents["Orange"]},
        {"@ACCENT_BLUE",      accents["Blue"]},
        {"@ACCENT_LBLUE",     accents["Blue Light"]},
        {"@SELECTION_BLUE",   accents["Selection Blue"]},

        // --- Badge  ---
        {"@BDG_E_FG", "#4ade80"}, {"@BDG_E_BG", "rgba(74, 222, 128, 0.15)"},
        {"@BDG_P_FG", "#a78bfa"}, {"@BDG_P_BG", "rgba(167, 139, 250, 0.15)"},
        {"@BDG_O_FG", "#a1a1aa"}, {"@BDG_O_BG", "rgba(161, 161, 170, 0.15)"},
        {"@BDG_X_FG", "#f87171"}, {"@BDG_X_BG", "rgba(248, 113, 113, 0.15)"},
        {"@BDG_R_FG", "#fbbf24"}, {"@BDG_R_BG", "rgba(251, 191, 36, 0.15)"},
        {"@BDG_C_FG", "#fafafa"}, {"@BDG_C_BG", "rgba(124, 58, 237, 0.2)"},

        {"@ICON_PREFIX", iconPrefix }
    };
}
