#ifndef THEMECONTROLLER_H
#define THEMECONTROLLER_H

#include <QString>
#include <QObject>

class ThemeController : public QObject {
    Q_OBJECT

public:
    // Available themes
    enum Theme {
        Dark = 0,
        Light = 1
    };

    // Singleton
    static ThemeController& getInstance();
    ThemeController(const ThemeController&) = delete;
    ThemeController& operator=(const ThemeController&) = delete;

    void applyTheme(Theme theme);

private:
    ThemeController();
    ~ThemeController();

    static QString loadAndProcessQss(const QString &path, Theme theme);
    static QMap<QString, QString> getPalette(Theme theme);
};

#endif // THEMECONTROLLER_H
