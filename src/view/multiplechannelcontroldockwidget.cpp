#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>

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

    auto recordingGb = new QGroupBox(QString::fromStdString("Recording"));
    auto recordingVBoxLayout = new QVBoxLayout();
    auto qhBoxLayout = new QHBoxLayout();
    recordingVBoxLayout->addLayout(qhBoxLayout);

    recordingGb->setLayout(recordingVBoxLayout);
    mainLayout->addWidget(recordingGb);
    recordingStartBtn = new QPushButton("START");
    connect(recordingStartBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStartRecording);
    qhBoxLayout->addWidget(recordingStartBtn);
    recordingStopBtn = new QPushButton("STOP");

    auto hboxRecordingPath = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxRecordingPath);

    auto hboxBrowseFile = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxBrowseFile);

    auto hboxFileName = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxFileName);


    hboxRecordingPath->addWidget(new QLabel("Recording path:"), 0, 0);
    auto recordPathLineEdit = new QLineEdit();
    recordPathLineEdit->setReadOnly(true);
    hboxRecordingPath->addWidget(recordPathLineEdit);

    auto browseBtn = new QPushButton("Browse");
    connect(browseBtn, &QPushButton::clicked, [=](){
        // Open a directory selection dialog
        QString directoryPath = QFileDialog::getExistingDirectory(this,
                                                                  "Select Directory",
                                                                  QDir::homePath(),
                                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        // Check if the user selected a directory
        if (!directoryPath.isEmpty()) {
            auto path = directoryPath + "/";
            QSettings settings;
            settings.setValue(GLB_PROTOCOL_RECORD_PATH_TAG, path);
            recordPathLineEdit->setText(path);
            emit sigRecordPathChanged(path);
        }
    });
    auto goToDirBtn = new QPushButton("Recordings");
    connect(goToDirBtn, &QPushButton::clicked, [=](){
        // Open a directory selection dialog
        QUrl folderUrl = QUrl::fromLocalFile(recordPathLineEdit->text());
        QDir folderDir(recordPathLineEdit->text());
        if (folderDir.exists()){
            QDesktopServices::openUrl(folderUrl);
        } else {
            QMessageBox::information(nullptr, "Warning", "This Path seems to be incorrect.");
        }
    });
    hboxBrowseFile->addWidget(browseBtn);
    hboxBrowseFile->addWidget(goToDirBtn);

    hboxFileName->addWidget(new QLabel("File name:"), 0, 0);
    auto fileNameLineEdit = new QLineEdit();
    hboxFileName->addWidget(fileNameLineEdit);

    QSettings settings;
    recordPathLineEdit->setText(settings.value(GLB_PROTOCOL_RECORD_PATH_TAG, PSD_DEFAULT_RECORD_PATH).toString());
    fileNameLineEdit->setText(settings.value(GLB_PROTOCOL_RECORD_NAME_TAG, PSD_DEFAULT_RECORD_NAME).toString());

    QPixmap pixmapStop("://imgs/stop protocol.png");
    QIcon stopRecordIcon(pixmapStop);
    recordingStopBtn->setIcon(stopRecordIcon);
    connect(recordingStopBtn, &QPushButton::clicked, this, &MultipleChannelControlDockWidget::sigStopRecording);
    qhBoxLayout->addWidget(recordingStopBtn);
    this->setRecording(false);
    connect(fileNameLineEdit, &QLineEdit::editingFinished, [=](){
       QSettings settings;
       auto filename = fileNameLineEdit->text();
       settings.setValue(GLB_PROTOCOL_RECORD_NAME_TAG, filename);
       emit sigFileNameChanged(filename);
    });
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
