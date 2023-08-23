#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>
#include <QGroupBox>

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

    if (msgDisp->hasOffsetCompensation() == Success) {
        auto gb = new QGroupBox(QString::fromStdString("Offset compensation"));
        auto qhbl = new QHBoxLayout();
        gb->setLayout(qhbl);
        mainLayout->addWidget(gb);
        offsetCompensationOnBtn = new QPushButton("ON (C)");
        connect(offsetCompensationOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnDocOn);
        qhbl->addWidget(offsetCompensationOnBtn);
        offsetCompensationOffBtn = new QPushButton("OFF");
        connect(offsetCompensationOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnDocOff);
        qhbl->addWidget(offsetCompensationOffBtn);
        offsetCompensationResetBtn = new QPushButton("RESET");
        connect(offsetCompensationResetBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigResetDoc);
        qhbl->addWidget(offsetCompensationResetBtn);
    }

//    mainGl->addWidget(new QLabel("Expand trace"), rowIdx, 0, Qt::AlignRight);

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

    auto recording_gb = new QGroupBox(QString::fromStdString("Recording"));
    auto qhBoxLayout = new QHBoxLayout();

    recording_gb->setLayout(qhBoxLayout);
    mainLayout->addWidget(recording_gb);
    recordingStartBtn = new QPushButton("START");
    connect(recordingStartBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStartRecording);
    qhBoxLayout->addWidget(recordingStartBtn);
    recordingStopBtn = new QPushButton("STOP");
    QPixmap pixmapStop("://imgs/stop protocol.png");
    QIcon stopRecordIcon(pixmapStop);
    recordingStopBtn->setIcon(stopRecordIcon);
    connect(recordingStopBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStopRecording);
    qhBoxLayout->addWidget(recordingStopBtn);
    this->setRecording(false);
}

void MultipleChannelControlDockWidget::setRecording(bool flag) {
    if (flag) {
        QPixmap pixmapRecors("://imgs/recording protocol.png");
        QIcon recordIcon(pixmapRecors);
        recordingStartBtn->setIcon(recordIcon);

    } else {
        QPixmap pixmapRecors("://imgs/record protocol.png");
        QIcon recordIcon(pixmapRecors);
        recordingStartBtn->setIcon(recordIcon);
    }
}
