#include "themecontroller.h"
#include <QFile>
#include <QApplication>
#include <QMap>
#include <QSet>
#include <QDebug>
#include <QRegularExpression>

ThemeController::ThemeController() {}
ThemeController::~ThemeController() {}

// Singleton
ThemeController& ThemeController::getInstance() {
    static ThemeController instance;
    return instance;
}

void ThemeController::applyTheme(Theme theme) {
    this->currentTheme = theme;
    this->colorCache.clear();

    qApp->setPalette(buildPalette());

    QString qssPath = ":/styles/emcr_base.qss";
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

    // Every @VARIABLE must have been replaced.
    // Any leftover substitution muse be reported.
    static const QRegularExpression leftoverVar(QStringLiteral("@[A-Z][A-Z0-9_]*"));
    QSet<QString> leftovers;
    for (auto it = leftoverVar.globalMatch(qss); it.hasNext();) {
        leftovers.insert(it.next().captured());
    }
    if (!leftovers.isEmpty()) {
        QStringList names(leftovers.begin(), leftovers.end());
        names.sort();
        qWarning().noquote() << "ThemeController: QSS variables not defined for the"
                             << getThemeStr(theme) << "theme:" << names.join(", ");
    }

    return qss;
}

// =============================================================================
// Two levels of colors:
// - primitives (below): the raw colors. Never used by the QSS.
// - theme variables (darkTheme(), lightTheme()): one role each. The QSS and
//   ThemeController::color() only use these.
// =============================================================================

// NEUTRALS
// One base scale shared by every theme: 50 is the lightest, 950 the darkest.
// Dark themes take the surfaces from the dark end and the text from the light
// end, light themes the opposite: the two themes are the same app inverted.
static const QMap<QString, QString> basePalette = {
    {"50",  "#F7F9FC"},
    {"100", "#EDF0F4"},
    {"150", "#DEE3EA"},
    {"200", "#CDD4DF"},
    {"250", "#C3CBD7"},
    {"300", "#B4BDCB"},
    {"400", "#8E99AE"},
    {"500", "#717D96"},
    {"600", "#5A657C"},
    {"700", "#4A5468"},
    {"750", "#3D475A"},
    {"800", "#2D3648"},
    {"900", "#1A202C"},
    {"950", "#11161F"}
};

// ACCENTS
// Per theme: on light surfaces the same hue must be darker to stay readable.
static const QMap<QString, QString> darkAccentsPalette = {
    {"Blue",           "#4DA3FF"},
    {"Selection Blue", "#24507D"},
    {"Orange",         "#FE9D35"},
    {"Red",            "rgb(255, 82, 82)"},
    {"Red 70",         "rgba(255, 82, 82, 70%)"},

    // White overlays, for hover/press on any dark surface
    {"Overlay 20", "rgba(255, 255, 255, 20%)"},
    {"Overlay 10", "rgba(255, 255, 255, 10%)"},
    {"Overlay 8",  "rgba(255, 255, 255, 8%)"}
};

static const QMap<QString, QString> lightAccentsPalette = {
    {"Blue",           "#0B5CAD"},
    {"Selection Blue", "#B5CFEC"},
    {"Orange",         "#C2570C"},
    {"Amber",          "#B45309"},
    {"Amber Text",     "#92400E"},  // darker, for text on the light surfaces
    {"Green",          "#15803D"},
    {"Green Text",     "#166534"},  // darker, for text on the light surfaces
    {"Purple",         "#7E22CE"},
    {"Red",            "#B91C1C"},
    {"Red 15",         "rgba(185, 28, 28, 15%)"},

    // Slate 900 overlays, for hover/press on any light surface
    {"Overlay 14", "rgba(26, 32, 44, 14%)"},
    {"Overlay 12", "rgba(26, 32, 44, 12%)"},
    {"Overlay 10", "rgba(26, 32, 44, 10%)"},
    {"Overlay 8",  "rgba(26, 32, 44, 8%)"},
    {"Overlay 5",  "rgba(26, 32, 44, 5%)"}
};

