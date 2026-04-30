#include "splashview.h"
#include "globaldefines.h"
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

SplashView::SplashView(QWidget *parent) : QWidget(parent, Qt::FramelessWindowHint | Qt::SplashScreen)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1000, 450);
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
    logoLabel->setPixmap(QPixmap(":/imgs/logo_with_name.png").scaled(200, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

    m_stack = new QStackedWidget();

    // Page 0: Loading State
    QWidget *loadingPage = new QWidget();
    loadingPage->setObjectName("loadingPage");
    QVBoxLayout *loadingLayout = new QVBoxLayout(loadingPage);
    loadingLayout->setContentsMargins(0,0,0,0);

    m_statusLbl = new QLabel("LOADING DEVICES...");
    m_statusLbl->setObjectName("statusLbl");
    loadingLayout->addWidget(m_statusLbl);
    loadingLayout->addStretch();

    // Page 1: Selection State
    QWidget *selectionPage = new QWidget();
    QVBoxLayout *selectionLayout = new QVBoxLayout(selectionPage);
    selectionLayout->setContentsMargins(0,0,0,0);
    selectionLayout->setSpacing(10);

    QHBoxLayout *formHLayout = new QHBoxLayout();
    QVBoxLayout *widgetsColumn = new QVBoxLayout();
    widgetsColumn->setSpacing(15);

    m_deviceCombo = new QComboBox();
    m_deviceCombo->setObjectName("deviceCombo");
    m_deviceCombo->setFixedWidth(260);

    m_connectBtn = new QPushButton("CONNECT");
    m_connectBtn->setObjectName("connectBtn");
    m_connectBtn->setFixedWidth(260);
    m_connectBtn->setCursor(Qt::PointingHandCursor);

    widgetsColumn->addWidget(m_deviceCombo);
    widgetsColumn->addWidget(m_connectBtn);

    formHLayout->addLayout(widgetsColumn);
    formHLayout->addStretch();

    selectionLayout->addLayout(formHLayout);
    selectionLayout->addStretch();

    m_stack->addWidget(loadingPage);   // Index 0
    m_stack->addWidget(selectionPage); // Index 1
    splashLayout->addWidget(m_stack);

    // --- BOTTOM AREA: Copyright
    splashLayout->addStretch();
    QLabel *copyrightLbl = new QLabel("Copyright 2025-2026 by Elements s.r.l\nSupport at info@elements-ic.com | Powered by Qt.");
    copyrightLbl->setObjectName("copyrightLbl");
    splashLayout->addWidget(copyrightLbl);

    mainLayout->addWidget(container);

}

void SplashView::showSelectionPage(){
    m_stack->setCurrentIndex(1);
}

void SplashView::setMessage(const QString &message)
{
    m_statusLbl->setText(message.toUpper());
}
