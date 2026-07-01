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
    this->currentTheme = theme;
    QString qssPath = ":/styles/emcr_base_test.qss";
    QString processedQss = loadAndProcessQss(qssPath, theme);
    qApp->setStyleSheet(processedQss);

    /* Emit signal for those widgets that should
     * handle manually the theme update
     */
    emit sigThemeUpdated();
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
    {"500", "rgba(160, 171, 192, 70%)"},
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
    {"Red", "rgb(255, 82, 82)"},

    {"Yellow Light", "#FFEDAD"},
    {"Orange Light", "#FED7AD"},
    {"Blue Light", "#B0DCFF"},
    {"Purple Light", "#F6D1FF"},
    {"Red Light", "rgba(255, 82, 82, 70%)"},

    {"Selection Blue", "#3a6a9a"},
    {"Selection Green", "#68FF83"},
    {"Table header", "#2B313C"},

    {"White 20", "rgba(255, 255, 255, 20%)"},
    {"White 16", "rgba(255, 255, 255, 16%)"},
    {"White 13", "rgba(255, 255, 255, 13%)"},
    {"White 10", "rgba(255, 255, 255, 10%)"},
    {"White 8", "rgba(255, 255, 255, 8%)"}

};


// LIGHT PALETTE
static QMap<QString, QString> lightPalette = {
    {"900", "#E2E5E6"},
    {"800", "#F3F3F5"},
    {"700", "#F8F8F8"},
    {"600", "#DADEE3"},
    {"500", "#C9CDD0"},
    {"400", "#9EA0A2"},
    {"300", "#797B7A"},
    {"200", "#585858"},
    {"100", "#3E3E3E"},
    {"White", "#0A0A0A"}
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

    {"White 20", "rgba(0, 0, 0, 20%)"},
    {"White 16", "rgba(0, 0, 0, 16%)"},
    {"White 13", "rgba(0, 0, 0, 13%)"},
    {"White 10", "rgba(0, 0, 0, 10%)"},
    {"White 8", "rgba(0, 0, 0, 8%)"}
};

