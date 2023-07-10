#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>

MultipleChannelControlDockWidget::MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setWindowTitle("Multiple channel controls");
    setObjectName("multipleChannelControlsDw");
    this->setWidget(mainWg);

//    QVBoxLayout * mainVl = new QVBoxLayout();
//    mainVl->setContentsMargins(0, 0, 0, 0);
//    mainVl->setSpacing(1);
//    mainWg->setLayout(mainVl);

    QGridLayout * mainGl = new QGridLayout;
    mainWg->setLayout(mainGl);

    int rowIdx = 0;

    if (msgDisp->hasChannelSwitches() == Success) {
        mainGl->addWidget(new QLabel("Channels input"), rowIdx, 0, Qt::AlignRight);
        switchChannelsOnBtn = new QPushButton("ON");
        connect(switchChannelsOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOn);
        mainGl->addWidget(switchChannelsOnBtn, rowIdx, 1);
        switchChannelsOffBtn = new QPushButton("OFF");
        connect(switchChannelsOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnChannelOff);
        mainGl->addWidget(switchChannelsOffBtn, rowIdx, 2);
        rowIdx++;
    }

    if (msgDisp->hasStimulusSwitches() == Success) {
        mainGl->addWidget(new QLabel("Stimulus"), rowIdx, 0, Qt::AlignRight);
        turnStimulusOnBtn = new QPushButton("ON");
        connect(turnStimulusOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOn);
        mainGl->addWidget(turnStimulusOnBtn, rowIdx, 1);
        turnStimulusOffBtn = new QPushButton("OFF");
        connect(turnStimulusOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnStimulsOff);
        mainGl->addWidget(turnStimulusOffBtn, rowIdx, 2);
        rowIdx++;
    }

    if (msgDisp->hasOffsetCompensation() == Success) {
        mainGl->addWidget(new QLabel("Offset compensation"), rowIdx, 0, Qt::AlignRight);
        offsetCompensationOnBtn = new QPushButton("ON");
        connect(offsetCompensationOnBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnDocOn);
        mainGl->addWidget(offsetCompensationOnBtn, rowIdx, 1);
        offsetCompensationOffBtn = new QPushButton("OFF");
        connect(offsetCompensationOffBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigTurnDocOff);
        mainGl->addWidget(offsetCompensationOffBtn, rowIdx, 2);
        rowIdx++;
    }

    mainGl->addWidget(new QLabel("Expand trace"), rowIdx, 0, Qt::AlignRight);
    expandTraceBtn = new QPushButton("ON");
    connect(expandTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigAddToBigPlot);
    mainGl->addWidget(expandTraceBtn, rowIdx, 1);
    reduceTraceBtn = new QPushButton("OFF");
    connect(reduceTraceBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigRemoveFromBigPlot);
    mainGl->addWidget(reduceTraceBtn, rowIdx, 2);
    rowIdx++;

    mainGl->addWidget(new QLabel("Recording"), rowIdx, 0, Qt::AlignRight);
    recordingStartBtn = new QPushButton("START");
    connect(recordingStartBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStartRecording);
    mainGl->addWidget(recordingStartBtn, rowIdx, 1);
    recordingStopBtn = new QPushButton("STOP");
    QPixmap pixmapStop("://imgs/stop protocol.png");
    QIcon stopRecordIcon(pixmapStop);
    recordingStopBtn->setIcon(stopRecordIcon);
    connect(recordingStopBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStopRecording);
    mainGl->addWidget(recordingStopBtn, rowIdx, 2);
    rowIdx++;

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
