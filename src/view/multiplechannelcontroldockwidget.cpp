#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>
#include <QSettings>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QStyle>
#include <QSizePolicy>
#include <QTimer>
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QScrollArea>
#include "globaldefines.h"

MultipleChannelControlDockWidget::MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    setAttribute(Qt::WA_TranslucentBackground);
    this->setObjectName("MultipleChannelControlDockWidget");
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    // Custom TitleBar widget
    QWidget* customTitleBar = new QWidget();
    customTitleBar->setObjectName("customTitleBar");
    QHBoxLayout* titleLayout = new QHBoxLayout(customTitleBar);
    titleLayout->setContentsMargins(10, 5, 10, 5);

    QLabel* titleLabel = new QLabel("MULTIPLE CHANNEL CONTROLS");
    titleLabel->setObjectName("titleLabel");

    // Custom Window buttons - expand & close
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

    // External Panel Layout (3 sections)
    // TOP - Custom title bar
    // CENTRAL - Scroll area section
    // BOTTOM - Summary section
    QVBoxLayout* externalLayout = new QVBoxLayout(centralWidget);
    externalLayout->setContentsMargins(0, 0, 0, 0);
    externalLayout->setSpacing(0);

    setWindowTitle("Multiple channel controls");
    this->setObjectName("multipleChannelControlsDw");

    m_selectionCounterLabel = new QLabel();
    m_selectionCounterLabel->setObjectName("selectionCounter");
    m_selectionCounterLabel->setAlignment(Qt::AlignCenter);
    externalLayout->addWidget(m_selectionCounterLabel);

    // CENTRAL scroll area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    scrollArea->setMinimumHeight(0);
    scrollArea->setObjectName("centralScrollArea");

    // Scroll area main frame
    QFrame* mainWrapper = new QFrame();
    mainWrapper->setObjectName("mainWrapper");
    mainWrapper->setFrameStyle(QFrame::Panel | QFrame::Raised);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWrapper);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Regola i margini interni
    mainLayout->setSpacing(0);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    scrollArea->setWidget(mainWrapper);

    // External layout embeds scroll area
    externalLayout->addWidget(scrollArea);

    // Helper lambda for panel sub-property creation in central area
    auto addPropertyRow = [&](ChannelProperty property, AutoToggle* &autoTgl, QLabel* &badge,
                              QPushButton* &onBtn, QPushButton* &offBtn, QVBoxLayout* layout) {
        QFrame* rowContainer = new QFrame();
        rowContainer->setObjectName("propertyContainer");
        auto vbl = new QVBoxLayout(rowContainer);
        vbl->setContentsMargins(0, 5, 0, 10);
        vbl->setSpacing(4);

        // Row 1 - Title + AutoToggle
        auto r1 = new QHBoxLayout();
        QLabel * titleLbl = new QLabel(channelPropertyName[property]);
        titleLbl->setObjectName("propertyName");
        r1->addWidget(titleLbl);

        if ((property == STIMULUS) | (property == CH_INPUT)){
            QLabel * infoBox = new QLabel();
            infoBox->setObjectName("infoBox");
            infoBox->setToolTip(QString("Status badge is showed when feature is OFF."));
            infoBox->setFixedSize(14, 14);
            r1->addWidget(infoBox);
        }

        r1->addStretch();
        autoTgl = new AutoToggle();
        r1->addWidget(autoTgl);
        vbl->addLayout(r1);

        // Row 2 - Badge + ON/OFF
        auto r2 = new QHBoxLayout();

        // Custom Badge (E, X, P, O, etc.)
        badge = new QLabel(channelPropertyBadge[property]);
        badge->setObjectName("propertyBadge");
        badge->setProperty("propertyValue", channelPropertyBadge[property]);
        badge->setFixedSize(18, 18);
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

    // -- SCROLL AREA SECTIONS --
    // Section - VISIBILITY
    QFrame* visibilityHeader = new QFrame();
    visibilityHeader->setObjectName("sectionHeaderContainer");
    QHBoxLayout* visibilityHeaderLayout = new QHBoxLayout(visibilityHeader);
    visibilityHeaderLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *visibilityLbl = new QLabel("VISIBILITY & PLOTTING");
    visibilityLbl->setObjectName("sectionHeader");
    visibilityHeaderLayout->addWidget(visibilityLbl);
    mainLayout->addWidget(visibilityHeader);

    // Property - Expand Trace
    QLabel *expandTraceBadge;
    addPropertyRow(EXPAND, expandTraceAutoBtn, expandTraceBadge, expandTraceBtn, reduceTraceBtn, mainLayout);
    connect(expandTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigAddToBigPlot);
    connect(reduceTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigRemoveFromBigPlot);
    connect(expandTraceAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigAddToBigPlotAuto);

    // Property - Plot detail
    QLabel *plotDetailBadge;
    addPropertyRow(PLOT_DETAIL, plotDetailAutoBtn, plotDetailBadge, expandChannelDetailBtn, reduceChannelDetailBtn, mainLayout);
    connect(expandChannelDetailBtn, &QPushButton::clicked, this, [=]() {
        emit sigAddRemovePlotDetail(true);
    });
    connect(reduceChannelDetailBtn, &QPushButton::clicked, this, [=]() {
        emit sigAddRemovePlotDetail(false);
    });
    connect(plotDetailAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigAddPlotDetailAuto);

    // Section - SIGNAL (3 properties available)
    RangedMeasurement_t zapDurationRange;
    bool activeChInput = msgDisp->hasChannelSwitches() == Success;
    bool activeStimulus = msgDisp->hasStimulusSwitches() == Success;
    bool hasZap = msgDisp->getZapFeatures(zapDurationRange) == Success;

    if (activeChInput | activeStimulus | hasZap){
        QFrame* signalHeader = new QFrame();
        signalHeader->setObjectName("sectionHeaderContainer");
        QHBoxLayout* signalHeaderLayout = new QHBoxLayout(signalHeader);
        signalHeaderLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *signalLbl = new QLabel("SIGNAL");
        signalLbl->setObjectName("sectionHeader");
        signalHeaderLayout->addWidget(signalLbl);
        mainLayout->addWidget(signalHeader);
    }

    if (activeChInput) {
        QLabel *chInputBadge;
        addPropertyRow(CH_INPUT, switchChannelsAutoBtn, chInputBadge, switchChannelsOnBtn, switchChannelsOffBtn, mainLayout);
        connect(switchChannelsOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOn);
        connect(switchChannelsOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOff);
        connect(switchChannelsAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigTurnChannelAuto);
    }

    // Property - Stimulus
    if (activeStimulus) {
        QLabel *stimulusBadge;
        addPropertyRow(STIMULUS, turnStimulusAutoBtn, stimulusBadge, turnStimulusOnBtn, turnStimulusOffBtn, mainLayout);
        connect(turnStimulusOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOn);
        connect(turnStimulusOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOff);
        connect(turnStimulusAutoBtn, &AutoToggle::toggled, this, &MultipleChannelControlDockWidget::sigTurnStimulusAuto);
    }

    // Property - Zap pulse
    if (hasZap) {
        auto zapRow = new QFrame();
        zapRow->setObjectName("propertyContainer");
        auto zapVbl = new QVBoxLayout(zapRow);
        zapVbl->setContentsMargins(0, 10, 0, 10);
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
        auto zapSbx = new QDoubleSpinBox();
        zapDurationRange.convertValues(UnitPfxMilli);
        zapSbx->setRange(zapDurationRange.min, zapDurationRange.max);
        zapSbx->setValue(100.0);
        zapSbx->setProperty("technical", true);
        zapSbx->setMinimumWidth(80);
        zapDurationRow->addWidget(zapSbx);
        zapDurationRow->addWidget(new QLabel("ms"));
        zapBtn = new QPushButton("ZAP");
        zapDurationRow->addWidget(zapBtn);
        zapVbl->addLayout(zapDurationRow);
        mainLayout->addWidget(zapRow);

        connect(zapBtn, &QPushButton::clicked, this, [=] () {
            emit sigZap({zapSbx->value(), UnitPfxMilli, "s"});
        });
    }

    // Section - CALIBRATION RESISTORS
    if (msgDisp->hasCalSw() == Success && debugControlsEnabled()) {

        QFrame* calibHeader = new QFrame();
        calibHeader->setObjectName("sectionHeaderContainer");
        QHBoxLayout* calibHeaderLayout = new QHBoxLayout(calibHeader);
        calibHeaderLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *calibHeaderLbl = new QLabel("CALIBRATION RESISTORS");
        calibHeaderLbl->setObjectName("sectionHeader");
        calibHeaderLayout->addWidget(calibHeaderLbl);
        mainLayout->addWidget(calibHeader);

        QFrame* rowContainer = new QFrame();
        rowContainer->setObjectName("propertyContainer");
        auto vbl = new QVBoxLayout(rowContainer);
        vbl->setContentsMargins(0, 10, 0, 10);
        vbl->setSpacing(4);

        // Row 2 - Badge + ON/OFF
        auto calibRow = new QHBoxLayout();

        // Custom Badge (R)
        QLabel *calibResistorsBadge;
        calibResistorsBadge = new QLabel(channelPropertyBadge[CALIB_RESISTORS]);
        calibResistorsBadge->setObjectName("propertyBadge");
        calibResistorsBadge->setProperty("propertyValue", channelPropertyBadge[CALIB_RESISTORS]);
        calibResistorsBadge->setFixedSize(18, 18);
        calibResistorsBadge->setAlignment(Qt::AlignCenter);

        calibrationResistorsOnBtn = new QPushButton("ON");
        connect(calibrationResistorsOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnCalibrationResistorsOn);
        calibrationResistorsOffBtn = new QPushButton("OFF");
        connect(calibrationResistorsOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnCalibrationResistorsOff);
        calibrationResistorsOnBtn->setFixedWidth(48);
        calibrationResistorsOffBtn->setFixedWidth(48);

        calibRow->addWidget(calibResistorsBadge);
        calibRow->addStretch();
        calibRow->addWidget(calibrationResistorsOnBtn);
        calibRow->addWidget(calibrationResistorsOffBtn);
        vbl->addLayout(calibRow);

        mainLayout->addWidget(rowContainer);
    }

    // Section - OFFSET CORRECTION
    if (msgDisp->hasOffsetCompensation() == Success) {
        QFrame* offsetHeaderContainer = new QFrame();
        offsetHeaderContainer->setObjectName("sectionHeaderContainer");
        QHBoxLayout* offsetHeaderLayout = new QHBoxLayout(offsetHeaderContainer);
        offsetHeaderLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *offsetHeader = new QLabel("OFFSET CORRECTION");
        offsetHeader->setObjectName("sectionHeader");

        offsetCorrectionMode = new QPushButton("Expert mode ▼");
        offsetCorrectionMode->setObjectName("modeToggleBtn");
        offsetCorrectionMode->setCheckable(true);

        offsetHeaderLayout->addWidget(offsetHeader);
        offsetHeaderLayout->addStretch();
        offsetHeaderLayout->addWidget(offsetCorrectionMode);
        mainLayout->addWidget(offsetHeaderContainer);

        //  Container for: BASIC MODE
        QFrame* basicContentFrame = new QFrame();
        basicContentFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        basicContentFrame->setObjectName("modeContainer");
        QVBoxLayout* basicVLayout = new QVBoxLayout(basicContentFrame);
        QHBoxLayout* basicRow = new QHBoxLayout();
        basicVLayout->setContentsMargins(10, 10, 10, 10);
        basicVLayout->setSizeConstraint(QLayout::SetMinimumSize);

        offsetCorrectionStartBtn = new QPushButton("START");
        offsetCorrectionStopBtn = new QPushButton("STOP");
        offsetCorrectionStartBtn->setFixedHeight(24);
        offsetCorrectionStopBtn->setFixedHeight(24);

        basicRow->addWidget(offsetCorrectionStartBtn);
        basicRow->addWidget(offsetCorrectionStopBtn);
        basicVLayout->addLayout(basicRow);

        connect(offsetCorrectionStartBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStartOffsetCorrection);
        connect(offsetCorrectionStopBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStopOffsetCorrection);
        mainLayout->addWidget(basicContentFrame);

        // Container for: EXPERT MODE
        QFrame* expertContentFrame = new QFrame();
        expertContentFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        expertContentFrame->setObjectName("modeContainer");
        QVBoxLayout* expertVLayout = new QVBoxLayout(expertContentFrame);
        expertVLayout->setContentsMargins(10, 10, 10, 10);
        expertVLayout->setSpacing(8);
        expertVLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // Helper for creating expert mode - (Recalibration & Liquid Junction)
        auto addExpertRow = [&](ChannelProperty property, QPushButton* on, QPushButton* off, QPushButton* reset) {
            QHBoxLayout* row = new QHBoxLayout();
            QLabel* iconLbl = new QLabel(channelPropertyBadge[property]);
            iconLbl->setObjectName("propertyBadge");
            iconLbl->setProperty("propertyValue", channelPropertyBadge[property]);
            iconLbl->setFixedSize(18, 18);
            iconLbl->setAlignment(Qt::AlignCenter);

            QLabel* nameLbl = new QLabel(channelPropertyName[property]);
            nameLbl->setObjectName("offsetRowLabel");

            on->setFixedWidth(60);
            off->setFixedWidth(60);
            reset->setFixedWidth(60);

            row->addWidget(iconLbl);
            row->addWidget(nameLbl);
            row->addStretch();
            row->addWidget(on);
            row->addWidget(off);
            row->addWidget(reset);
            expertVLayout->addLayout(row);
        };

        liquidJunctionCompensationOnBtn = new QPushButton("ON");
        connect(liquidJunctionCompensationOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnLjcOn);
        liquidJunctionCompensationOffBtn = new QPushButton("OFF");
        connect(liquidJunctionCompensationOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnLjcOff);
        liquidJunctionCompensationResetBtn = new QPushButton("RESET");
        connect(liquidJunctionCompensationResetBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigResetLj);

        offsetRecalibrationOnBtn = new QPushButton("ON");
        connect(offsetRecalibrationOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOn);
        offsetRecalibrationOffBtn = new QPushButton("OFF");
        connect(offsetRecalibrationOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnOffsetRecalibrationOff);
        offsetRecalibrationResetBtn = new QPushButton("RESET");
        connect(offsetRecalibrationResetBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigResetOffsetRecalibration);

        addExpertRow(RECALIBRATION, offsetRecalibrationOnBtn, offsetRecalibrationOffBtn, offsetRecalibrationResetBtn);
        addExpertRow(LIQUID_JUNCTION, liquidJunctionCompensationOnBtn, liquidJunctionCompensationOffBtn, liquidJunctionCompensationResetBtn);

        mainLayout->addWidget(expertContentFrame);
        expertContentFrame->setVisible(false);
        offsetCorrectionMode->setChecked(false);

        // Mode switch button logic (Basic or Expert)
        connect(offsetCorrectionMode, &QPushButton::toggled, this, [=](bool checked) {
            basicContentFrame->setVisible(!checked);
            expertContentFrame->setVisible(checked);
            offsetCorrectionMode->setText(checked ? "Basic mode ▲" : "Expert mode ▼");
            this->setMinimumWidth(mainWrapper->sizeHint().width() + 8);
        });
    }

    mainLayout->addStretch();

    // BOTTOM - Status summary
    QWidget *summaryWg = new QWidget();
    summaryWg->setObjectName("summaryContainer");
    QGridLayout *summaryLayout = new QGridLayout(summaryWg);
    summaryLayout->setSpacing(0);
    summaryLayout->setContentsMargins(0, 0, 0, 0);

    // Helper for summary creation
    auto addSummaryItem = [&](ChannelProperty property, QString value, QString status, int row, int col) {
        QWidget *cell = new QWidget();
        cell->setObjectName("summaryCell");
        // E, X, P, O, etc.
        cell->setProperty("value", channelPropertyBadge[property]);

        QHBoxLayout *l = new QHBoxLayout(cell);
        l->setContentsMargins(8, 6, 8, 6);

        QLabel *nameLbl = new QLabel(channelPropertyName[property]);
        nameLbl->setObjectName("summaryCellName");
        l->addWidget(nameLbl);
        l->addStretch();

        QLabel *valLbl = new QLabel(value);
        valLbl->setObjectName("summaryCellValue");
        valLbl->setProperty("status", status);
        l->addWidget(valLbl);

        if (((property == CH_INPUT) & !(activeChInput)) | ((property == STIMULUS) & !(activeStimulus))){
            QLabel *infoBox = new QLabel(value);
            infoBox->setObjectName("infoBox");
            infoBox->setToolTip(QString("Not available for connected device."));
            infoBox->setFixedSize(14, 14);
            l->addWidget(infoBox);
            valLbl->setText("N/A");
        }

        m_summaryLabels[channelPropertyId[property]] = valLbl;
        summaryLayout->addWidget(cell, row, col);
    };

    // Grid layout column based
    addSummaryItem(EXPAND, "-", "-", 0, 0);
    addSummaryItem(PLOT_DETAIL, "-", "-", 0, 1);
    addSummaryItem(CH_INPUT, "-", "-", 1, 0);
    addSummaryItem(STIMULUS, "-", "-", 1, 1);
    externalLayout->addWidget(summaryWg);
    scrollArea->setMinimumWidth(mainWrapper->sizeHint().width());
}