namespace {

// Primitive lookups. A missing key is a typo in this file: caught in debug builds.
QString pick(const QMap<QString, QString> &scale, const char *key) {
    Q_ASSERT_X(scale.contains(QLatin1String(key)), "ThemeController", key);
    return scale.value(QLatin1String(key));
}
QString base(const char *key)       { return pick(basePalette, key); }
QString darkAccent(const char *key)  { return pick(darkAccentsPalette, key); }
QString lightAccent(const char *key) { return pick(lightAccentsPalette, key); }

// =============================================================================
// THEMES
// One dictionary per theme, with the same variables in the same order (keep
// them aligned: two themes can then be compared side by side in a diff).
// Names describe the role, never the color: the same name means the same thing
// in every theme. getPalette() warns if a theme misses a variable of the dark
// one, or has one more.
// =============================================================================

// DARK THEME
QMap<QString, QString> darkTheme() {
    return {
        // --- Surfaces: window, panels, section headers, inset areas (fields, plots, tables), popups ---
        {"@SURFACE_APP",       base("900")},
        {"@SURFACE_PANEL",     base("800")},
        {"@SURFACE_HEADER",    base("750")},
        {"@SURFACE_INSET",     base("900")},
        {"@SURFACE_POPUP",     base("800")},
        {"@TOOLTIP_BG",        base("700")},
        {"@TOOLTIP_TEXT",      base("50")},
        {"@TABLE_HEADER_BG",   "#2B313C"},
        {"@TABLE_HEADER_TEXT", base("50")},

        // --- Text, from strongest to weakest, plus text with a role ---
        {"@TEXT_STRONG",    base("50")},
        {"@TEXT_DEFAULT",   base("150")},
        {"@TEXT_SUBTLE",    base("200")},
        {"@TEXT_CAPTION",   base("400")},
        {"@TEXT_INACTIVE",  base("400")},
        {"@TEXT_DISABLED",  base("500")},
        {"@TEXT_VALUE",     base("150")},
        {"@TEXT_HIGHLIGHT", darkAccent("Blue")},
        {"@TEXT_LINK",      darkAccent("Blue")},
        {"@TEXT_ON_ACCENT", base("800")},
        {"@TEXT_ON_DANGER", base("200")},
        {"@SPLASH_TITLE",   "#FFFFFF"},

        // --- Buttons and other clickable controls ---
        {"@CONTROL_BG",             base("700")},
        {"@CONTROL_BG_HOVER",       base("500")},
        {"@CONTROL_BG_PRESS",       base("900")},
        {"@CONTROL_BG_DISABLED",    darkAccent("Overlay 20")},
        {"@CONTROL_BORDER",         base("700")},
        {"@CONTROL_BORDER_HOVER",   base("500")},
        {"@CONTROL_BORDER_PRESS",   darkAccent("Orange")},
        {"@CONTROL_BORDER_CHECKED", darkAccent("Blue")},
        {"@WINDOW_BUTTON_HOVER_BG", base("700")},
        {"@COUNTER_OFF_BG",         base("700")},

        // --- Editable fields (line edits, spin boxes, combo boxes) and check/radio indicators ---
        {"@FIELD_BG",           base("900")},
        {"@FIELD_BG_FOCUS",     "rgba(0, 0, 0, 0.25)"},
        {"@FIELD_BORDER",       base("500")},
        {"@FIELD_BORDER_FOCUS", darkAccent("Orange")},
        {"@INDICATOR_CHECKED",  darkAccent("Blue")},

        // --- Hover / selection of rows and menu items ---
        {"@ROW_HOVER_BG",       base("500")},
        {"@ROW_SELECTED_BG",    "#3A404B"}, // neutral grey: the blue was too strong on dark tables
        {"@STATE_HOVER",        darkAccent("Overlay 20")},
        {"@STATE_PRESS",        darkAccent("Overlay 10")},
        {"@STATE_HOVER_SUBTLE", "rgba(255, 255, 255, 0.05)"},

        // --- Borders, separators, scroll bars, splitters, sliders ---
        {"@BORDER_DEFAULT",      base("700")},
        {"@BORDER_SUBTLE",       base("750")},
        {"@BORDER_DISABLED",     base("700")},
        {"@SEPARATOR",           base("700")},
        {"@SEPARATOR_HOVER",     base("500")},
        {"@MENU_BORDER",         darkAccent("Overlay 8")},
        {"@POPUP_BORDER",        "rgba(255, 255, 255, 8)"},
        {"@SPLITTER",            darkAccent("Overlay 8")},
        {"@SPLITTER_HOVER",      darkAccent("Blue")},
        {"@SCROLL_TRACK",        base("800")},
        {"@SCROLL_HANDLE",       base("500")},
        {"@SCROLL_HANDLE_HOVER", darkAccent("Blue")},
        {"@SCROLL_SHADOW",       "rgba(0, 0, 0, 0.75)"},
        {"@SLIDER_HANDLE",       darkAccent("Orange")},

        // --- Accent (selection) and danger ---
        {"@ACCENT",       darkAccent("Blue")},
        {"@ACCENT_BG",    darkAccent("Selection Blue")},
        {"@DANGER",       darkAccent("Red")},
        {"@DANGER_HOVER", darkAccent("Red 70")},
        {"@RECORD",       darkAccent("Red")},

        // --- Status: notifications, on/mixed labels ---
        {"@STATUS_OK",      "#4ade80"},
        {"@STATUS_OK_BG",   "rgba(74, 222, 128, 15%)"},
        {"@STATUS_INFO",    darkAccent("Blue")},
        {"@STATUS_WARN",    darkAccent("Orange")},
        {"@STATUS_WARN_BG", "rgba(226, 158, 46, 0.2)"},
        {"@STATUS_ERROR",   darkAccent("Red")},

        // --- Channel property badges (E, P, O, X, R, C) ---
        {"@BDG_E_FG", "#4ade80"},
        {"@BDG_E_BG", "rgba(74, 222, 128, 15%)"},
        {"@BDG_P_FG", "#a78bfa"},
        {"@BDG_P_BG", "rgba(167, 139, 250, 15%)"},
        {"@BDG_O_FG", "#a1a1aa"},
        {"@BDG_O_BG", "rgba(161, 161, 170, 15%)"},
        {"@BDG_X_FG", "#f87171"},
        {"@BDG_X_BG", "rgba(248, 113, 113, 15%)"},
        {"@BDG_R_FG", "#fbbf24"},
        {"@BDG_R_BG", "rgba(251, 191, 36, 15%)"},
        {"@BDG_C_FG", "#fafafa"},
        {"@BDG_C_BG", "rgba(124, 58, 237, 20%)"},

        // --- Channel property badges, chessboard solid colors (painted from C++) ---
        {"@BDG_E_FG_CH", "#FFFFFF"},
        {"@BDG_E_BG_CH", "#16a34a"},
        {"@BDG_P_FG_CH", "#FFFFFF"},
        {"@BDG_P_BG_CH", "#7c3aed"},
        {"@BDG_O_FG_CH", "#FFFFFF"},
        {"@BDG_O_BG_CH", "#4b5563"},
        {"@BDG_X_FG_CH", "#FFFFFF"},
        {"@BDG_X_BG_CH", "#dc2626"},
        {"@BDG_R_FG_CH", "#1e293b"},
        {"@BDG_R_BG_CH", "#eab308"},
        {"@BDG_C_FG_CH", "#FFFFFF"},
        {"@BDG_C_BG_CH", "#2563eb"},

        // --- Temperature sensor badges ---
        {"@BDG_NOR_FG", "#4ade80"},
        {"@BDG_NOR_BG", "rgba(74, 222, 128, 15%)"},
        {"@BDG_WAR_FG", "#fbbf24"},
        {"@BDG_WAR_BG", "rgba(251, 191, 36, 15%)"},
        {"@BDG_DNG_FG", "#f87171"},
        {"@BDG_DNG_BG", "rgba(248, 113, 113, 15%)"},

        // --- Chessboard mini-plots (cards, overlay, curve) ---
        {"@BG_STAMP",        base("800")},
        {"@BG_STAMP_SEL",    "#283E5B"},
        {"@BORDER_STAMP",    base("700")},
        {"@STAMP_GRID",      base("750")},
        {"@STAMP_CURVE",     base("200")},
        {"@STAMP_NAME_BG",   base("50")},
        {"@STAMP_NAME_TEXT", base("900")},

        // --- Plots ---
        {"@PLOT_BG",   base("900")},
        {"@PLOT_GRID", "#3A4458"},

        // --- Timer (LCD) ---
        {"@LCD_BG",          base("900")},
        {"@LCD_FG",          base("150")},
        {"@LCD_FG_DISABLED", base("500")},
        {"@LCD_BORDER",      base("750")}

    };
}

// LIGHT THEME
QMap<QString, QString> lightTheme() {
    return {
        // --- Surfaces: window, panels, section headers, inset areas (fields, plots, tables), popups ---
        {"@SURFACE_APP",       base("250")},
        {"@SURFACE_PANEL",     base("150")},
        {"@SURFACE_HEADER",    base("200")},
        {"@SURFACE_INSET",     base("100")},
        {"@SURFACE_POPUP",     base("100")},
        {"@TOOLTIP_BG",        base("800")},
        {"@TOOLTIP_TEXT",      base("50")},
        {"@TABLE_HEADER_BG",   base("200")},
        {"@TABLE_HEADER_TEXT", base("900")},

        // --- Text, from strongest to weakest, plus text with a role ---
        {"@TEXT_STRONG",    base("900")},
        {"@TEXT_DEFAULT",   base("800")},
        {"@TEXT_SUBTLE",    base("700")},
        {"@TEXT_CAPTION",   base("700")},
        {"@TEXT_INACTIVE",  base("700")},
        {"@TEXT_DISABLED",  base("400")},
        {"@TEXT_VALUE",     base("800")},
        {"@TEXT_HIGHLIGHT", lightAccent("Blue")},
        {"@TEXT_LINK",      lightAccent("Blue")},
        {"@TEXT_ON_ACCENT", base("50")},
        {"@TEXT_ON_DANGER", base("50")},
        {"@SPLASH_TITLE",   base("900")},

        // --- Buttons and other clickable controls ---
        {"@CONTROL_BG",             base("200")},
        {"@CONTROL_BG_HOVER",       base("250")},
        {"@CONTROL_BG_PRESS",       base("300")},
        {"@CONTROL_BG_DISABLED",    base("150")},
        {"@CONTROL_BORDER",         base("500")},
        {"@CONTROL_BORDER_HOVER",   base("600")},
        {"@CONTROL_BORDER_PRESS",   lightAccent("Orange")},
        {"@CONTROL_BORDER_CHECKED", lightAccent("Blue")},
        {"@WINDOW_BUTTON_HOVER_BG", base("300")},
        {"@COUNTER_OFF_BG",         base("200")},

        // --- Editable fields (line edits, spin boxes, combo boxes) and check/radio indicators ---
        {"@FIELD_BG",           base("100")},
        {"@FIELD_BG_FOCUS",     base("50")},
        {"@FIELD_BORDER",       base("500")},
        {"@FIELD_BORDER_FOCUS", lightAccent("Orange")},
        {"@INDICATOR_CHECKED",  lightAccent("Blue")},

        // --- Hover / selection of rows and menu items ---
        {"@ROW_HOVER_BG",       base("200")},
        {"@ROW_SELECTED_BG",    lightAccent("Selection Blue")},
        {"@STATE_HOVER",        lightAccent("Overlay 8")},
        {"@STATE_PRESS",        lightAccent("Overlay 12")},
        {"@STATE_HOVER_SUBTLE", lightAccent("Overlay 5")},

        // --- Borders, separators, scroll bars, splitters, sliders ---
        {"@BORDER_DEFAULT",      base("300")},
        {"@BORDER_SUBTLE",       base("300")},
        {"@BORDER_DISABLED",     base("300")},
        {"@SEPARATOR",           base("300")},
        {"@SEPARATOR_HOVER",     base("300")},
        {"@MENU_BORDER",         base("300")},
        {"@POPUP_BORDER",        base("300")},
        {"@SPLITTER",            lightAccent("Overlay 10")},
        {"@SPLITTER_HOVER",      lightAccent("Blue")},
        {"@SCROLL_TRACK",        base("150")},
        {"@SCROLL_HANDLE",       base("400")},
        {"@SCROLL_HANDLE_HOVER", lightAccent("Blue")},
        {"@SCROLL_SHADOW",       "rgba(26, 32, 44, 0.35)"},
        {"@SLIDER_HANDLE",       lightAccent("Orange")},

        // --- Accent (selection) and danger ---
        {"@ACCENT",       lightAccent("Blue")},
        {"@ACCENT_BG",    lightAccent("Selection Blue")},
        {"@DANGER",       lightAccent("Red")},
        {"@DANGER_HOVER", lightAccent("Red 15")},
        {"@RECORD",       lightAccent("Red")},

        // --- Status: notifications, on/mixed labels ---
        {"@STATUS_OK",      lightAccent("Green Text")},
        {"@STATUS_OK_BG",   "rgba(22, 101, 52, 15%)"},
        {"@STATUS_INFO",    lightAccent("Blue")},
        {"@STATUS_WARN",    lightAccent("Amber Text")},
        {"@STATUS_WARN_BG", "rgba(146, 64, 14, 0.15)"},
        {"@STATUS_ERROR",   lightAccent("Red")},

        // --- Channel property badges (E, P, O, X, R, C) ---
        {"@BDG_E_FG", lightAccent("Green Text")},
        {"@BDG_E_BG", "rgba(22, 101, 52, 15%)"},
        {"@BDG_P_FG", lightAccent("Purple")},
        {"@BDG_P_BG", "rgba(126, 34, 206, 15%)"},
        {"@BDG_O_FG", base("700")},
        {"@BDG_O_BG", "rgba(74, 84, 104, 15%)"},
        {"@BDG_X_FG", lightAccent("Red")},
        {"@BDG_X_BG", "rgba(185, 28, 28, 15%)"},
        {"@BDG_R_FG", lightAccent("Amber Text")},
        {"@BDG_R_BG", "rgba(146, 64, 14, 15%)"},
        {"@BDG_C_FG", lightAccent("Blue")},
        {"@BDG_C_BG", "rgba(11, 92, 173, 15%)"},

        // --- Channel property badges, chessboard solid colors (painted from C++) ---
        // Light: white text on every badge >= 4.5:1
        {"@BDG_E_FG_CH", "#FFFFFF"},
        {"@BDG_E_BG_CH", lightAccent("Green")},
        {"@BDG_P_FG_CH", "#FFFFFF"},
        {"@BDG_P_BG_CH", lightAccent("Purple")},
        {"@BDG_O_FG_CH", "#FFFFFF"},
        {"@BDG_O_BG_CH", base("700")},
        {"@BDG_X_FG_CH", "#FFFFFF"},
        {"@BDG_X_BG_CH", lightAccent("Red")},
        {"@BDG_R_FG_CH", "#FFFFFF"},
        {"@BDG_R_BG_CH", lightAccent("Amber")},
        {"@BDG_C_FG_CH", "#FFFFFF"},
        {"@BDG_C_BG_CH", lightAccent("Blue")},

        // --- Temperature sensor badges ---
        {"@BDG_NOR_FG", lightAccent("Green Text")},
        {"@BDG_NOR_BG", "rgba(22, 101, 52, 15%)"},
        {"@BDG_WAR_FG", lightAccent("Amber Text")},
        {"@BDG_WAR_BG", "rgba(146, 64, 14, 15%)"},
        {"@BDG_DNG_FG", lightAccent("Red")},
        {"@BDG_DNG_BG", "rgba(185, 28, 28, 15%)"},

        // --- Chessboard mini-plots (cards, overlay, curve) ---
        {"@BG_STAMP",        base("150")},
        {"@BG_STAMP_SEL",    "#D6E4F5"},
        {"@BORDER_STAMP",    base("400")},
        {"@STAMP_GRID",      base("200")},
        {"@STAMP_CURVE",     base("500")},
        {"@STAMP_NAME_BG",   base("50")},
        {"@STAMP_NAME_TEXT", base("900")},

        // --- Plots ---
        {"@PLOT_BG",   base("100")},
        {"@PLOT_GRID", base("200")},

        // --- Timer (LCD) ---
        {"@LCD_BG",          base("100")},
        {"@LCD_FG",          base("800")},
        {"@LCD_FG_DISABLED", base("400")},
        {"@LCD_BORDER",      base("300")}

    };
}

} // namespace

