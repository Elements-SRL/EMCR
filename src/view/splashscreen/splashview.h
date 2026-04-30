#ifndef SPLASHVIEW_H
#define SPLASHVIEW_H

#include "qcombobox.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qstackedwidget.h"
#include <QWidget>
#include <QSvgWidget>


class SplashView : public QWidget {
    Q_OBJECT

public:
    explicit SplashView(QWidget *parent = nullptr);

    void setDevicesFound(const std::vector<std::string>& devicesList);
    void setMessage(const QString &message);
    void showSelectionPage();
    QPushButton * getConnectButton();

signals:
    void connectionRequested(const QString &deviceName);

private:
    void setupUi();
    QWidget *m_splash;
    QWidget *m_deviceCombo;
    QPushButton *m_connectBtn;
    QStackedWidget *m_stack;
    QLabel *m_logoLabel;
    QLabel *m_statusLbl;
    QLabel *m_appNameLbl;
    QLabel *m_appVersionLbl;
    QLabel *m_copyrightLbl;

};
#endif // SPLASHVIEW_H