QMap<QString, QString> ThemeController::getPalette(Theme theme) {

    QMap<QString, QString> variables;
    QString iconPrefix = ":/theme/";

    if (theme == Dark) {
        iconPrefix += "dark/icons";

        variables = {
            // --- Panel layers ---
            {"@BG_APP",        darkPalette["900"]},
            {"@BG_PANEL",      darkPalette["800"]},
            {"@BG_HEADER",     darkPalette["700"]},
            {"@BG_INPUT",      darkPalette["900"]},
            {"@BG_BTN",        darkPalette["700"]},
            {"@BG_BTN_HOVER",  darkPalette["600"]},
            {"@BG_BTN_PRESS",  darkPalette["900"]},
            {"@BG_SCROLL",     darkPalette["800"]},

            // --- Borders ---
            {"@BORDER_STD",    darkPalette["800"]},
            {"@BORDER_PANEL",  darkPalette["700"]},
            {"@BORDER_INPUT",  darkPalette["700"]},
            {"@BORDER_FOCUS",  darkPalette["600"]},
            {"@BORDER_LIGHT",  darkAccentsPalette["White 8"]},

            // --- Text ---
            {"@TEXT_MAIN",       darkPalette["200"]},
            {"@TEXT_SECONDARY",  darkPalette["400"]},
            {"@TEXT_LIGHT",      darkPalette["100"]},
            {"@TEXT_MUTED",      darkPalette["500"]},
            {"@TEXT_TERTIARY",   darkPalette["600"]},

            // --- Items ---
            {"@ITEM_SELECTED",  darkAccentsPalette["White 20"]},
            {"@ITEM_PRESS",     darkAccentsPalette["White 10"]},
            {"@ITEM_DISABLED",  darkAccentsPalette["White 20"]},

            // --- Accents ---
            {"@ACCENT_RED",       darkAccentsPalette["Red"]},
            {"@ACCENT_LRED",      darkAccentsPalette["Red Light"]},
            {"@ACCENT_ORANGE",    darkAccentsPalette["Orange"]},
            {"@ACCENT_BLUE",      darkAccentsPalette["Blue"]},
            {"@ACCENT_LBLUE",     darkAccentsPalette["Blue Light"]},
            {"@SELECTION_BLUE",   darkAccentsPalette["Selection Blue"]},
            {"@TABLE_HEADER",     darkAccentsPalette["Table header"]},

            // --- Badges ---
            {"@BDG_E_FG", "#4ade80"}, {"@BDG_E_BG", "rgba(74, 222, 128, 15%)"},
            {"@BDG_P_FG", "#a78bfa"}, {"@BDG_P_BG", "rgba(167, 139, 250, 15%)"},
            {"@BDG_O_FG", "#a1a1aa"}, {"@BDG_O_BG", "rgba(161, 161, 170, 15%)"},
            {"@BDG_X_FG", "#f87171"}, {"@BDG_X_BG", "rgba(248, 113, 113, 15%)"},
            {"@BDG_R_FG", "#fbbf24"}, {"@BDG_R_BG", "rgba(251, 191, 36, 15%)"},
            {"@BDG_C_FG", "#fafafa"}, {"@BDG_C_BG", "rgba(124, 58, 237, 20%)"},

            // --- Badges - chessboard solid colors ---
            {"@BDG_E_FG_CH", "#FFFFFF"}, {"@BDG_E_BG_CH", "#16a34a"},
            {"@BDG_P_FG_CH", "#FFFFFF"}, {"@BDG_P_BG_CH", "#7c3aed"},
            {"@BDG_O_FG_CH", "#FFFFFF"}, {"@BDG_O_BG_CH", "#4b5563"},
            {"@BDG_X_FG_CH", "#FFFFFF"}, {"@BDG_X_BG_CH", "#dc2626"},
            {"@BDG_R_FG_CH", "#1e293b"}, {"@BDG_R_BG_CH", "#eab308"},
            {"@BDG_C_FG_CH", "#FFFFFF"}, {"@BDG_C_BG_CH", "#2563eb"}
        };

    } else if (theme == Light) {
        iconPrefix += "light/icons";

        variables = {
            // --- Panel layers ---
            {"@BG_APP",        lightPalette["800"]},
            {"@BG_PANEL",      lightPalette["700"]},
            {"@BG_HEADER",     lightPalette["900"]},
            {"@BG_INPUT",      lightPalette["700"]},
            {"@BG_BTN",        lightPalette["900"]},
            {"@BG_BTN_HOVER",  lightPalette["600"]},
            {"@BG_BTN_PRESS",  lightPalette["500"]},
            {"@BG_SCROLL",     lightPalette["800"]},

            // --- Borders ---
            {"@BORDER_STD",    lightPalette["600"]},
            {"@BORDER_PANEL",  lightPalette["600"]},
            {"@BORDER_INPUT",  lightPalette["500"]},
            {"@BORDER_FOCUS",  lightAccentsPalette["Blue"]},
            {"@BORDER_LIGHT",  lightAccentsPalette["White 8"]},

            // --- Text ---
            {"@TEXT_MAIN",       lightPalette["White"]},
            {"@TEXT_SECONDARY",  lightPalette["100"]},
            {"@TEXT_LIGHT",      lightPalette["300"]},
            {"@TEXT_MUTED",      lightPalette["400"]},
            {"@TEXT_TERTIARY",   lightPalette["300"]},

            // --- Items ---
            {"@ITEM_SELECTED",  lightAccentsPalette["White 20"]},
            {"@ITEM_PRESS",     lightAccentsPalette["White 16"]},
            {"@ITEM_DISABLED",  lightAccentsPalette["White 10"]},

            // --- Accents ---
            {"@ACCENT_RED",       lightAccentsPalette["Red"]},
            {"@ACCENT_LRED",      lightAccentsPalette["Red Light"]},
            {"@ACCENT_ORANGE",    lightAccentsPalette["Orange"]},
            {"@ACCENT_BLUE",      lightAccentsPalette["Blue"]},
            {"@ACCENT_LBLUE",     lightAccentsPalette["Blue"]},
            {"@SELECTION_BLUE",   lightAccentsPalette["Selection Blue"]},
            {"@TABLE_HEADER",     lightPalette["600"]},

            // --- Badges ---
            {"@BDG_E_FG", lightAccentsPalette["Green"]},    {"@BDG_E_BG", "rgba(22, 163, 74, 15%)"},
            {"@BDG_P_FG", lightAccentsPalette["Purple"]},   {"@BDG_P_BG", "rgba(147, 51, 234, 15%)"},
            {"@BDG_O_FG", lightPalette["200"]},             {"@BDG_O_BG", "rgba(88, 88, 88, 15%)"},
            {"@BDG_X_FG", lightAccentsPalette["Red"]},      {"@BDG_X_BG", "rgba(220, 38, 38, 15%)"},
            {"@BDG_R_FG", lightAccentsPalette["Orange"]},   {"@BDG_R_BG", "rgba(217, 119, 6, 15%)"},
            {"@BDG_C_FG", lightAccentsPalette["Blue"]},     {"@BDG_C_BG", "rgba(0, 102, 204, 15%)"},

            // --- Badges - chessboard solid colors ---
            {"@BDG_E_FG_CH", "#FFFFFF"}, {"@BDG_E_BG_CH", lightAccentsPalette["Green"]},
            {"@BDG_P_FG_CH", "#FFFFFF"}, {"@BDG_P_BG_CH", lightAccentsPalette["Purple"]},
            {"@BDG_O_FG_CH", "#FFFFFF"}, {"@BDG_O_BG_CH", lightPalette["200"]},
            {"@BDG_X_FG_CH", "#FFFFFF"}, {"@BDG_X_BG_CH", lightAccentsPalette["Red"]},
            {"@BDG_R_FG_CH", "#FFFFFF"}, {"@BDG_R_BG_CH", lightAccentsPalette["Orange"]},
            {"@BDG_C_FG_CH", "#FFFFFF"}, {"@BDG_C_BG_CH", lightAccentsPalette["Blue"]}
        };
    }

    // Common properties
    variables.insert("@ICON_PREFIX", iconPrefix);
    variables.insert("@IMGS_PREFIX", ThemeController::imgsPath());

    return variables;
}

