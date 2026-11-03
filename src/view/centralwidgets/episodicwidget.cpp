#include "episodicwidget.h"
#include <QSplitter>
#include <QGroupBox>
#include <QSettings>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>

EpisodicWidget::EpisodicWidget(BigPlot* plot, QWidget* parent):
    QWidget(parent) {
    this->setObjectName("episodic");
    this->setWindowTitle("Episodic");

    auto outerLayout = new QVBoxLayout(this);
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    outerLayout->addWidget(splitter);

    auto sideWidget = new QWidget();
    splitter->addWidget(sideWidget);
    auto sideLayout = new QVBoxLayout(sideWidget);
    sideLayout->setContentsMargins(18, 10, 10, 10);
    sideLayout->setSpacing(12);

    splitter->addWidget(plot);
    splitter->addWidget(sideWidget);

    // TOP section - CONTROLS
    auto controlsTitle = new QLabel("CONTROLS", sideWidget);
    controlsTitle->setObjectName("controlsTitle");
    sideLayout->addWidget(controlsTitle);

    auto controlsRow = new QHBoxLayout();
    controlsRow->setAlignment(Qt::AlignLeft);
    controlsRow->setSpacing(8);

    autoZoom = new QPushButton("   ZOOM");
    autoZoom->setObjectName("autoZoom");
    autoZoom->setToolTip("Auto zoom");
    autoZoom->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    autoZoom->setCursor(Qt::PointingHandCursor);
    controlsRow->addWidget(autoZoom);
    sideLayout->addLayout(controlsRow);
    connect(autoZoom, &QPushButton::clicked, this, &EpisodicWidget::sigAutoZoom);

    // Separator
    QFrame* line = new QFrame(sideWidget);
    line->setObjectName("lineSeparator");
    line->setFrameShape(QFrame::HLine);
    sideLayout->addWidget(line);

    recordingWg = new RecordingWidget(sideWidget,
                                      GLB_EPISODIC_RECORD_NAME_TAG,
                                      GLB_EPISODIC_RECORD_PATH_TAG,
                                      PSD_DEFAULT_RECORD_NAME,
                                      PSD_DEFAULT_RECORD_PATH);


    // CONNECTIONS
    connect(recordingWg, &RecordingWidget::sigRecordingStarted, this, &EpisodicWidget::sigStartRecording);
    connect(recordingWg, &RecordingWidget::sigRecordingStopped, this, &EpisodicWidget::sigStopRecording);

    // Propagate event to ABF writer
    connect(recordingWg, &RecordingWidget::sigFileNameChanged, this, [=](QString value) {
        emit sigFileNameChanged(value);
    });
    connect(recordingWg, &RecordingWidget::sigRecordPathChanged,this, [=](QString value) {
        emit sigRecordPathChanged(value);
    });

    sideLayout->addWidget(recordingWg);

    // Vertical sapcer
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    sideLayout->addWidget(spacer);

    splitter->addWidget(sideWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
}


void EpisodicWidget::setRecording(bool flag) {
    recordingWg->setRecording(flag);
}
