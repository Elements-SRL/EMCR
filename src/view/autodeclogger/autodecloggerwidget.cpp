#include "autodecloggerwidget.h"
#include "qscrollarea.h"

void configureDoubleSpinbox(QDoubleSpinBox * spinbox, RangedMeasurement rm) {
    spinbox->setSuffix(" " + QString::fromStdString(rm.getFullUnit()));
    spinbox->setMinimum(0.0);
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


    // Window border management when floating
    connect(this, &QDockWidget::topLevelChanged, this, [centralWidget](bool isFloating) {
        if (isFloating) {
            centralWidget->setStyleSheet("#autodecloggerScrollContainer { border: none; }"
                                         "#customTitleBar { border-left: none; border-right: none; }"
                                         "#poreHeaderContainer { border-bottom: none; border-left: none; border-right: none;}");
        } else {
            centralWidget->setStyleSheet("");
        }
    });

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

    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContainer);
    scrollLayout->setContentsMargins(0, 8, 0, 0);
    scrollLayout->setSpacing(8);

    scrollArea->setWidget(scrollContainer);
    outerLayout->addWidget(scrollArea);

    /* Current threshold */
    QFrame* currentContainer = new QFrame();
    QHBoxLayout * currentLayout = new QHBoxLayout();
    currentLayout->setContentsMargins(8, 0, 8, 2);
    currentLayout->setSpacing(4);
    currentContainer->setLayout(currentLayout);

    thField = new QDoubleSpinBox();
    configureDoubleSpinbox(thField, cr);

    currentLayout->addWidget(new QLabel("Current threshold"));
    currentLayout->addStretch();
    currentLayout->addWidget(thField);


    /* Voltage - Declogging stimulus */
    QFrame* voltageContainer = new QFrame();
    QHBoxLayout * voltageLayout = new QHBoxLayout();
    voltageLayout->setContentsMargins(8, 4, 8, 2);
    voltageLayout->setSpacing(4);
    voltageContainer->setLayout(voltageLayout);

    voltageField = new QDoubleSpinBox();
    configureDoubleSpinbox(voltageField, vr);
    voltageField->setMinimum(vr.min);
    voltageField->setValue(-100.0);

    voltageLayout->addWidget(new QLabel("Declogging stimulus"));
    voltageLayout->addStretch();
    voltageLayout->addWidget(voltageField);


    /* Declogging time */
    QFrame* decTimeContainer = new QFrame();
    QHBoxLayout * decTimeLayout = new QHBoxLayout();
    decTimeLayout->setContentsMargins(8, 4, 8, 2);
    decTimeLayout->setSpacing(4);
    decTimeContainer->setLayout(decTimeLayout);

    timeField = new QDoubleSpinBox(this);
    timeField->setSuffix(" ms");
    timeField->setMaximum(20000);
    timeField->setValue(500.0);
    timeField->setDecimals(0);

    decTimeLayout->addWidget(new QLabel("Declogging time"));
    decTimeLayout->addStretch();
    decTimeLayout->addWidget(timeField);


    /* Time below threshold */
    QFrame* timeBelowContainer = new QFrame();
    QHBoxLayout * timeBelowLayout = new QHBoxLayout();
    timeBelowLayout->setContentsMargins(8, 4, 8, 2);
    timeBelowLayout->setSpacing(4);
    timeBelowContainer->setLayout(timeBelowLayout);

    timeBelowThreshold = new QDoubleSpinBox(this);
    timeBelowThreshold->setSuffix(" ms");
    timeBelowThreshold->setMaximum(20000);
    timeBelowThreshold->setValue(500.0);
    timeBelowThreshold->setDecimals(0);

    timeBelowLayout->addWidget(new QLabel("Time below threshold"));
    timeBelowLayout->addStretch();
    timeBelowLayout->addWidget(timeBelowThreshold);

    // BOTTOM/FOOTER - pore status
    QFrame* poreHeaderContainer = new QFrame();
    poreHeaderContainer->setObjectName("poreHeaderContainer");
    QHBoxLayout* poreHeaderOutputLayout = new QHBoxLayout(poreHeaderContainer);
    poreHeaderOutputLayout->setContentsMargins(0, 0, 0, 0);

    QLabel * poreTitle = new QLabel("PORE STATUS:");
    poreTitle->setObjectName("poreOutputLbl");

    poreHeaderOutputLayout->addWidget(poreTitle);
    poreHeaderOutputLayout->addStretch();

    poreStatusLbl = new QLabel("DEFAULT");
    poreStatusLbl->setObjectName("poreStatusLbl");
    poreHeaderOutputLayout->addWidget(poreStatusLbl);


    scrollLayout->addWidget(currentContainer);
    scrollLayout->addWidget(voltageContainer);
    scrollLayout->addWidget(decTimeContainer);
    scrollLayout->addWidget(timeBelowContainer);
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
