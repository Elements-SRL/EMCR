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
#include <QWheelEvent>
#include <QCoreApplication>
#include <qmenu.h>

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
    mainLayout->setContentsMargins(18, 10, 10, 10);
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
    autoZoom->setToolTip("Auto zoom");
    autoZoom->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    autoZoom->setCursor(Qt::PointingHandCursor);
    connect(autoZoom, &QPushButton::clicked, this, &GapFreeWidget::sigAutoZoom);

    controlsRow->addWidget(autoZoom);

    btnZoomIn = new QPushButton(sideWidget);
    btnZoomIn->setObjectName("zoomInBtn");
    btnZoomIn->setFixedSize(32, 32);
    btnZoomIn->setToolTip("Zoom In");

    btnZoomOut = new QPushButton(sideWidget);
    btnZoomOut->setObjectName("zoomOutBtn");
    btnZoomOut->setFixedSize(32, 32);
    btnZoomOut->setToolTip("Zoom Out");

    // --- Manual zoom controls
    auto manualZoom = new QFrame();
    manualZoom->setObjectName("manualZoomContainer");
    auto mZLayout = new QHBoxLayout(manualZoom);
    mZLayout->setContentsMargins(0,0,0,0);

    mZLayout->addWidget(btnZoomIn);
    mZLayout->addWidget(btnZoomOut);

    mainLayout->addLayout(controlsRow);

    // Dropdown multi selection for chosing axis
    auto btnSelectAxes = new QPushButton(sideWidget);
    btnSelectAxes->setObjectName("btnSelectAxes");
    auto axesMenu = new QMenu(btnSelectAxes);

    // Checkable axis options
    auto actX  = axesMenu->addAction("X - Time");
    auto actY1 = axesMenu->addAction("Y1 - Current");
    auto actY2 = axesMenu->addAction("Y2 - Voltage");

    for (auto act : {actX, actY1, actY2}) {
        act->setCheckable(true);
        act->setChecked(true);
    }

    // Updates selected axis text in dropdown
    auto updateButtonText = [=]() {
        QStringList selected;
        if (actX->isChecked())  selected << "X";
        if (actY1->isChecked())  selected << "Y1";
        if (actY2->isChecked())  selected << "Y2";

        if (selected.isEmpty()) {
            btnSelectAxes->setText("None");
        } else {
            btnSelectAxes->setText(selected.join(", "));
        }
    };

    updateButtonText();

    connect(actX,  &QAction::toggled, updateButtonText);
    connect(actY1, &QAction::toggled, updateButtonText);
    connect(actY2, &QAction::toggled, updateButtonText);

    btnSelectAxes->setMenu(axesMenu);
    mZLayout->addWidget(btnSelectAxes);

    controlsRow->addWidget(manualZoom);
    controlsRow->addStretch();

    // --- Manual zoom buttons logic (+/-)
    // --- Can zoom on single or multiple axis
    if (plot != nullptr) {
        auto sendWheelEvent = [plot](Qt::KeyboardModifier modifier, QPoint localPos, int deltaY) {
            QPointF globalPos = plot->mapToGlobal(localPos);

            QWheelEvent wheelEvent(
                localPos,
                globalPos,
                QPoint(0, 0),
                QPoint(0, deltaY),
                Qt::NoButton,
                modifier,
                Qt::NoScrollPhase,
                false
                );

            QCoreApplication::sendEvent(plot, &wheelEvent);
        };

        auto triggerSelectedZoom = [=](int deltaY) {
            int w = plot->width();
            int h = plot->height();

            // Mock mouse position
            QPoint centerPos(w * 0.5, h * 0.5);     // xBottom - Time
            QPoint leftHalfPos(w * 0.25, h * 0.5);  // yLeft   - Current
            QPoint rightHalfPos(w * 0.75, h * 0.5); // yRight  - Tension

            if (actX->isChecked()) {
                sendWheelEvent(Qt::ShiftModifier, centerPos, deltaY);
            }

            if (actY1->isChecked()) {
                sendWheelEvent(Qt::ControlModifier, leftHalfPos, deltaY);
            }

            if (actY2->isChecked()) {
                sendWheelEvent(Qt::ControlModifier, rightHalfPos, deltaY);
            }
        };

        connect(btnZoomIn, &QPushButton::clicked, plot, [triggerSelectedZoom]() {
            triggerSelectedZoom(120);
        });

        connect(btnZoomOut, &QPushButton::clicked, plot, [triggerSelectedZoom]() {
            triggerSelectedZoom(-120);
        });
    }

    // Separator
    QFrame* line = new QFrame(sideWidget);
    line->setObjectName("lineSeparator");
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    // --- Section 2. RECORDING
    recordingWg = new RecordingWidget(this,
                                      GLB_PROTOCOL_RECORD_NAME_TAG,
                                      GLB_PROTOCOL_RECORD_PATH_TAG,
                                      PSD_DEFAULT_RECORD_NAME,
                                      PSD_DEFAULT_RECORD_PATH);

    // CONNECTIONS
    connect(recordingWg, &RecordingWidget::sigRecordingStarted, this, &GapFreeWidget::sigStartRecording);
    connect(recordingWg, &RecordingWidget::sigRecordingStopped, this, &GapFreeWidget::sigStopRecording);

    // Propagate event to ABF writer
    connect(recordingWg, &RecordingWidget::sigFileNameChanged, this, [=](QString value) {
        emit sigFileNameChanged(value);
    });
    connect(recordingWg, &RecordingWidget::sigRecordPathChanged,this, [=](QString value) {
        emit sigRecordPathChanged(value);
    });

    mainLayout->addWidget(recordingWg);
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    mainLayout->addWidget(spacer);

    splitter->addWidget(sideWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
}

void GapFreeWidget::setRecording(bool flag){
    this->recordingWg->setRecording(flag);
}

