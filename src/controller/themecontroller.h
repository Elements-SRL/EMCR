#ifndef THEMECONTROLLER_H
#define THEMECONTROLLER_H

#include <QString>
#include <QObject>

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


private:
    ThemeController();
    ~ThemeController();

    QString * iconsLocation = nullptr;
    Theme currentTheme = Dark;

    static QString loadAndProcessQss(const QString &path, Theme theme);
    static QMap<QString, QString> getPalette(Theme theme);
};

#endif // THEMECONTROLLER_H
