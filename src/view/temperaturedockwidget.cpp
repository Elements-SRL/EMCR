#include "temperaturedockwidget.h"
#include "activationbutton.h"
#include "globaldefines.h"
#include <QStyle>

TemperatureDockWidget::TemperatureDockWidget(QWidget * parent) :
    QDockWidget(parent) {

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setObjectName("temperatureSensorDw");
    this->setWindowTitle("T Sensors");

    QWidget * centralWidget = new QWidget();
    centralWidget->setObjectName("tSensorCentralWidget");
    externalLayout = new QVBoxLayout(centralWidget);
    externalLayout->setSpacing(0);
    externalLayout->setContentsMargins(10, 0, 10, 10);

    this->setWidget(centralWidget);

    connect(this, &QDockWidget::topLevelChanged, this, [centralWidget](bool isFloating) {
        if (isFloating) {
            centralWidget->setStyleSheet("");
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

    // TODO REMOVE FLAG 2
    for (uint16_t idx = 0; idx < 2; idx++) {

        QFrame* tempRowFrame = new QFrame();
        tempRowFrame->setObjectName("tempRowFrame");
        QHBoxLayout* rowLayout = new QHBoxLayout(tempRowFrame);
        rowLayout->setContentsMargins(8, 6, 8, 6);

        QLabel * chLbl = new QLabel(QString::number(idx));
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
    QHBoxLayout * fanHl = new QHBoxLayout;
    externalLayout->addLayout(fanHl);

    QDoubleSpinBox * fanSbx = new QDoubleSpinBox;
    fanSbx->setRange(range.min, range.max);
    fanSbx->setValue(range.max);
    fanHl->addWidget(fanSbx);
    fanHl->addWidget(new QLabel(QString::fromStdString(range.getFullUnit())));

    // TODO REMOVE FLAG
    if (debugControlsEnabled() || 1){

        QHBoxLayout * setTHl = new QHBoxLayout;
        externalLayout->addLayout(setTHl);

        setTSbx = new QDoubleSpinBox;
        setTSbx->setRange(30.0, 60.0);
        setTHl->addWidget(setTSbx);
        setTHl->addWidget(new QLabel("°C"));

        ActivationButton * setTEnBtn = new ActivationButton;
        setTHl->addWidget(setTEnBtn);

        connect(fanSbx, &QDoubleSpinBox::editingFinished, this, [=]() {
            emit sigSetFanSpeed({fanSbx->value(), range.prefix, range.unit});
        });

        connect(setTEnBtn, &ActivationButton::clicked, this, &TemperatureDockWidget::sigEnableKTControl);

        QHBoxLayout * setT1Hl = new QHBoxLayout;
        externalLayout->addLayout(setT1Hl);

        setT1Sbx = new QDoubleSpinBox;
        setT1Sbx->setRange(30.0, 60.0);
        setT1Hl->addWidget(setT1Sbx);
        setT1Hl->addWidget(new QLabel("°C"));

        ActivationButton * setTEn1Btn = new ActivationButton;
        setT1Hl->addWidget(setTEn1Btn);

        connect(setTEn1Btn, &ActivationButton::clicked, this, [=](bool enable) {
            emit sigEnableTControl({setT1Sbx->value(), e384CommLib::UnitPfxNone, "°C"}, enable);
        });

        QGridLayout * gl = new QGridLayout;
        externalLayout->addLayout(gl);
        auto pg = new QDoubleSpinBox;
        pg->setRange(0.0001, 100.0);
        pg->setDecimals(4);
        pg->setValue(0.04);
        gl->addWidget(pg, 0, 0);
        gl->addWidget(new QLabel("pg"), 0, 1);

        auto ig = new QDoubleSpinBox;
        ig->setRange(0.0001, 100.0);
        ig->setDecimals(4);
        ig->setValue(0.006);
        gl->addWidget(ig, 1, 0);
        gl->addWidget(new QLabel("ig"), 1, 1);

        auto ieMax = new QDoubleSpinBox;
        ieMax->setRange(1.0, 10000.0);
        ieMax->setDecimals(0);
        ieMax->setValue(200.0);
        gl->addWidget(ieMax, 2, 0);
        gl->addWidget(new QLabel("ieMax"), 2, 1);

        connect(pg, &QDoubleSpinBox::editingFinished, this, [=]() {
            emit sigPidParams(pg->value(), ig->value(), ieMax->value());
        });
        connect(ig, &QDoubleSpinBox::editingFinished, this, [=]() {
            emit sigPidParams(pg->value(), ig->value(), ieMax->value());
        });
        connect(ieMax, &QDoubleSpinBox::editingFinished, this, [=]() {
            emit sigPidParams(pg->value(), ig->value(), ieMax->value());
        });
    }
}

e384cl::Measurement_t TemperatureDockWidget::getTSet() {
    return {setTSbx->value(), e384CommLib::UnitPfxNone, "°C"};
}

void TemperatureDockWidget::onTemperatureRead(std::vector <e384cl::Measurement_t> values) {
    for (int idx = 0; idx < values.size(); idx++) {
        temperatureLbls[idx]->setText(QString::fromStdString(values[idx].label() + " °C"));
        updateTemperatureUi(idx, values[idx].value);
    }
}

void TemperatureDockWidget::updateTemperatureUi(unsigned int chIdx, double tempVal){
    // TODO - when will be defined, update logic for switching status
    // based on the temp value of specific device
    temperatureLbls[chIdx]->setProperty("status", DeviceTemperatureStatus::DEFAULT);
    temperatureLbls[chIdx]->style()->unpolish(temperatureLbls[chIdx]);
    temperatureLbls[chIdx]->style()->polish(temperatureLbls[chIdx]);
}
