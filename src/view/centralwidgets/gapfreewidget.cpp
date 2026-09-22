#include "gapfreewidget.h"
#include <QSplitter>
#include <QGroupBox>
#include <QSettings>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QFrame>
#include <QStyle>
#include <qcheckbox.h>
#include <qformlayout.h>
#include "globaldefines.h"
#include "bigplot.h"

GapFreeWidget::GapFreeWidget(BigPlot* plot, QWidget* parent):
    QWidget(parent) {
    this->setObjectName("gapFree");
    this->setWindowTitle("GapFree");

    auto outerLayout = new QVBoxLayout(this);
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    outerLayout->addWidget(splitter);
    splitter->addWidget(plot);

    auto sideWidget = new QWidget(splitter);
    auto mainLayout = new QVBoxLayout(sideWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(12);

    // --- Section 1. CONTROLS
    auto controlsTitle = new QLabel("CONTROLS", sideWidget);
    controlsTitle->setObjectName("controlsTitle");
    mainLayout->addWidget(controlsTitle);

    auto controlsRow = new QHBoxLayout();
    controlsRow->setAlignment(Qt::AlignLeft);
    controlsRow->setSpacing(8);

    autoZoom = new QPushButton("   ZOOM");
    autoZoom->setObjectName("autoZoom");
    autoZoom->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    autoZoom->setCursor(Qt::PointingHandCursor);

    btnZoomIn = new QPushButton(sideWidget);
    btnZoomIn->setObjectName("zoomInBtn");
    btnZoomIn->setFixedSize(32, 32);
    btnZoomIn->setToolTip("Zoom In");

    btnZoomOut = new QPushButton(sideWidget);
    btnZoomOut->setObjectName("zoomOutBtn");
    btnZoomOut->setFixedSize(32, 32);
    btnZoomOut->setToolTip("Zoom Out");

    controlsRow->addWidget(autoZoom);
    controlsRow->addWidget(btnZoomIn);
    controlsRow->addWidget(btnZoomOut);
    controlsRow->addStretch();

    mainLayout->addLayout(controlsRow);

    // Zoom signals
    connect(autoZoom, &QPushButton::clicked, this, &GapFreeWidget::sigAutoZoom);

    if (plot != nullptr) {
        connect(btnZoomIn, &QPushButton::clicked, plot, [plot]() {
            plot->zoomInFactor(0.8);
        });

        connect(btnZoomOut, &QPushButton::clicked, plot, [plot]() {
            plot->zoomOutFactor(1.25);
        });
    }

    // Separator
    QFrame* line = new QFrame(sideWidget);
    line->setObjectName("lineSeparator");
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    // --- Section 2. RECORDING
    auto recordingTitle = new QLabel("RECORDING", sideWidget);
    recordingTitle->setObjectName("recordingTitle");
    mainLayout->addWidget(recordingTitle);

    // Row 1 - start/stop - timer - settings
    auto recRow1 = new QHBoxLayout();
    recRow1->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    recRow1->setSpacing(6);

    startStopBtn = new QPushButton(sideWidget);
    startStopBtn->setObjectName("startStopBtn");
    startStopBtn->setFixedWidth(60);
    startStopBtn->setFixedHeight(28);
    startStopBtn->setCheckable(true);
    startStopBtn->setToolTip("Start or stop recording");

    // TimerDisplay
    protocolTimer = new TimerDisplay(this, "hh.mm.ss");

    auto btnSettings = new QPushButton(sideWidget);
    btnSettings->setObjectName("btnSettings");
    btnSettings->setToolTip("Open settings");
    btnSettings->setCheckable(true);
    btnSettings->setFixedSize(28, 28);

    recRow1->addWidget(startStopBtn);
    recRow1->addWidget(protocolTimer);
    recRow1->addWidget(btnSettings);
    recRow1->addStretch();
    mainLayout->addLayout(recRow1);

    // Start/stop recording
    connect(startStopBtn, &QPushButton::clicked, this, [=](bool checked) {
        if (checked) {
            startStopBtn->setProperty("running", true);
            protocolTimer->onStartTimer();
            emitFileName();
            emitFilePath();
            emit sigStartRecording();
        } else {
            startStopBtn->setProperty("running", false);
            protocolTimer->onStopTimer();
            emit sigStopRecording();
        }

        startStopBtn->style()->unpolish(startStopBtn);
        startStopBtn->style()->polish(startStopBtn);
    });

    // Row 2 - set location and recordigns buttons
    auto recRow2 = new QHBoxLayout();
    recRow2->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    recRow2->setSpacing(6);

    browseBtn = new QPushButton(" LOCATION", sideWidget);
    browseBtn->setObjectName("locationBtn");
    browseBtn->setToolTip("Set save location");

    auto goToDirBtn = new QPushButton(" RECORDINGS", sideWidget);
    goToDirBtn->setObjectName("recordingsBtn");
    goToDirBtn->setToolTip("Open recordings folder");

    recRow2->addWidget(browseBtn);
    recRow2->addWidget(goToDirBtn);
    recRow2->addStretch();
    mainLayout->addLayout(recRow2);

    // --- Section 3. SETTINGS PANEL
    auto settingsContainer = new QWidget(sideWidget);
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
    recordPathLineEdit->setText(settings.value(GLB_PROTOCOL_RECORD_PATH_TAG, PSD_DEFAULT_RECORD_PATH).toString());
    fileNameLineEdit->setText(settings.value(GLB_PROTOCOL_RECORD_NAME_TAG, PSD_DEFAULT_RECORD_NAME).toString());

    QDir directory(recordPathLineEdit->text());
    if (!directory.exists()) {
        directory.mkpath(".");
    }

    // --- CONNECTIONS
    connect(btnSettings, &QPushButton::clicked, this, [=]() {
        settingsContainer->setVisible(!settingsContainer->isVisible());
    });

    connect(startStopBtn, &QPushButton::clicked, this, [=](bool checked) {
        if (checked) {
            startStopBtn->setProperty("running", true);
            protocolTimer->onStartTimer();
            emitFileName();
            emitFilePath();
            emit sigStartRecording();
        } else {
            startStopBtn->setProperty("running", false);
            protocolTimer->onStopTimer();
            emit sigStopRecording();
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
            QMessageBox::information(nullptr, "Warning", "This Path seems to be incorrect.");
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

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    mainLayout->addWidget(spacer);

    splitter->addWidget(sideWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
}

void GapFreeWidget::emitFilePath() {
    QSettings settings;
    auto filePath = recordPathLineEdit->text();
    settings.setValue(GLB_PROTOCOL_RECORD_PATH_TAG, filePath);
    emit sigRecordPathChanged(filePath);
}

void GapFreeWidget::emitFileName() {
    QSettings settings;
    auto filename = fileNameLineEdit->text();
    settings.setValue(GLB_PROTOCOL_RECORD_NAME_TAG, filename);
    emit sigFileNameChanged(filename);
}

// Enable/disable UI componets based on recording stataus
void GapFreeWidget::lockUnlockSettings(bool isRecording) {
    browseBtn->setEnabled(!isRecording);
    fileNameLineEdit->setEnabled(!isRecording);
    recordPathLineEdit->setEnabled(!isRecording);
};

// Manages the timer and start button
void GapFreeWidget::setRecording(bool flag) {
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
