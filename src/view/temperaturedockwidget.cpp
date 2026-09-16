#include "temperaturedockwidget.h"
#include "activationbutton.h"
#include "globaldefines.h"
#include <QStyle>
#include <QDebug>

enum DeviceTemperatureStatus {
    DEFAULT,
    NORMAL,
    WARNING,
    DANGER
};

static QMap<DeviceTemperatureStatus, QString> deviceTempStatusId = {
    {DEFAULT, "DEF"},
    {NORMAL, "NOR"},
    {WARNING, "WAR"},
    {DANGER, "DNG"}
};

TemperatureDockWidget::TemperatureDockWidget(QWidget * parent) :
    QDockWidget(parent) {

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setObjectName("temperatureSensorDw");
    this->setWindowTitle("T Sensors");

    QWidget * centralWidget = new QWidget();
    externalLayout = new QVBoxLayout(centralWidget);
    externalLayout->setContentsMargins(0, 0, 0, 0);
    externalLayout->setSpacing(0);

    this->setWidget(centralWidget);

    connect(this, &QDockWidget::topLevelChanged, this, [centralWidget](bool isFloating) {
        if (isFloating) {
            centralWidget->setStyleSheet("#controlsSectionContainer { border: none; }"
                                         "#tempRowFrame {border-left: none; border-right: none}"
                                         "#tempTableHeader {border-left: none; border-right: none}");
        } else {
            centralWidget->setStyleSheet("");
        }
    });

    // TOP BAR - Temperature grid
    QFrame * tempTableHeader = new QFrame();
    tempTableHeader->setObjectName("tempTableHeader");
    QHBoxLayout* tempTableHeaderLayout = new QHBoxLayout(tempTableHeader);
    tempTableHeaderLayout->setContentsMargins(8, 6, 8, 6);

    auto channelLbl = new QLabel("Channel");
    auto tempLbl = new QLabel("Temperature");


    tempTableHeaderLayout->addWidget(channelLbl);
    tempTableHeaderLayout->addStretch();
    tempTableHeaderLayout->addWidget(tempLbl);
    externalLayout->addWidget(tempTableHeader);
}

void TemperatureDockWidget::setChannels(int channelsNum) {

    for (uint16_t idx = 0; idx < channelsNum; idx++) {

        QFrame* tempRowFrame = new QFrame();
        tempRowFrame->setObjectName("tempRowFrame");
        QHBoxLayout* rowLayout = new QHBoxLayout(tempRowFrame);
        rowLayout->setContentsMargins(8, 6, 8, 6);

        QLabel * chLbl = new QLabel(QString::number(idx + 1));
        chLbl->setObjectName("tempChannelLbl");
        QLabel * tempLbl = new QLabel("---");
        tempLbl->setObjectName("tempValueLbl");
        tempLbl->setProperty("status" , deviceTempStatusId[DeviceTemperatureStatus::DEFAULT]);

        rowLayout->addWidget(chLbl);
        rowLayout->addStretch();
        rowLayout->addWidget(tempLbl);

        temperatureLbls.push_back(tempLbl);
        externalLayout->addWidget(tempRowFrame);
    }
}

