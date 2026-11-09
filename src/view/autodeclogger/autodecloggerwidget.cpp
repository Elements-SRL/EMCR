#include "autodecloggerwidget.h"
#include "qscrollarea.h"

void configureDoubleSpinbox(QDoubleSpinBox * spinbox, RangedMeasurement rm) {
    spinbox->setMinimum(rm.min);
    spinbox->setMaximum(rm.max);
    spinbox->setDecimals(4);
}

AutoDecloggerWidget::AutoDecloggerWidget(RangedMeasurement cr, RangedMeasurement vr, QWidget* parent):
    QDockWidget(parent) {

    this->setObjectName("autoDeclogger");
    this->setWindowTitle("Auto declogger");

    auto centralWidget = new QWidget(this);
    centralWidget->setObjectName("autodecloggerCentralWidget");
    auto outerLayout = new QVBoxLayout();
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    centralWidget->setLayout(outerLayout);
    setWidget(centralWidget);

    // TOP BAR
    QFrame* activationHeader = new QFrame();
    activationHeader->setObjectName("sectionHeaderContainer");
    QHBoxLayout* topBarLayout = new QHBoxLayout(activationHeader);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(0);
    topBarLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    topBarLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    activateBtnLbl = new QLabel("Activate");
    activateBtnLbl->setObjectName("activateBtnLbl");
    active = new ActivationButton(this);

    topBarLayout->addWidget(activateBtnLbl);
    topBarLayout->addStretch();
    topBarLayout->addWidget(active);

    outerLayout->addWidget(activationHeader);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    scrollArea->setMinimumHeight(0);

    QFrame * scrollContainer = new QFrame();
    scrollContainer->setObjectName("autodecloggerScrollContainer");
    scrollContainer->setProperty("floatingAware", true);    // border depends on the dock floating state

    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContainer);
    scrollLayout->setContentsMargins(0, 8, 0, 0);
    scrollLayout->setSpacing(8);

    scrollArea->setWidget(scrollContainer);
    outerLayout->addWidget(scrollArea);

    QFrame* fieldsContainer = new QFrame();
    fieldsContainer->setObjectName("autodecloggerFields");
    QGridLayout* grid = new QGridLayout(fieldsContainer);
    grid->setContentsMargins(8, 4, 8, 4);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(6);

    auto addRow = [&](int row, const QString& text, QDoubleSpinBox* sbx) {
        auto* lbl = new QLabel(text);
        lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        sbx->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sbx->setMinimumWidth(sbx->sizeHint().width());   // Avoids truncation
        grid->addWidget(lbl, row, 0);
        grid->addWidget(sbx, row, 1);
    };

    thField             = new QDoubleSpinBox(); configureDoubleSpinbox(thField, cr);    /* Current threshold */
    voltageField        = new QDoubleSpinBox(); configureDoubleSpinbox(voltageField, vr);   /* Voltage - Declogging stimulus */
    timeField           = new QDoubleSpinBox(); timeField->setMaximum(20000); timeField->setDecimals(0);    /* Declogging time */
    timeBelowThreshold  = new QDoubleSpinBox(); timeBelowThreshold->setMaximum(20000); timeBelowThreshold->setDecimals(0);  /* Time below threshold */

    /* Initial values */
    voltageField->setValue(-100.0);
    timeField->setValue(500.0);
    timeBelowThreshold->setValue(500.0);

    addRow(0, QStringLiteral("Current threshold (%1)").arg(QString::fromStdString(cr.getFullUnit())), thField);
    addRow(1, QStringLiteral("Declogging stimulus (%1)").arg(QString::fromStdString(vr.getFullUnit())), voltageField);
    addRow(2, QStringLiteral("Declogging time (ms)"), timeField);
    addRow(3, QStringLiteral("Time below threshold (ms)"), timeBelowThreshold);

    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);

    // BOTTOM/FOOTER - pore status
    QFrame* poreHeaderContainer = new QFrame();
    poreHeaderContainer->setObjectName("poreHeaderContainer");
    poreHeaderContainer->setProperty("floatingAware", true);    // border depends on the dock floating state
    QHBoxLayout* poreHeaderOutputLayout = new QHBoxLayout(poreHeaderContainer);
    poreHeaderOutputLayout->setContentsMargins(0, 0, 0, 0);

    QLabel * poreTitle = new QLabel("PORE STATUS:");
    poreTitle->setObjectName("poreOutputLbl");

    poreHeaderOutputLayout->addWidget(poreTitle);
    poreHeaderOutputLayout->addStretch();

    poreStatusLbl = new QLabel("DEFAULT");
    poreStatusLbl->setObjectName("poreStatusLbl");
    poreHeaderOutputLayout->addWidget(poreStatusLbl);

    scrollLayout->addWidget(fieldsContainer);
    scrollLayout->addStretch();

    // Sticky footer
    outerLayout->addWidget(poreHeaderContainer);

    /* Connections */
    connect(active, &ActivationButton::clicked, this, [=](bool checked) { emit sigActive(checked);
                                                                          activateBtnLbl->setProperty("active", checked);

                                                                        });
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
    poreStatusLbl->setText("CLOGGED");
}

void AutoDecloggerWidget::onPoreFree() {
    poreStatusLbl->setText("FREE");
}