void MultipleChannelControlDockWidget::updateSummary(const ChannelProperty &propertyType, const QString &text, const QString &status) {
    QString propertyId = channelPropertyId[propertyType];    
    if (m_summaryLabels.count(propertyId)) {
        m_summaryLabels[propertyId]->setText(text);
        m_summaryLabels[propertyId]->setProperty("status", status);
        m_summaryLabels[propertyId]->style()->unpolish(m_summaryLabels[propertyId]);
        m_summaryLabels[propertyId]->style()->polish(m_summaryLabels[propertyId]);
    }
}

void MultipleChannelControlDockWidget::setSelectionCount(int count, int totalChannels) {
    m_selectionCounterLabel->setText(QString(
        "<span style='color:#0078d4;'>●</span> %1 Selected "
        "<span style='color:#666666;'> ●</span> %2 Total"
        ).arg(count).arg(totalChannels));

    m_selectionCounterLabel->setProperty("empty", count == 0);
    m_selectionCounterLabel->style()->unpolish(m_selectionCounterLabel);
    m_selectionCounterLabel->style()->polish(m_selectionCounterLabel);
}

/*!
  Disables or enables controls (buttons) based on flag. If flag is TRUE -> controls
  are disabled.
*/
void MultipleChannelControlDockWidget::enableDisableControls(ChannelProperty propertyType, bool flag){

    switch (propertyType)
    {
    case EXPAND:
        expandTraceBtn->setDisabled(flag);
        reduceTraceBtn->setDisabled(flag);
        break;
    case CH_INPUT:
        switchChannelsOnBtn->setDisabled(flag);
        switchChannelsOffBtn->setDisabled(flag);
        break;
    case PLOT_DETAIL:
        expandChannelDetailBtn->setDisabled(flag);
        reduceChannelDetailBtn->setDisabled(flag);
        break;
    case STIMULUS:
        turnStimulusOnBtn->setDisabled(flag);
        turnStimulusOffBtn->setDisabled(flag);
        break;
    case RECALIBRATION:
        offsetRecalibrationOnBtn->setDisabled(flag);
        break;
    case LIQUID_JUNCTION:
        liquidJunctionCompensationOnBtn->setDisabled(flag);
        break;
    case CALIB_RESISTORS:
        // Nothing to do
        break;
    }

}

void MultipleChannelControlDockWidget::setChannelsAuto(bool flag) {
    if (switchChannelsAutoBtn != nullptr) {
        switchChannelsAutoBtn->setChecked(flag);
        enableDisableControls(CH_INPUT, flag);
    }
}

void MultipleChannelControlDockWidget::setStimulusAuto(bool flag) {
    if (turnStimulusAutoBtn != nullptr) {
        turnStimulusAutoBtn->setChecked(flag);
        enableDisableControls(STIMULUS, flag);
    }
}

void MultipleChannelControlDockWidget::setExpandAuto(bool flag) {
    expandTraceAutoBtn->setChecked(flag);
    enableDisableControls(EXPAND, flag);
}

void MultipleChannelControlDockWidget::setPlotDetailAuto(bool flag) {
    plotDetailAutoBtn->setChecked(flag);
    enableDisableControls(PLOT_DETAIL, flag);
}

bool MultipleChannelControlDockWidget::getExpertMode() {
    return offsetCorrectionMode->isChecked();
}

void MultipleChannelControlDockWidget::enableExpertMode(bool flag) {
    offsetCorrectionMode->setEnabled(flag);
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
