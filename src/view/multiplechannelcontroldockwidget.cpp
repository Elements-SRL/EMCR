#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>
#include <QSettings>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QStyle>
#include "globaldefines.h"

MultipleChannelControlDockWidget::MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    setAttribute(Qt::WA_TranslucentBackground);
    this->setObjectName("MultipleChannelControlDockWidget");

    // Custom TitleBar widget
    QWidget* customTitleBar = new QWidget();
    customTitleBar->setObjectName("customTitleBar");
    QHBoxLayout* titleLayout = new QHBoxLayout(customTitleBar);
    titleLayout->setContentsMargins(10, 5, 10, 5);

    QLabel* titleLabel = new QLabel("MULTIPLE CHANNEL CONTROLS");
    titleLabel->setObjectName("titleLabel");

    // Custom Window buttons - expand & close
    // TODO Fix hover
    QPushButton* expandBtn = new QPushButton();
    expandBtn->setObjectName("windowExpand");
    QPushButton* closeBtn = new QPushButton();
    closeBtn->setObjectName("windowClose");

    // Custom buttons actions
    connect(closeBtn, &QPushButton::clicked, this, &QDockWidget::hide);
    connect(expandBtn, &QPushButton::clicked, this, [=]() {
        this->setFloating(!this->isFloating());
    });

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(expandBtn);
    titleLayout->addWidget(closeBtn);
    this->setTitleBarWidget(customTitleBar);


    QWidget* centralWidget = new QWidget();
    this->setWidget(centralWidget);

    // Main Frame Layout
    QVBoxLayout* externalLayout = new QVBoxLayout(centralWidget);
    externalLayout->setContentsMargins(0, 0, 0, 0);
    externalLayout->setSpacing(0);

    // Main Frame
    QFrame* mainWrapper = new QFrame();
    mainWrapper->setObjectName("mainWrapper");
    mainWrapper->setFrameStyle(QFrame::Panel | QFrame::Raised);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWrapper);

    setWindowTitle("Multiple channel controls");
    this->setObjectName("multipleChannelControlsDw");
    mainWrapper->setLayout(mainLayout);

    m_selectionCounterLabel = new QLabel();
    m_selectionCounterLabel->setObjectName("selectionCounter");
    m_selectionCounterLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(m_selectionCounterLabel);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Helper lambda for panel sub-property creation
    auto addPropertyRow = [&](QString title, AutoToggle* &autoTgl, QLabel* &badge,
                              QPushButton* &onBtn, QPushButton* &offBtn, QVBoxLayout* layout) {
        QFrame* rowContainer = new QFrame();
        rowContainer->setObjectName("propertyContainer");
        auto vbl = new QVBoxLayout(rowContainer);
        vbl->setContentsMargins(0, 5, 0, 10);
        vbl->setSpacing(4);

        // Row 1 - Title + AutoToggle
        auto r1 = new QHBoxLayout();
        QLabel * titleLbl = new QLabel(title);
        titleLbl->setObjectName("propertyName");
        r1->addWidget(titleLbl);
        r1->addStretch();
        autoTgl = new AutoToggle();
        r1->addWidget(autoTgl);
        vbl->addLayout(r1);

        // Row 2 - Badge + ON/OFF
        auto r2 = new QHBoxLayout();

        // TODO show proper badge
        badge = new QLabel("Mixed");
        badge->setObjectName("badge");
        badge->setProperty("status", "mixed");
        badge->setAlignment(Qt::AlignCenter);

        onBtn = new QPushButton("ON");
        offBtn = new QPushButton("OFF");
        onBtn->setFixedWidth(48);
        offBtn->setFixedWidth(48);

        r2->addWidget(badge);
        r2->addStretch();
        r2->addWidget(onBtn);
        r2->addWidget(offBtn);
        vbl->addLayout(r2);

        layout->addWidget(rowContainer);
    };

    // TODO capise se nascondere sezioni quando
    // non ci sono proprietà attive
    // TODO connessione stati nel summary

    // First section - VISIBILITY
    QLabel *header = new QLabel("VISIBILITY & PLOTTING");
    header->setObjectName("sectionHeader");
    mainLayout->addWidget(header);

    // Property - Expand Trace
    QLabel *expandTraceBadge;
    addPropertyRow("Expand trace", expandTraceAutoBtn, expandTraceBadge, expandTraceBtn, reduceTraceBtn, mainLayout);
    connect(expandTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigAddToBigPlot);
    connect(reduceTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigRemoveFromBigPlot);
    connect(expandTraceAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigAddToBigPlotAuto);

    // Property - Plot detail
    QLabel *plotDetailBadge;
    addPropertyRow("Plot detail", plotDetailAutoBtn, plotDetailBadge, expandChannelDetailBtn, reduceChannelDetailBtn, mainLayout);
    connect(expandChannelDetailBtn, &QPushButton::clicked, this, [=]() {
        emit sigAddRemovePlotDetail(true);
    });
    connect(reduceChannelDetailBtn, &QPushButton::clicked, this, [=]() {
        emit sigAddRemovePlotDetail(false);
    });
    connect(plotDetailAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigAddPlotDetailAuto);

    // Second section - SIGNAL
    QLabel *signalHeader = new QLabel("SIGNAL");
    signalHeader->setObjectName("sectionHeader");
    mainLayout->addWidget(signalHeader);

    if (msgDisp->hasChannelSwitches() == Success) {

        QLabel *chInputBadge;
        addPropertyRow("Channel input", switchChannelsAutoBtn, chInputBadge, switchChannelsOnBtn, switchChannelsOffBtn, mainLayout);
        connect(switchChannelsOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOn);
        connect(switchChannelsOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOff);
        connect(switchChannelsAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigTurnChannelAuto);
    }

    // TODO continue creating new propeties
    if (msgDisp->hasCalSw() == Success && debugControlsEnabled()) {
        auto calib_gb = new QGroupBox(QString::fromStdString("Calibration resistors"));
        auto qhbl = new QHBoxLayout();
        calib_gb->setLayout(qhbl);
        mainLayout->addWidget(calib_gb);
        calibrationResistorsOnBtn = new QPushButton("ON (R)");
        connect(calibrationResistorsOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnCalibrationResistorsOn);
        qhbl->addWidget(calibrationResistorsOnBtn);
        calibrationResistorsOffBtn = new QPushButton("OFF");
        connect(calibrationResistorsOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnCalibrationResistorsOff);
        qhbl->addWidget(calibrationResistorsOffBtn);
    }

    if (msgDisp->hasStimulusSwitches() == Success) {
        QLabel *stimulusBadge;
        addPropertyRow("Stimulus", turnStimulusAutoBtn, stimulusBadge, turnStimulusOnBtn, turnStimulusOffBtn, mainLayout);
        connect(turnStimulusOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOn);
        connect(turnStimulusOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOff);
        connect(turnStimulusAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigTurnStimulusAuto);

    }

    RangedMeasurement_t zapDurationRange;
    if (msgDisp->getZapFeatures(zapDurationRange) == Success) {

        // Zap Pulse
        auto zapRow = new QFrame();
        zapRow->setObjectName("propertyContainer");
        auto zapVbl = new QVBoxLayout(zapRow);
        zapVbl->setContentsMargins(0, 5, 0, 10);
        zapVbl->setSpacing(4);

        auto hblZap = new QHBoxLayout(zapRow);
        QLabel * zaptitleLbl = new QLabel("Zap pulse");
        zaptitleLbl->setObjectName("propertyName");
        hblZap->addWidget(zaptitleLbl);
        hblZap->addStretch();
        zapVbl->addLayout(hblZap);

        auto zapDurationRow = new QHBoxLayout();
        zapDurationRow->addWidget(new QLabel("Duration"));
        zapDurationRow->addStretch();
        zapBtn = new QPushButton("ZAP");
        zapDurationRow->addWidget(zapBtn);
        auto zapSbx = new QDoubleSpinBox();
        zapDurationRange.convertValues(UnitPfxMilli);
        zapSbx->setRange(zapDurationRange.min, zapDurationRange.max);
        zapSbx->setValue(100.0);
        zapSbx->setProperty("technical", true);
        zapDurationRow->addWidget(zapSbx);
        zapDurationRow->addWidget(new QLabel("ms"));
        zapVbl->addLayout(zapDurationRow);
        mainLayout->addWidget(zapRow);

        connect(zapBtn, &QPushButton::clicked, this, [=] () {
            emit sigZap({zapSbx->value(), UnitPfxMilli, "s"});
        });
    }

    if (msgDisp->hasOffsetCompensation() == Success) {
        auto gb = new QGroupBox("Offset correction");
        mainLayout->addWidget(gb);
        auto qvbl = new QVBoxLayout();
        gb->setLayout(qvbl);
        offsetCorrectionExpertChb = new QCheckBox("Expert");
        qvbl->addWidget(offsetCorrectionExpertChb);

        auto ww = new QWidget;
        auto qhblw = new QHBoxLayout();
        ww->setLayout(qhblw);
        qvbl->addWidget(ww);
        offsetCorrectionStartBtn = new QPushButton("Start");
        connect(offsetCorrectionStartBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStartOffsetCorrection);
        qhblw->addWidget(offsetCorrectionStartBtn);
        offsetCorrectionStopBtn = new QPushButton("Stop");
        connect(offsetCorrectionStopBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStopOffsetCorrection);
        qhblw->addWidget(offsetCorrectionStopBtn);

        auto gbr = new QGroupBox("Offset recalibration");
        gbr->setVisible(false);
        auto qhblr = new QHBoxLayout();
        gbr->setLayout(qhblr);
        qvbl->addWidget(gbr);
        offsetRecalibrationOnBtn = new QPushButton("ON (C)");
        connect(offsetRecalibrationOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOn);
        qhblr->addWidget(offsetRecalibrationOnBtn);
        offsetRecalibrationOffBtn = new QPushButton("OFF");
        connect(offsetRecalibrationOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOff);
        qhblr->addWidget(offsetRecalibrationOffBtn);
        offsetRecalibrationResetBtn = new QPushButton("RESET");
        connect(offsetRecalibrationResetBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigResetOffsetRecalibration);
        qhblr->addWidget(offsetRecalibrationResetBtn);

        auto gbl = new QGroupBox("Liquid junction compensation");
        gbl->setVisible(false);
        auto qhbll = new QHBoxLayout();
        gbl->setLayout(qhbll);
        qvbl->addWidget(gbl);
        liquidJunctionCompensationOnBtn = new QPushButton("ON (J)");
        connect(liquidJunctionCompensationOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnLjcOn);
        qhbll->addWidget(liquidJunctionCompensationOnBtn);
        liquidJunctionCompensationOffBtn = new QPushButton("OFF");
        connect(liquidJunctionCompensationOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnLjcOff);
        qhbll->addWidget(liquidJunctionCompensationOffBtn);
        liquidJunctionCompensationResetBtn = new QPushButton("RESET");
        connect(liquidJunctionCompensationResetBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigResetLj);
        qhbll->addWidget(liquidJunctionCompensationResetBtn);

        connect(offsetCorrectionExpertChb, &QPushButton::clicked, this, [=](bool checked) {
            ww->setVisible(!checked);
            gbr->setVisible(checked);
            gbl->setVisible(checked);
        });
    }

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    mainLayout->addWidget(spacer);

    // Summary
    QWidget *summaryWg = new QWidget();
    summaryWg->setObjectName("summaryContainer");
    QGridLayout *summaryLayout = new QGridLayout(summaryWg);
    summaryLayout->setSpacing(0); // Per far toccare i bordi delle celle
    summaryLayout->setContentsMargins(0, 0, 0, 0);
    summaryWg->setAttribute(Qt::WA_TranslucentBackground);

    // Helper for summary creation
    auto addSummaryItem = [&](QString label, QString value, QString status, int row, int col) {
        QWidget *cell = new QWidget();
        cell->setObjectName(("summaryCell" + label).remove(" "));

        QHBoxLayout *l = new QHBoxLayout(cell);
        l->setContentsMargins(8, 6, 8, 6);

        QLabel *nameLbl = new QLabel(label);
        nameLbl->setObjectName("summaryName");

        QLabel *valLbl = new QLabel(value);
        valLbl->setObjectName("summaryValue");
        valLbl->setProperty("status", status);

        l->addWidget(nameLbl);
        l->addStretch();
        l->addWidget(valLbl);
        m_summaryLabels[label] = valLbl;
        summaryLayout->addWidget(cell, row, col);
    };

    // Grid layout column based
    // TODO use enum for IDS
    addSummaryItem("EXPAND", "FAKE", "on", 0, 0);
    addSummaryItem("PLOT DETAIL", "FAKE", "on", 0, 1);
    addSummaryItem("CH INPUT", "FAKE", "on", 1, 0);
    addSummaryItem("STIMULUS", "FAKE", "auto", 1, 1);

    mainLayout->addWidget(summaryWg);
    externalLayout->addWidget(mainWrapper);
}