QMap<QString, QString> ThemeController::getPalette(Theme theme) {
    QMap<QString, QString> variables;
    switch (theme) {
    case Dark:
        variables = darkTheme();
        break;
    case Light:
        variables = lightTheme();
        break;
    default:
        qWarning() << "ThemeController: unknown theme" << theme;
        return variables;
    }

    // Every theme must define the same variables as the dark one
    if (theme != Dark) {
        const QMap<QString, QString> reference = darkTheme();
        for (auto it = reference.cbegin(); it != reference.cend(); ++it) {
            if (!variables.contains(it.key())) {
                qWarning().noquote() << "ThemeController:" << it.key() << "is missing in the" << getThemeStr(theme) << "theme";
            }
        }
        for (auto it = variables.cbegin(); it != variables.cend(); ++it) {
            if (!reference.contains(it.key())) {
                qWarning().noquote() << "ThemeController:" << it.key() << "is not defined in the dark theme";
            }
        }
    }

    // Common properties
    const QString themeRoot = QStringLiteral(":/theme/") + getThemeStr(theme);
    variables.insert("@ICON_PREFIX", themeRoot + "/icons");
    variables.insert("@IMGS_PREFIX", themeRoot + "/imgs");

    return variables;
}


/*  Application palette from the same variables: it covers what the QSS
 *  does not reach (placeholder text, links in rich text, standard dialogs)
 */
