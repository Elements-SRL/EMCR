#include "gapfreewidget.h"
#include <QSplitter>
#include <QGroupBox>
#include <QSettings>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>

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

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    mainLayout->addWidget(spacer);

    recordingStartBtn = new QPushButton("START");
    connect(recordingStartBtn, &QPushButton::clicked, this, [=]() {
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
    connect(browseBtn, &QPushButton::clicked, [=]() {
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
    connect(goToDirBtn, &QPushButton::clicked, [=]() {
        // Open a directory selection dialog
        QUrl folderUrl = QUrl::fromLocalFile(recordPathLineEdit->text());
        QDir folderDir(recordPathLineEdit->text());
        if (folderDir.exists()) {
            QDesktopServices::openUrl(folderUrl);
        }
        else {
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
    connect(recordingStopBtn, &QPushButton::clicked, this, &GapFreeWidget::sigStopRecording);
    qhBoxLayout->addWidget(recordingStopBtn);
    this->setRecording(false);
    connect(fileNameLineEdit, &QLineEdit::editingFinished, [=]() {
        QSettings settings;
        auto filename = fileNameLineEdit->text();
        settings.setValue(GLB_PROTOCOL_RECORD_NAME_TAG, filename);
        emit sigFileNameChanged(filename);
        });

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

void GapFreeWidget::setRecording(bool flag) {
    if (flag) {
        QPixmap pixmapRecors("://imgs/recording protocol.png");
        QIcon recordIcon(pixmapRecors);
        recordingStartBtn->setIcon(recordIcon);

    }
    else {
        QPixmap pixmapRecors("://imgs/record protocol.png");
        QIcon recordIcon(pixmapRecors);
        recordingStartBtn->setIcon(recordIcon);
    }
}