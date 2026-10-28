#include "recordingwidget.h"
#include "globaldefines.h"
#include <QStyle>
#include <QSettings>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>

RecordingWidget::RecordingWidget(QWidget* parent,
                                 const char * defaultFileNameTag,
                                 const char * defaultFilePathTag,
                                 QString defaultFileName,
                                 QString defaultFilePath){

    mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

    this->defaultFileNameTag = defaultFileNameTag;
    this->defaultFilePathTag = defaultFilePathTag;

    auto recordingTitle = new QLabel("RECORDING", this);
    recordingTitle->setObjectName("recordingTitle");
    mainLayout->addWidget(recordingTitle);

    // Row 1 - start/stop - timer - settings
    auto recRow1 = new QHBoxLayout();
    recRow1->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    recRow1->setSpacing(6);

    startStopBtn = new QPushButton(this);
    startStopBtn->setText(" REC");
    startStopBtn->setObjectName("startRecordingButton");
    startStopBtn->setFixedWidth(65);
    startStopBtn->setFixedHeight(28);
    startStopBtn->setCheckable(true);
    startStopBtn->setToolTip("Start or stop recording");

    // TimerDisplay
    protocolTimer = new TimerDisplay(this, "hh.mm.ss");

    auto btnSettings = new QPushButton(this);
    btnSettings->setObjectName("btnSettings");
    btnSettings->setToolTip("Open settings");
    btnSettings->setCheckable(true);
    btnSettings->setFixedSize(28, 28);

    recRow1->addWidget(startStopBtn);
    recRow1->addWidget(protocolTimer);
    recRow1->addWidget(btnSettings);
    recRow1->addStretch();
    mainLayout->addLayout(recRow1);

    // Row 2 - set location and recordigns buttons
    auto recRow2 = new QHBoxLayout();
    recRow2->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    recRow2->setSpacing(6);

    browseBtn = new QPushButton(" SAVEPATH", this);
    browseBtn->setObjectName("locationBtn");
    browseBtn->setToolTip("Set save location");

    auto goToDirBtn = new QPushButton(" RECORDINGS", this);
    goToDirBtn->setObjectName("recordingsBtn");
    goToDirBtn->setToolTip("Open recordings folder");

    recRow2->addWidget(browseBtn);
    recRow2->addWidget(goToDirBtn);
    recRow2->addStretch();
    mainLayout->addLayout(recRow2);

    // --- Section 3. SETTINGS PANEL
    auto settingsContainer = new QWidget(this);
    settingsContainer->setObjectName("settingsContainer");
    auto settingsLayout = new QVBoxLayout(settingsContainer);
    settingsLayout->setContentsMargins(8, 0, 8, 0);
    settingsLayout->setSpacing(8);

    auto fileNameLayout = new QHBoxLayout();
    auto fileIco = new QLabel();
    fileIco->setObjectName("fileIco");
    auto fileNameLbl = new QLabel("Filename ");
    fileNameLbl->setObjectName("fileNameLbl");
    fileNameLineEdit = new QLineEdit(settingsContainer);
    fileNameLineEdit->setObjectName("fileNameLineEdit");

    fileNameLayout->addWidget(fileIco);
    fileNameLayout->addWidget(fileNameLbl);
    fileNameLayout->addWidget(fileNameLineEdit);
    fileNameLayout->addStretch();

    auto recordPathLayout = new QHBoxLayout();
    auto recordIco = new QLabel();
    recordIco->setObjectName("recordIco");
    auto recordPathLbl = new QLabel("Save path");
    recordPathLbl->setObjectName("recordPathLbl");
    recordPathLineEdit = new QLineEdit(settingsContainer);
    recordPathLineEdit->setObjectName("recordPathLineEdit");

    recordPathLayout->addWidget(recordIco);
    recordPathLayout->addWidget(recordPathLbl);
    recordPathLayout->addWidget(recordPathLineEdit);
    recordPathLayout->addStretch();

    settingsLayout->addLayout(fileNameLayout);
    settingsLayout->addLayout(recordPathLayout);

    settingsContainer->setVisible(false);
    mainLayout->addWidget(settingsContainer);

    // Loading default settings
    QSettings settings;
    recordPathLineEdit->setText(settings.value(defaultFilePathTag, defaultFilePath).toString());
    fileNameLineEdit->setText(settings.value(defaultFileNameTag, defaultFileName).toString());

    QDir directory(recordPathLineEdit->text());
    if (!directory.exists()) {
        directory.mkpath(".");
    }

    // CONNECTIONS
    connect(btnSettings, &QPushButton::clicked, this, [=]() {
        settingsContainer->setVisible(!settingsContainer->isVisible());
    });

    connect(startStopBtn, &QPushButton::clicked, this, [=](bool checked) {
        if (checked) {
            startStopBtn->setProperty("running", true);
            startStopBtn->setText(" STOP");
            protocolTimer->onStartTimer();
            emitFileName();
            emitFilePath();
            emit sigRecordingStarted();
        } else {
            startStopBtn->setProperty("running", false);
            startStopBtn->setText(" REC");
            protocolTimer->onStopTimer();
            emit sigRecordingStopped();
        }

        lockUnlockSettings(checked);

        startStopBtn->style()->unpolish(startStopBtn);
        startStopBtn->style()->polish(startStopBtn);
    });

    connect(browseBtn, &QPushButton::clicked, [=]() {
        QString directoryPath = QFileDialog::getExistingDirectory(
            this, "Select Directory", recordPathLineEdit->text(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!directoryPath.isEmpty()) {
            auto recordingsDirectoryPath = directoryPath + "/";
            recordPathLineEdit->setText(recordingsDirectoryPath);
            emitFilePath();
        }
    });

    connect(goToDirBtn, &QPushButton::clicked, [=]() {
        QUrl folderUrl = QUrl::fromLocalFile(recordPathLineEdit->text());
        QDir folderDir(recordPathLineEdit->text());
        if (folderDir.exists()) {
            QDesktopServices::openUrl(folderUrl);
        } else {
            QMessageBox::warning(nullptr, "Warning", folderUrl.toLocalFile() + " seems to be incorrect.");
        }
    });

    connect(fileNameLineEdit, &QLineEdit::editingFinished, [=]() {
        emitFileName();
        fileNameLineEdit->clearFocus();
    });

    connect(recordPathLineEdit, &QLineEdit::editingFinished, [=]() {
        emitFilePath();
        recordPathLineEdit->clearFocus();
    });

}

void RecordingWidget::emitFilePath() {
    QSettings settings;
    auto filePath = recordPathLineEdit->text();

    settings.setValue(QAnyStringView(defaultFilePathTag), filePath);
    emit sigRecordPathChanged(filePath);
}

void RecordingWidget::emitFileName() {
    QSettings settings;
    auto filename = fileNameLineEdit->text();
    settings.setValue(QAnyStringView(defaultFileNameTag), filename);
    emit sigFileNameChanged(filename);
}


void RecordingWidget::lockUnlockSettings(bool isRecording) {
    browseBtn->setEnabled(!isRecording);
    fileNameLineEdit->setEnabled(!isRecording);
    recordPathLineEdit->setEnabled(!isRecording);
};

void RecordingWidget::setRecording(bool flag) {
    startStopBtn->setChecked(flag);
    startStopBtn->setProperty("running", flag);
    startStopBtn->style()->unpolish(startStopBtn);
    startStopBtn->style()->polish(startStopBtn);

    if (flag) {
        protocolTimer->onStartTimer();
    } else {
        protocolTimer->onStopTimer();
    }

    lockUnlockSettings(flag);
}

