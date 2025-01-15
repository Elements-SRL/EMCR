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
    timeField->setMaximum(20000);
    timeField->setValue(500.0);
    timeField->setDecimals(0);
    timeBelowThreshold = new QDoubleSpinBox(this);
    timeBelowThreshold->setSuffix("ms");
    timeBelowThreshold->setMaximum(20000);
    timeBelowThreshold->setValue(500.0);
    timeBelowThreshold->setDecimals(0);
    voltageField = new QDoubleSpinBox(this);
    configureDoubleSpinbox(voltageField, vr);
    voltageField->setMinimum(vr.min);
    voltageField->setValue(-100.0);
    freePore = new QLabel("The pore is free");
    cloggedPore = new QLabel("The pore is clogged");
    outerLayout->addWidget(new QLabel("Activate", this));
    outerLayout->addWidget(active);
    outerLayout->addWidget(new QLabel("Current threshold", this));
    outerLayout->addWidget(thField);
    outerLayout->addWidget(new QLabel("Declogging stimulus", this));
    outerLayout->addWidget(voltageField);
    outerLayout->addWidget(new QLabel("Declogging time", this));
    outerLayout->addWidget(timeField);
    outerLayout->addWidget(new QLabel("Time below treshold", this));
    outerLayout->addWidget(timeBelowThreshold);
    outerLayout->addWidget(freePore);
    outerLayout->addWidget(cloggedPore);
    freePore->hide();
    cloggedPore->hide();
    setLayout(outerLayout);
    setWidget(centralWidget);
    connect(active, &QCheckBox::clicked, this, [=](bool checked) { emit sigActive(checked); });
    connect(timeField, &QDoubleSpinBox::editingFinished, this, [=]() { emit sigTimeFieldChanged(timeField->value()); });
    connect(timeBelowThreshold, &QDoubleSpinBox::editingFinished, this, [=]() { emit sigTimeBelowThresholdChanged(timeBelowThreshold->value()); });
    connect(thField, &QDoubleSpinBox::editingFinished, this, [=]() { emit sigThFieldChanged(thField->value()); });
    connect(voltageField, &QDoubleSpinBox::editingFinished, this, [=]() { emit sigVotageFieldChanged(voltageField->value()); });
}

double AutoDecloggerWidget::getThreshold() {
    return this->thField->value();
}

double AutoDecloggerWidget::getTime() {
    return this->timeField->value();
}

double AutoDecloggerWidget::getTimeBelowThreshold() {
    return this->timeBelowThreshold->value();
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

void AutoDecloggerWidget::onPoreClogged() {
    freePore->hide();
    cloggedPore->show();
}

void AutoDecloggerWidget::onPoreFree() {
    freePore->show();
    cloggedPore->hide();
}