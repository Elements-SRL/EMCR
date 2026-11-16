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
// - primitives (below): the raw scales of each theme. Never used by the QSS.
// - theme variables (themeVariables()): one role each, with its value in both
//   themes. The QSS and ThemeController::color() only use these.
// =============================================================================

// DARK PALETTE
// Palette is monochrome with different numeric shades (tonality):
// 900 darker - 100 almost white
static const QMap<QString, QString> darkPalette = {
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

static const QMap<QString, QString> darkAccentsPalette = {
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
static const QMap<QString, QString> lightPalette = {
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

static const QMap<QString, QString> lightAccentsPalette = {
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

namespace {

// Primitive lookups. A missing key is a typo in this file: caught in debug builds.
QString pick(const QMap<QString, QString> &scale, const char *key) {
    Q_ASSERT_X(scale.contains(QLatin1String(key)), "ThemeController", key);
    return scale.value(QLatin1String(key));
}
QString dark(const char *key)        { return pick(darkPalette, key); }
QString darkAccent(const char *key)  { return pick(darkAccentsPalette, key); }
QString light(const char *key)       { return pick(lightPalette, key); }
QString lightAccent(const char *key) { return pick(lightAccentsPalette, key); }

// One value per theme, in the order of the Theme enum (Dark, Light, ...).
// A new theme = a new enum entry + one more value on every row: a row left
// without it does not compile.
struct ThemeVariable {
    template <typename... Values>
    ThemeVariable(const char *variableName, const Values &... themeValues) :
        name(variableName), values{QString(themeValues)...} {
        static_assert(sizeof...(Values) == ThemeCount, "one value per theme (see enum Theme)");
    }

    const char *name;
    QString values[ThemeCount];
};

// Names describe the role, never the color: the same name means the same
// thing in every theme.
const QList<ThemeVariable> &themeVariables() {
    static const QList<ThemeVariable> variables = {
            // --- Surfaces: window, panels, section headers, inset areas (tables, segmented tracks), popups ---
            {"@SURFACE_APP",       dark("900"),                light("800")},
            {"@SURFACE_PANEL",     dark("800"),                light("700")},
            {"@SURFACE_HEADER",    dark("700"),                light("900")},
            {"@SURFACE_INSET",     dark("900"),                light("800")},
            {"@SURFACE_POPUP",     dark("800"),                light("700")},
            {"@TOOLTIP_BG",        dark("700"),                light("900")},
            {"@TOOLTIP_TEXT",      dark("100"),                light("300")},
            {"@TABLE_HEADER_BG",   darkAccent("Table header"), light("600")},
            {"@TABLE_HEADER_TEXT", dark("100"),                light("300")},

            // --- Text, from strongest to weakest, plus text with a role ---
            {"@TEXT_STRONG",    dark("100"),              light("300")},
            {"@TEXT_DEFAULT",   dark("200"),              light("White")},
            {"@TEXT_SUBTLE",    dark("400"),              light("100")},
            {"@TEXT_CAPTION",   dark("600"),              light("300")},
            {"@TEXT_INACTIVE",  dark("500"),              light("400")},
            {"@TEXT_DISABLED",  dark("500"),              light("400")},
            {"@TEXT_VALUE",     darkAccent("Orange"),     lightAccent("Orange")},
            {"@TEXT_HIGHLIGHT", darkAccent("Blue Light"), lightAccent("Blue")},
            {"@TEXT_LINK",      darkAccent("Blue"),       lightAccent("Blue")},
            {"@TEXT_ON_ACCENT", dark("800"),              light("700")},
            {"@TEXT_ON_DANGER", dark("400"),              light("100")},
            {"@SPLASH_TITLE",   dark("White"),            light("White")},

            // --- Buttons and other clickable controls ---
            {"@CONTROL_BG",             dark("700"),              light("900")},
            {"@CONTROL_BG_HOVER",       dark("600"),              light("600")},
            {"@CONTROL_BG_PRESS",       dark("900"),              light("500")},
            {"@CONTROL_BG_DISABLED",    darkAccent("White 20"),   lightAccent("White 10")},
            {"@CONTROL_BORDER",         dark("700"),              light("500")},
            {"@CONTROL_BORDER_HOVER",   dark("600"),              lightAccent("Blue")},
            {"@CONTROL_BORDER_PRESS",   darkAccent("Orange"),     lightAccent("Orange")},
            {"@CONTROL_BORDER_CHECKED", darkAccent("Blue Light"), lightAccent("Blue")},
            {"@WINDOW_BUTTON_HOVER_BG", dark("700"),              light("600")},
            {"@COUNTER_OFF_BG",         dark("700"),              light("900")},

            // --- Editable fields (line edits, spin boxes, combo boxes) and check/radio indicators ---
            {"@FIELD_BG",           dark("900"),              light("700")},
            {"@FIELD_BG_FOCUS",     "rgba(0, 0, 0, 0.25)",    light("700")},
            {"@FIELD_BORDER",       dark("700"),              light("500")},
            {"@FIELD_BORDER_FOCUS", dark("600"),              lightAccent("Blue")},
            {"@INDICATOR_CHECKED",  darkAccent("Blue Light"), lightAccent("Blue")},

            // --- Hover / selection of rows and menu items ---
            {"@ROW_HOVER_BG",       dark("600"),                 light("600")},
            {"@ROW_SELECTED_BG",    dark("600"),                 light("600")},
            {"@STATE_HOVER",        darkAccent("White 20"),      lightAccent("White 20")},
            {"@STATE_SELECTED",     darkAccent("White 20"),      lightAccent("White 20")},
            {"@STATE_PRESS",        darkAccent("White 10"),      lightAccent("White 16")},
            {"@STATE_HOVER_SUBTLE", "rgba(255, 255, 255, 0.05)", "rgba(0, 0, 0, 0.05)"},

            // --- Borders, separators, scroll bars, splitters, sliders ---
            {"@BORDER_DEFAULT",      dark("700"),              light("600")},
            {"@BORDER_SUBTLE",       dark("800"),              light("600")},
            {"@BORDER_DISABLED",     dark("800"),              light("600")},
            {"@SEPARATOR",           dark("700"),              light("500")},
            {"@SEPARATOR_HOVER",     dark("600"),              light("600")},
            {"@MENU_BORDER",         darkAccent("White 8"),    lightAccent("White 8")},
            {"@POPUP_BORDER",        "rgba(255, 255, 255, 8)", light("500")},
            {"@SPLITTER",            darkAccent("White 8"),    lightAccent("White 8")},
            {"@SPLITTER_HOVER",      darkAccent("Blue Light"), lightAccent("Blue")},
            {"@SCROLL_TRACK",        dark("800"),              light("700")},
            {"@SCROLL_HANDLE",       dark("600"),              light("600")},
            {"@SCROLL_HANDLE_HOVER", darkAccent("Blue Light"), lightAccent("Blue")},
            {"@SLIDER_HANDLE",       darkAccent("Orange"),     lightAccent("Orange")},

            // --- Accent (selection) and danger ---
            {"@ACCENT",       darkAccent("Blue"),           lightAccent("Blue")},
            {"@ACCENT_BG",    darkAccent("Selection Blue"), lightAccent("Selection Blue")},
            {"@DANGER",       darkAccent("Red"),            lightAccent("Red")},
            {"@DANGER_HOVER", darkAccent("Red Light"),      lightAccent("Red Light")},
            {"@RECORD",       darkAccent("Red"),            lightAccent("Red")},

            // --- Status: notifications, on/mixed labels ---
            {"@STATUS_OK",      "#4ade80",                 lightAccent("Green")},
            {"@STATUS_OK_BG",   "rgba(74, 222, 128, 15%)", "rgba(22, 163, 74, 15%)"},
            {"@STATUS_INFO",    darkAccent("Blue Light"),  lightAccent("Blue")},
            {"@STATUS_WARN",    darkAccent("Orange"),      lightAccent("Orange")},
            {"@STATUS_WARN_BG", "rgba(226, 158, 46, 0.2)", "rgba(226, 158, 46, 0.2)"},
            {"@STATUS_ERROR",   darkAccent("Red"),         lightAccent("Red")},
            
            // --- Channel property badges (E, P, O, X, R, C) ---
            {"@BDG_E_FG", "#4ade80",                  lightAccent("Green")},
            {"@BDG_E_BG", "rgba(74, 222, 128, 15%)",  "rgba(22, 163, 74, 15%)"},
            {"@BDG_P_FG", "#a78bfa",                  lightAccent("Purple")},
            {"@BDG_P_BG", "rgba(167, 139, 250, 15%)", "rgba(147, 51, 234, 15%)"},
            {"@BDG_O_FG", "#a1a1aa",                  light("200")},
            {"@BDG_O_BG", "rgba(161, 161, 170, 15%)", "rgba(88, 88, 88, 15%)"},
            {"@BDG_X_FG", "#f87171",                  lightAccent("Red")},
            {"@BDG_X_BG", "rgba(248, 113, 113, 15%)", "rgba(220, 38, 38, 15%)"},
            {"@BDG_R_FG", "#fbbf24",                  lightAccent("Orange")},
            {"@BDG_R_BG", "rgba(251, 191, 36, 15%)",  "rgba(217, 119, 6, 15%)"},
            {"@BDG_C_FG", "#fafafa",                  lightAccent("Blue")},
            {"@BDG_C_BG", "rgba(124, 58, 237, 20%)",  "rgba(0, 102, 204, 15%)"},

            // --- Channel property badges, chessboard solid colors (painted from C++) ---
            {"@BDG_E_FG_CH", "#FFFFFF", "#FFFFFF"}, {"@BDG_E_BG_CH", "#16a34a", lightAccent("Green")},
            {"@BDG_P_FG_CH", "#FFFFFF", "#FFFFFF"}, {"@BDG_P_BG_CH", "#7c3aed", lightAccent("Purple")},
            {"@BDG_O_FG_CH", "#FFFFFF", "#FFFFFF"}, {"@BDG_O_BG_CH", "#4b5563", light("200")},
            {"@BDG_X_FG_CH", "#FFFFFF", "#FFFFFF"}, {"@BDG_X_BG_CH", "#dc2626", lightAccent("Red")},
            {"@BDG_R_FG_CH", "#1e293b", "#FFFFFF"}, {"@BDG_R_BG_CH", "#eab308", lightAccent("Orange")},
            {"@BDG_C_FG_CH", "#FFFFFF", "#FFFFFF"}, {"@BDG_C_BG_CH", "#2563eb", lightAccent("Blue")},

            // --- Temperature sensor badges ---
            {"@BDG_NOR_FG", "#4ade80",                  lightAccent("Green")},
            {"@BDG_NOR_BG", "rgba(74, 222, 128, 15%)",  "rgba(22, 163, 74, 15%)"},
            {"@BDG_WAR_FG", "#fbbf24",                  lightAccent("Orange")},
            {"@BDG_WAR_BG", "rgba(251, 191, 36, 15%)",  "rgba(217, 119, 6, 15%)"},
            {"@BDG_DNG_FG", "#f87171",                  lightAccent("Red")},
            {"@BDG_DNG_BG", "rgba(248, 113, 113, 15%)", "rgba(220, 38, 38, 15%)"},

            // --- Chessboard mini-plots (cards, overlay, curve) ---
            {"@BG_STAMP",        "#11161F",   "#E3E8EE"},
            {"@BG_STAMP_SEL",    "#3A4A66",   "#CFE0F3"},
            {"@BORDER_STAMP",    dark("700"), light("500")},
            {"@STAMP_GRID",      "#2A3344",   "#CBD3DC"},
            {"@STAMP_CURVE",     "#CBD2E0",   light("100")},
            {"@STAMP_NAME_BG",   dark("100"), light("300")},
            {"@STAMP_NAME_TEXT", dark("900"), light("800")},

            // --- Plots ---
            {"@PLOT_BG",   dark("900"), light("800")},
            {"@PLOT_GRID", "#3A4458",   "#C0C0C0"},

            // --- Timer (LCD) ---
            {"@LCD_BG",          "#CBD2E0", "#CBD2E0"},
            {"@LCD_FG",          "#2C353F", "#2C353F"},
            {"@LCD_FG_DISABLED", "#8A94A0", "#8A94A0"}
    };
    return variables;
}

} // namespace

QMap<QString, QString> ThemeController::getPalette(Theme theme) {
    QMap<QString, QString> variables;
    if (theme < 0 || theme >= ThemeCount) {
        qWarning() << "ThemeController: unknown theme" << theme;
        return variables;
    }
    for (const ThemeVariable &v : themeVariables()) {
        variables.insert(QLatin1String(v.name), v.values[theme]);
    }

    // Common properties
    const QString themeRoot = QStringLiteral(":/theme/") + getThemeStr(theme);
    variables.insert("@ICON_PREFIX", themeRoot + "/icons");
    variables.insert("@IMGS_PREFIX", themeRoot + "/imgs");

    return variables;
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
