#include "splashview.h"
#include "globaldefines.h"
#include "themecontroller.h"
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

SplashView::SplashView(QWidget *parent) : QWidget(parent, Qt::Window)
{
    setFixedSize(1000, 380);
    setupUi();
    move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
}

void SplashView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QFrame *container = new QFrame();
    container->setObjectName("SplashContainer");
    QVBoxLayout *splashLayout = new QVBoxLayout(container);
    splashLayout->setContentsMargins(60, 50, 60, 50);

    // --- TOP AREA: Logo Elements
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(ThemeController::imgsPath() + "/logo_with_name.png").scaled(200, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    topLayout->addWidget(logoLabel);
    splashLayout->addLayout(topLayout);

    // --- MIDDLE AREA: App Name & Version
    m_appNameLbl = new QLabel(GLB_SOFTWARE_NAME);
    m_appNameLbl->setObjectName("appNameLbl");

    m_appVersionLbl = new QLabel(GLB_SOFTWARE_VERSION_NUMBER);
    m_appVersionLbl->setObjectName("appVersionLbl");

    splashLayout->addWidget(m_appNameLbl);
    splashLayout->addWidget(m_appVersionLbl);
    splashLayout->addSpacing(30);

    m_statusLbl = new QLabel("LOADING DEVICES...");
    m_statusLbl->setObjectName("statusLbl");
    splashLayout->addWidget(m_statusLbl);
    splashLayout->addStretch();

    // --- BOTTOM AREA: Copyright
    splashLayout->addStretch();
    QLabel *copyrightLbl = new QLabel("Copyright 2025-2026 by Elements s.r.l\nSupport at info@elements-ic.com\nPowered by Qt");
    copyrightLbl->setObjectName("copyrightLbl");
    splashLayout->addWidget(copyrightLbl);

    mainLayout->addWidget(container);

}

void SplashView::setMessage(const QString &message)
{
    m_statusLbl->setText(message.toUpper());
}
