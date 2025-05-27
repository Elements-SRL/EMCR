#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>
#include <QSettings>
#include <QDesktopServices>
#include <QDoubleSpinBox>

MultipleChannelControlDockWidget::MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setWindowTitle("Multiple channel controls");
    setObjectName("multipleChannelControlsDw");
    this->setWidget(mainWg);

    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainWg->setLayout(mainLayout);
    if (msgDisp->hasChannelSwitches() == Success) {
        auto channels_input_gb = new QGroupBox(QString::fromStdString("Channels input"));
        auto qhblChannels_input = new QHBoxLayout();
        channels_input_gb->setLayout(qhblChannels_input);
        mainLayout->addWidget(channels_input_gb);
        switchChannelsOnBtn = new QPushButton("ON");
        connect(switchChannelsOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOn);
        qhblChannels_input->addWidget(switchChannelsOnBtn);
        switchChannelsOffBtn = new QPushButton("OFF (O)");
        connect(switchChannelsOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOff);
        qhblChannels_input->addWidget(switchChannelsOffBtn);
        switchChannelsAutoBtn = new QPushButton("AUTO");
        switchChannelsAutoBtn->setCheckable(true);
        connect(switchChannelsAutoBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelAuto);
        qhblChannels_input->addWidget(switchChannelsAutoBtn);
    }

    if (msgDisp->hasCalSw() == Success) {
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
        auto gb = new QGroupBox(QString::fromStdString("Stimulus"));
        auto qhbl = new QHBoxLayout();
        gb->setLayout(qhbl);
        mainLayout->addWidget(gb);
        turnStimulusOnBtn = new QPushButton("ON");
        connect(turnStimulusOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOn);
        qhbl->addWidget(turnStimulusOnBtn);
        turnStimulusOffBtn = new QPushButton("OFF (X)");
        connect(turnStimulusOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOff);
        qhbl->addWidget(turnStimulusOffBtn);
        turnStimulusAutoBtn = new QPushButton("AUTO");
        turnStimulusAutoBtn->setCheckable(true);
        connect(turnStimulusAutoBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulusAuto);
        qhbl->addWidget(turnStimulusAutoBtn);
    }

    RangedMeasurement_t zapDurationRange;
    if (msgDisp->getZapFeatures(zapDurationRange) == Success) {
        zapGb = new QGroupBox(QString::fromStdString("Zap pulse"));
        auto qhbl = new QHBoxLayout();
        zapGb->setLayout(qhbl);
        mainLayout->addWidget(zapGb);
        zapBtn = new QPushButton("ZAP");
        zapBtn->setCheckable(false);
        qhbl->addWidget(zapBtn);
        QDoubleSpinBox * zapSbx = new QDoubleSpinBox;
        zapDurationRange.convertValues(UnitPfxMilli);
        zapSbx->setRange(zapDurationRange.min, zapDurationRange.max);
        zapSbx->setValue(100.0);
        qhbl->addWidget(zapSbx);
        qhbl->addWidget(new QLabel("ms"));
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

    auto expandTraceGb = new QGroupBox(QString::fromStdString("Expand trace"));
    auto qhblExpandTrace = new QHBoxLayout();

    expandTraceGb->setLayout(qhblExpandTrace);
    mainLayout->addWidget(expandTraceGb);

    expandTraceBtn = new QPushButton("ON (E)");
    connect(expandTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigAddToBigPlot);
    qhblExpandTrace->addWidget(expandTraceBtn);
    reduceTraceBtn = new QPushButton("OFF");
    connect(reduceTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigRemoveFromBigPlot);
    qhblExpandTrace->addWidget(reduceTraceBtn);
    expandTraceAutoBtn = new QPushButton("AUTO");
    expandTraceAutoBtn->setCheckable(true);
    connect(expandTraceAutoBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigAddToBigPlotAuto);
    qhblExpandTrace->addWidget(expandTraceAutoBtn);


    // PlotDetail
    auto channelDetailGb = new QGroupBox(QString::fromStdString("Plot Detail"));
    auto qhblChannelDetail = new QHBoxLayout();

    channelDetailGb->setLayout(qhblChannelDetail);
    mainLayout->addWidget(channelDetailGb);

    auto expandChannelDetailBtn = new QPushButton("ON (P)");
    connect(expandChannelDetailBtn, &QPushButton::clicked, this, [=]() {
        emit sigAddRemovePlotDetail(true);
    });
    qhblChannelDetail->addWidget(expandChannelDetailBtn);
    auto reduceChannelDetailBtn = new QPushButton("OFF");
    connect(reduceChannelDetailBtn, &QPushButton::clicked, this, [=]() {
        emit sigAddRemovePlotDetail(false);
    });
    qhblChannelDetail->addWidget(reduceChannelDetailBtn);
    auto channelDetailAutoBtn = new QPushButton("AUTO");
    channelDetailAutoBtn->setCheckable(true);
    connect(channelDetailAutoBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigAddPlotDetailAuto);
    qhblChannelDetail->addWidget(channelDetailAutoBtn);
    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    mainLayout->addWidget(spacer);
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

bool MultipleChannelControlDockWidget::getExpertMode() {
    return offsetCorrectionExpertChb->isChecked();
}

void MultipleChannelControlDockWidget::enableExpertMode(bool flag) {
    offsetCorrectionExpertChb->setEnabled(flag);
}

void MultipleChannelControlDockWidget::onSetClampingModality(ClampingModality_t clampingModality) {
    switch (clampingModality) {
    case ClampingModality_t::VOLTAGE_CLAMP:
    case ClampingModality_t::VOLTAGE_CLAMP_VOLTAGE_READ:
        if (zapGb!= nullptr) {
            zapGb->setEnabled(true);
        }
        break;

    case ClampingModality_t::CURRENT_CLAMP:
    case ClampingModality_t::ZERO_CURRENT_CLAMP:
    case ClampingModality_t::CURRENT_CLAMP_CURRENT_READ:
        if (zapGb!= nullptr) {
            zapGb->setEnabled(false);
        }
        break;
    }
}
