#include "temperaturedockwidget.h"

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

void TemperatureDockWidget::onTemperatureRead(std::vector <e384cl::Measurement_t> values) {
    for (int idx = 0; idx < values.size(); idx++) {
        temperatureLbls[idx]->setText(QString::fromStdString(values[idx].label()));
    }
}
