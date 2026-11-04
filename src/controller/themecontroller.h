#ifndef THEMECONTROLLER_H
#define THEMECONTROLLER_H

#include <QString>
#include <QObject>
#include <QColor>
#include <QMap>

// Available themes
enum Theme {
    Dark = 0,
    Light = 1
};

static QString getThemeStr(Theme th){
    switch (th) {
    case Dark:
        return "dark";
        break;
    case Light:
        return "light";
        break;
    default:
        return "none";
        break;
    }
}

class ThemeController : public QObject {
    Q_OBJECT

public:

    // Singleton
    static ThemeController& getInstance();
    ThemeController(const ThemeController&) = delete;
    ThemeController& operator=(const ThemeController&) = delete;
    void applyTheme(Theme theme);

    static QString iconPath(){
        return QString(":/theme/").append(getThemeStr(getInstance().currentTheme)).append("/icons");
    }

    static QString imgsPath(){
        return QString(":/theme/").append(getThemeStr(getInstance().currentTheme)).append("/imgs");
    }

    // QColor of a variable of the current theme palette (e.g. "@BDG_E_BG_CH").
    // Returns an invalid QColor if the variable does not exist or is not a color.
    static QColor color(const QString &var);

private:
    ThemeController();
    ~ThemeController();

    QString * iconsLocation = nullptr;
    Theme currentTheme = Dark;
    QMap<QString, QColor> colorCache; // current theme palette but with QColors

    static QString loadAndProcessQss(const QString &path, Theme theme);
    static QMap<QString, QString> getPalette(Theme theme);

signals:
    void sigThemeUpdated();
};

#endif // THEMECONTROLLER_H