QPalette ThemeController::buildPalette() {
    QPalette palette;
    auto set = [&palette](QPalette::ColorRole role, const char *var,
                          QPalette::ColorGroup group = QPalette::All) {
        palette.setColor(group, role, color(QLatin1String(var)));
    };
    set(QPalette::Window,          "@SURFACE_APP");
    set(QPalette::WindowText,      "@TEXT_DEFAULT");
    set(QPalette::Base,            "@FIELD_BG");
    set(QPalette::AlternateBase,   "@SURFACE_PANEL");
    set(QPalette::Text,            "@TEXT_DEFAULT");
    set(QPalette::Button,          "@CONTROL_BG");
    set(QPalette::ButtonText,      "@TEXT_DEFAULT");
    set(QPalette::BrightText,      "@TEXT_STRONG");
    set(QPalette::Highlight,       "@ACCENT_BG");
    set(QPalette::HighlightedText, "@TEXT_STRONG");
    set(QPalette::ToolTipBase,     "@TOOLTIP_BG");
    set(QPalette::ToolTipText,     "@TOOLTIP_TEXT");
    set(QPalette::PlaceholderText, "@TEXT_DISABLED");
    set(QPalette::Link,            "@TEXT_LINK");
    set(QPalette::LinkVisited,     "@TEXT_LINK");
    for (QPalette::ColorRole role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText}) {
        set(role, "@TEXT_DISABLED", QPalette::Disabled);
    }
    return palette;
}

