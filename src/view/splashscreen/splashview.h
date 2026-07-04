#ifndef SPLASHVIEW_H
#define SPLASHVIEW_H

#include "qlabel.h"
#include <QWidget>
#include <QSvgWidget>


class SplashView : public QWidget {
    Q_OBJECT

public:
    explicit SplashView(QWidget *parent = nullptr);
    void setMessage(const QString &message);

signals:
    void connectionRequested(const QString &deviceName);

private:
    void setupUi();
    QLabel *m_logoLabel;
    QLabel *m_statusLbl;
    QLabel *m_appNameLbl;
    QLabel *m_appVersionLbl;
    QLabel *m_copyrightLbl;

};
#endif // SPLASHVIEW_H
