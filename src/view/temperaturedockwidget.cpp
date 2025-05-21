#include "temperaturedockwidget.h"

#include "activationbutton.h"

TemperatureDockWidget::TemperatureDockWidget(QWidget * parent) :
    QDockWidget(parent) {

    this->setObjectName("temperatureSensorDw");
    this->setWindowTitle("T Sensors");

    QWidget * wid = new QWidget;
    this->setWidget(wid);

    mainVl = new QVBoxLayout;
    wid->setLayout(mainVl);
}

void TemperatureDockWidget::setChannels(int channelsNum) {
    for (uint16_t idx = 0; idx < channelsNum; idx++) {
        temperatureLbls.push_back(new QLabel("---"));
        mainVl->addWidget(temperatureLbls[idx]);
    }
}

void TemperatureDockWidget::enableFansControls(e384cl::RangedMeasurement_t range) {
    QHBoxLayout * fanHl = new QHBoxLayout;
    mainVl->addLayout(fanHl);

    QDoubleSpinBox * fanSbx = new QDoubleSpinBox;
    fanSbx->setRange(range.min, range.max);
    fanSbx->setValue(range.max);
    fanHl->addWidget(fanSbx);
    fanHl->addWidget(new QLabel(QString::fromStdString(range.getFullUnit())));

    QHBoxLayout * setTHl = new QHBoxLayout;
    mainVl->addLayout(setTHl);

    setTSbx = new QDoubleSpinBox;
    setTSbx->setRange(30.0, 60.0);
    setTHl->addWidget(setTSbx);
    setTHl->addWidget(new QLabel("°C"));

    ActivationButton * setTEnBtn = new ActivationButton;
    setTHl->addWidget(setTEnBtn);

    connect(fanSbx, &QDoubleSpinBox::editingFinished, this, [=]() {
        emit sigSetFanSpeed({fanSbx->value(), range.prefix, range.unit});
    });

    connect(setTEnBtn, &ActivationButton::clicked, this, &TemperatureDockWidget::sigEnableTControl);
}

e384cl::Measurement_t TemperatureDockWidget::getTSet() {
    return {setTSbx->value(), e384CommLib::UnitPfxNone, "°C"};
}

void TemperatureDockWidget::onTemperatureRead(std::vector <e384cl::Measurement_t> values) {
    for (int idx = 0; idx < values.size(); idx++) {
        temperatureLbls[idx]->setText(QString::fromStdString(values[idx].label()));
    }
}