// Converts colors into QColor. Used by QPainter which needs theme colors via code.
// Manages ("#RRGGBB", SVG names) also "rgb(r, g, b)" and "rgba(r, g, b, a)" as QSS does,
// with alpha channel support like ("70%"), in 0..1 ("0.7") or based in 0..255.
static QColor parseQssColor(const QString &value) {
    const QString s = value.trimmed();
    if (s.startsWith(QLatin1String("rgb"), Qt::CaseInsensitive)) {
        const int open = s.indexOf('(');
        const int close = s.lastIndexOf(')');
        if (open < 0 || close <= open) {
            return QColor();
        }
        const QStringList parts = s.mid(open + 1, close - open - 1).split(',');
        if (parts.size() < 3) {
            return QColor();
        }
        auto component = [](QString part, bool isAlpha) -> int {
            part = part.trimmed();
            bool ok = false;
            if (part.endsWith('%')) {
                const double pct = part.chopped(1).trimmed().toDouble(&ok);
                return ok ? qBound(0, qRound(pct * 255.0 / 100.0), 255) : -1;
            }
            const double v = part.toDouble(&ok);
            if (!ok) {
                return -1;
            }
            if (isAlpha && part.contains('.') && v <= 1.0) {
                return qBound(0, qRound(v * 255.0), 255);
            }
            return qBound(0, qRound(v), 255);
        };
        const int r = component(parts[0], false);
        const int g = component(parts[1], false);
        const int b = component(parts[2], false);
        const int a = parts.size() >= 4 ? component(parts[3], true) : 255;
        if (r < 0 || g < 0 || b < 0 || a < 0) {
            return QColor();
        }
        return QColor(r, g, b, a);
    }
    return QColor::fromString(s);
}

QColor ThemeController::color(const QString &var) {
    ThemeController &self = getInstance();
    if (self.colorCache.isEmpty()) {
        const QMap<QString, QString> palette = getPalette(self.currentTheme);
        for (auto it = palette.cbegin(); it != palette.cend(); ++it) {
            const QColor c = parseQssColor(it.value());
            if (c.isValid()) {
                self.colorCache.insert(it.key(), c);
            }
        }
    }
    const auto it = self.colorCache.constFind(var);
    if (it == self.colorCache.constEnd()) {
        qWarning().noquote() << "ThemeController::color: unknown variable" << var;
        return QColor();
    }
    return it.value();
}