void TemperatureDockWidget::enableFansControls(e384cl::RangedMeasurement_t range) {

    // Controls container
    QFrame* controlsSectionContainer = new QFrame();
    controlsSectionContainer->setObjectName("controlsSectionContainer");
    QVBoxLayout* controlsSectionLayout = new QVBoxLayout(controlsSectionContainer);
    controlsSectionLayout->setContentsMargins(0, 0, 0, 0);
    controlsSectionLayout->setSpacing(6);

    // Layout Label | SpinBox | Unit/Padding
    QGridLayout * formGl = new QGridLayout;
    formGl->setContentsMargins(8, 8, 8, 8);
    formGl->setHorizontalSpacing(8);
    formGl->setVerticalSpacing(6);
    formGl->setColumnStretch(0, 1);

    // Fan speed
    QLabel * fanSpeedLbl = new QLabel("Fan speed");
    QDoubleSpinBox * fanSbx = new QDoubleSpinBox;
    fanSbx->setRange(range.min, range.max);
    fanSbx->setValue(range.max);
    fanSbx->setMinimumWidth(90);

    QLabel* fanUnitLbl = new QLabel(QString::fromStdString(range.getFullUnit()));
    fanUnitLbl->setFixedWidth(30);

    formGl->addWidget(fanSpeedLbl, 0, 0);
    formGl->addWidget(fanSbx, 0, 1);
    formGl->addWidget(fanUnitLbl, 0, 2);

    // Temp 0
    ActivationButton * setTEnBtn = new ActivationButton;
    QLabel * setTHlbl = new QLabel("Temp 0");
    setTSbx = new QDoubleSpinBox;
    setTSbx->setRange(30.0, 60.0);
    setTSbx->setMinimumWidth(90);

    QHBoxLayout * temp0LblLayout = new QHBoxLayout;
    temp0LblLayout->setContentsMargins(0,0,0,0);
    temp0LblLayout->setSpacing(6);
    temp0LblLayout->addWidget(setTEnBtn);
    temp0LblLayout->addWidget(setTHlbl);
    temp0LblLayout->addStretch();

    QLabel * u0Lbl = new QLabel("°C");
    u0Lbl->setFixedWidth(30);

    formGl->addLayout(temp0LblLayout, 1, 0);
    formGl->addWidget(setTSbx, 1, 1);
    formGl->addWidget(u0Lbl, 1, 2);

    // Temp 1
    ActivationButton * setTEn1Btn = new ActivationButton;
    QLabel * setTEn1lbl = new QLabel("Temp 1");
    setT1Sbx = new QDoubleSpinBox;
    setT1Sbx->setRange(30.0, 60.0);
    setT1Sbx->setMinimumWidth(90);

    QHBoxLayout * temp1LblLayout = new QHBoxLayout;
    temp1LblLayout->setContentsMargins(0,0,0,0);
    temp1LblLayout->setSpacing(6);
    temp1LblLayout->addWidget(setTEn1Btn);
    temp1LblLayout->addWidget(setTEn1lbl);
    temp1LblLayout->addStretch();

    QLabel * u1Lbl = new QLabel("°C");
    u1Lbl->setFixedWidth(30);

    formGl->addLayout(temp1LblLayout, 2, 0);
    formGl->addWidget(setT1Sbx, 2, 1);
    formGl->addWidget(u1Lbl, 2, 2);

    // Prop Gain (Kp)
    QLabel * pgLbl = new QLabel("Prop Gain");
    pgLbl->setToolTip("Proportional Gain");

    auto pg = new QDoubleSpinBox;
    pg->setRange(0.0001, 100.0);
    pg->setDecimals(4);
    pg->setValue(0.04);
    pg->setMinimumWidth(90);

    QLabel * pgSpacerLbl = new QLabel("Kp");
    pgSpacerLbl->setFixedWidth(30);

    formGl->addWidget(pgLbl, 3, 0);
    formGl->addWidget(pg, 3, 1);
    formGl->addWidget(pgSpacerLbl, 3, 2);

    // Int Gain (Ki)
    QLabel * igLbl = new QLabel("Int Gain");
    igLbl->setToolTip("Integral Gain");

    auto ig = new QDoubleSpinBox;
    ig->setRange(0.0001, 100.0);
    ig->setDecimals(4);
    ig->setValue(0.006);
    ig->setMinimumWidth(90);

    QLabel * igSpacerLbl = new QLabel("Ki");
    igSpacerLbl->setFixedWidth(30);

    formGl->addWidget(igLbl, 4, 0);
    formGl->addWidget(ig, 4, 1);
    formGl->addWidget(igSpacerLbl, 4, 2);

    // Max Int Error
    QLabel * ieMaxLbl = new QLabel("Max Int Error");
    ieMaxLbl->setToolTip("Anti-windup limit for integral error");

    auto ieMax = new QDoubleSpinBox;
    ieMax->setRange(1.0, 10000.0);
    ieMax->setDecimals(0);
    ieMax->setValue(200.0);
    ieMax->setMinimumWidth(90);

    QLabel * ieMaxSpacerLbl = new QLabel("");
    ieMaxSpacerLbl->setFixedWidth(30);

    formGl->addWidget(ieMaxLbl, 5, 0);
    formGl->addWidget(ieMax, 5, 1);
    formGl->addWidget(ieMaxSpacerLbl, 5, 2);

    bool showDebug = debugControlsEnabled();

    // Following controls are available
    // in DEBUG mode only

    // Temp 0
    setTEnBtn->setVisible(showDebug);
    setTHlbl->setVisible(showDebug);
    setTSbx->setVisible(showDebug);
    u0Lbl->setVisible(showDebug);

    // Temp 1
    setTEn1Btn->setVisible(showDebug);
    setTEn1lbl->setVisible(showDebug);
    setT1Sbx->setVisible(showDebug);
    u1Lbl->setVisible(showDebug);

    // PID
    pgLbl->setVisible(showDebug);
    pg->setVisible(showDebug);
    pgSpacerLbl->setVisible(showDebug);

    igLbl->setVisible(showDebug);
    ig->setVisible(showDebug);
    igSpacerLbl->setVisible(showDebug);

    ieMaxLbl->setVisible(showDebug);
    ieMax->setVisible(showDebug);
    ieMaxSpacerLbl->setVisible(showDebug);

    // Connections
    connect(setTEnBtn, &ActivationButton::clicked, this, &TemperatureDockWidget::sigEnableKTControl);

    connect(setTEn1Btn, &ActivationButton::clicked, this, [=](bool enable) {
        emit sigEnableTControl({setT1Sbx->value(), e384CommLib::UnitPfxNone, "°C"}, enable);
    });

    auto emitPidParams = [=]() {
        emit sigPidParams(pg->value(), ig->value(), ieMax->value());
    };

    connect(pg, &QDoubleSpinBox::editingFinished, this, emitPidParams);
    connect(ig, &QDoubleSpinBox::editingFinished, this, emitPidParams);
    connect(ieMax, &QDoubleSpinBox::editingFinished, this, emitPidParams);

    connect(fanSbx, &QDoubleSpinBox::editingFinished, this, [=]() {
        emit sigSetFanSpeed({fanSbx->value(), range.prefix, range.unit});
    });

    controlsSectionLayout->addLayout(formGl);
    controlsSectionLayout->addStretch();
    externalLayout->addWidget(controlsSectionContainer);
}

