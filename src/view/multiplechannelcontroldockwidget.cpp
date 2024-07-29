#include "multiplechannelcontroldockwidget.h"

#include <QBoxLayout>
#include <QGroupBox>
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

    auto recordingGb = new QGroupBox(QString::fromStdString("Recording"));
    auto recordingVBoxLayout = new QVBoxLayout();
    auto qhBoxLayout = new QHBoxLayout();
    recordingVBoxLayout->addLayout(qhBoxLayout);

    fileNameLineEdit = new QLineEdit();
    recordPathLineEdit = new QLineEdit();
    QSettings settings;
    recordPathLineEdit->setText(settings.value(GLB_PROTOCOL_RECORD_PATH_TAG, PSD_DEFAULT_RECORD_PATH).toString());
    fileNameLineEdit->setText(settings.value(GLB_PROTOCOL_RECORD_NAME_TAG, PSD_DEFAULT_RECORD_NAME).toString());

    recordingGb->setLayout(recordingVBoxLayout);
    mainLayout->addWidget(recordingGb);
    recordingStartBtn = new QPushButton("START");
    connect(recordingStartBtn, &QPushButton::clicked, this,  [=](){
        emitFileName();
        emitFilePath();
        emit sigStartRecording();
//     &MultipleChannelControlDockWidget::sigStartRecording
    });
    qhBoxLayout->addWidget(recordingStartBtn);
    recordingStopBtn = new QPushButton("STOP");

    auto hboxRecordingPath = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxRecordingPath);

    auto hboxBrowseFile = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxBrowseFile);

    auto hboxFileName = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxFileName);


    hboxRecordingPath->addWidget(new QLabel("Recording path:"), 0, 0);
    recordPathLineEdit->setReadOnly(true);
    hboxRecordingPath->addWidget(recordPathLineEdit);

    QDir directory(recordPathLineEdit->text());
    if (!directory.exists()) {
        // Create the directory
        if (directory.mkpath(".")) {
        }
    }
    auto browseBtn = new QPushButton("Change recordings directory");
    connect(browseBtn, &QPushButton::clicked, [=](){
        // Open a directory selection dialog
        QString directoryPath = QFileDialog::getExistingDirectory(this,
                                                                  "Select Directory",
                                                                  directory.absolutePath(),
                                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        // Check if the user selected a directory
        if (!directoryPath.isEmpty()) {
            auto recordingsDirectoryPath = directoryPath + "/";
            recordPathLineEdit->setText(recordingsDirectoryPath);
            emitFilePath();
        }
    });
    auto goToDirBtn = new QPushButton("Go to folder");
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
    hboxFileName->addWidget(fileNameLineEdit);

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

bool MultipleChannelControlDockWidget::getExpertMode() {
    return offsetCorrectionExpertChb->isChecked();
}

void MultipleChannelControlDockWidget::enableExpertMode(bool flag) {
    offsetCorrectionExpertChb->setEnabled(flag);
}

void MultipleChannelControlDockWidget::emitFilePath(){
    QSettings settings;
    auto filePath = recordPathLineEdit->text();
    settings.setValue(GLB_PROTOCOL_RECORD_PATH_TAG, filePath);
    emit sigRecordPathChanged(filePath);
}

void MultipleChannelControlDockWidget::emitFileName(){
    QSettings settings;
    auto filename = fileNameLineEdit->text();
    settings.setValue(GLB_PROTOCOL_RECORD_NAME_TAG, filename);
    emit sigFileNameChanged(filename);
}
