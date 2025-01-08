#include "autodecloggerwidget.h"

void configureDoubleSpinbox(QDoubleSpinBox * spinbox, RangedMeasurement rm) {
    spinbox->setSuffix(QString::fromStdString(rm.getFullUnit()));
    spinbox->setMinimum(0.0);
    spinbox->setMaximum(rm.max);
    spinbox->setDecimals(4);
}

AutoDecloggerWidget::AutoDecloggerWidget(RangedMeasurement cr, RangedMeasurement vr, QWidget* parent):
    QDockWidget(parent) {

    this->setObjectName("autoDeclogger");
    this->setWindowTitle("Auto declogger");
    auto centralWidget = new QWidget(this);
    auto outerLayout = new QVBoxLayout(this);
    centralWidget->setLayout(outerLayout);
    active = new QCheckBox(this);
    thField = new QDoubleSpinBox(this);
    configureDoubleSpinbox(thField, cr);
    timeField = new QDoubleSpinBox(this);
    timeField->setSuffix("ms");
    voltageField = new QDoubleSpinBox(this);
    configureDoubleSpinbox(voltageField, vr);
    outerLayout->addWidget(new QLabel("Activate", this));
    outerLayout->addWidget(active);
    outerLayout->addWidget(new QLabel("Current threshold", this));
    outerLayout->addWidget(thField);
    outerLayout->addWidget(new QLabel("Declogging time", this));
    outerLayout->addWidget(timeField);
    outerLayout->addWidget(new QLabel("Declogging stimulus", this));
    outerLayout->addWidget(voltageField);
    setLayout(outerLayout);
    setWidget(centralWidget);
    //connect(autoZoomButton, &QPushButton::clicked, this, &IvGraphWidget::sigAutoZoom);
}

double AutoDecloggerWidget::getThreshold() {
    return this->thField->value();
}

double AutoDecloggerWidget::getTime() {
    return this->timeField->value();
}

double AutoDecloggerWidget::getVoltage() {
    return this->voltageField->value();
}

void AutoDecloggerWidget::currentRangeChanged(RangedMeasurement cr) {
    configureDoubleSpinbox(thField, cr);
}

void AutoDecloggerWidget::voltageRangeChanged(RangedMeasurement vr) {
    configureDoubleSpinbox(voltageField, vr);
}