e384cl::Measurement_t TemperatureDockWidget::getTSet() {
    return {setTSbx->value(), e384CommLib::UnitPfxNone, "°C"};
}

void TemperatureDockWidget::onTemperatureRead(std::vector <e384cl::Measurement_t> values) {
    for (int idx = 0; idx < values.size(); idx++) {
        temperatureLbls[idx]->setText(QString::fromStdString(values[idx].label()));
        updateTemperatureUi(idx, values[idx].value);
    }
}

void TemperatureDockWidget::updateTemperatureUi(unsigned int chIdx, double tempVal){
    if (tempVal < 0){
        qWarning() << "WARNING - Temperature sensor reporting [" + QString::number(tempVal) + "]";
    }

    auto tempStatus = DeviceTemperatureStatus::DEFAULT;

    // TODO - when will be defined, update logic for switching status
    // based on the temp value of specific device
    // if (tempStatus > 90){
    //     tempStatus = DeviceTemperatureStatus::DANGER;
    // }else if (tempStatus >= 65){
    //     tempStatus = DeviceTemperatureStatus::WARNING;
    // } else {
    //     tempStatus = DeviceTemperatureStatus::NORMAL;
    // }

    temperatureLbls[chIdx]->setProperty("status", tempStatus);
    temperatureLbls[chIdx]->style()->unpolish(temperatureLbls[chIdx]);
    temperatureLbls[chIdx]->style()->polish(temperatureLbls[chIdx]);
}