void MultipleChannelControlDockWidget::updateSummary(const QString &id, const QString &text, const QString &status) {
    if (m_summaryLabels.count(id)) {
        m_summaryLabels[id]->setText(text);
        m_summaryLabels[id]->setProperty("status", status);
        m_summaryLabels[id]->style()->unpolish(m_summaryLabels[id]);
        m_summaryLabels[id]->style()->polish(m_summaryLabels[id]);
    }
}

void MultipleChannelControlDockWidget::setSelectionCount(int count) {
    m_selectionCounterLabel->setText(QString(
        "<span style='color:#0078d4;'>●</span> %1 Selected "
        "<span style='color:#666666;'> ●</span> %2 Total"
        ).arg(count).arg("-"));

    m_selectionCounterLabel->setProperty("empty", count == 0);
    m_selectionCounterLabel->style()->unpolish(m_selectionCounterLabel);
    m_selectionCounterLabel->style()->polish(m_selectionCounterLabel);
}

void MultipleChannelControlDockWidget::setChannelsAuto(bool flag) {
    if (switchChannelsAutoBtn != nullptr) {
        switchChannelsAutoBtn->setChecked(flag);
    }
}

void MultipleChannelControlDockWidget::setStimulusAuto(bool flag) {
    if (turnStimulusAutoBtn != nullptr) {
        turnStimulusAutoBtn->setChecked(flag);
    }
}

void MultipleChannelControlDockWidget::setExpandAuto(bool flag) {
    expandTraceAutoBtn->setChecked(flag);
}

void MultipleChannelControlDockWidget::setPlotDetailAuto(bool flag) {
    plotDetailAutoBtn->setChecked(flag);
}

bool MultipleChannelControlDockWidget::getExpertMode() {
    return offsetCorrectionExpertChb->isChecked();
}

void MultipleChannelControlDockWidget::enableExpertMode(bool flag) {
    offsetCorrectionExpertChb->setEnabled(flag);
}

void MultipleChannelControlDockWidget::onSetClampingModality(ClampingModality_t clampingModality) {
    switch (clampingModality) {
    case ClampingModality_t::VOLTAGE_CLAMP:
        if (zapGb!= nullptr) {
            zapGb->setEnabled(true);
        }
        break;

    case ClampingModality_t::CURRENT_CLAMP:
    case ClampingModality_t::ZERO_CURRENT_CLAMP:
        if (zapGb!= nullptr) {
            zapGb->setEnabled(false);
        }
        break;
    }
}
