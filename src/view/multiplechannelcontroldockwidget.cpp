#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>
#include <QGroupBox>
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
    }

    RangedMeasurement_t zapDurationRange;
    if (msgDisp->getZapFeatures(zapDurationRange) == Success) {
        auto gb = new QGroupBox(QString::fromStdString("Zap pulse"));
        auto qhbl = new QHBoxLayout();
        gb->setLayout(qhbl);
        mainLayout->addWidget(gb);
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

    //widget that occupies as much space as possible for better visual effect
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    mainLayout->addWidget(spacer);
    
    expandTraceBtn = new QPushButton("ON (E)");
    connect(expandTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigAddToBigPlot);
    qhblExpandTrace->addWidget(expandTraceBtn);
    reduceTraceBtn = new QPushButton("OFF");
    connect(reduceTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigRemoveFromBigPlot);
    qhblExpandTrace->addWidget(reduceTraceBtn);
}

bool MultipleChannelControlDockWidget::getExpertMode() {
    return offsetCorrectionExpertChb->isChecked();
}

void MultipleChannelControlDockWidget::enableExpertMode(bool flag) {
    offsetCorrectionExpertChb->setEnabled(flag);
}
