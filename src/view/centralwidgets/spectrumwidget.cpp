#include "spectrumwidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QSplitter>

SpectrumWidget::SpectrumWidget(uint32_t channelsNum, BigPlot* plot, QWidget * parent) :
    QWidget(parent) {

    this->setObjectName("spectrum");
    this->setWindowTitle("Spectrum");
    auto outerLayout = new QVBoxLayout(this);

    auto splitter = new QSplitter(Qt::Horizontal, this);
    outerLayout->addWidget(splitter);
    splitter->addWidget(plot);

    auto sideWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(sideWidget);
    mainLayout->setContentsMargins(18, 10, 10, 10);
    mainLayout->setSpacing(12);
    splitter->addWidget(sideWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);

    // --- Section 1. CONTROLS
    auto controlsTitle = new QLabel("ANALYSIS CONTROLS", sideWidget);
    controlsTitle->setObjectName("controlsTitle");
    mainLayout->addWidget(controlsTitle);

    auto controlsRow = new QHBoxLayout();
    controlsRow->setAlignment(Qt::AlignLeft);
    controlsRow->setSpacing(8);
    mainLayout->addLayout(controlsRow);

    auto startButton = new QPushButton(this);
    startButton->setObjectName("startButton");
    startButton->setText(" START");
    startButton->setToolTip("Start the spectrum analysis if it was previously stopped");
    controlsRow->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &SpectrumWidget::sigStartPressed);

    auto stopButton = new QPushButton(this);
    stopButton->setObjectName("stopButton");
    stopButton->setText(" STOP");
    stopButton->setToolTip("Stop the spectrum analysis if it was previously started");
    controlsRow->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &SpectrumWidget::sigStopPressed);

    // Vertical Separator
    QFrame* vline = new QFrame(sideWidget);
    vline->setObjectName("lineSeparator");
    vline->setFrameShape(QFrame::VLine);
    controlsRow->addWidget(vline);

    auto btnAutoZoom = new QPushButton("   Zoom");
    btnAutoZoom->setObjectName("autoZoom");
    btnAutoZoom->setToolTip("Auto zoom");
    btnAutoZoom->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnAutoZoom->setCursor(Qt::PointingHandCursor);
    connect(btnAutoZoom, &QPushButton::clicked, this, &SpectrumWidget::sigAutoZoom);
    controlsRow->addWidget(btnAutoZoom);
    mainLayout->addLayout(controlsRow);

    // Separator
    QFrame* line = new QFrame(sideWidget);
    line->setObjectName("lineSeparator");
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    // --- Section 2. INTEGRATION WINDOW
    auto intWindowTitle = new QLabel("Integration window", sideWidget);
    intWindowTitle->setObjectName("controlsTitle");
    mainLayout->addWidget(intWindowTitle);

    auto integrationRow = new QHBoxLayout();
    integrationRow->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    integrationRow->setSpacing(6);
    mainLayout->addLayout(integrationRow);

    integrationWindowS = new QDoubleSpinBox;
    integrationWindowS->setRange(0.5, 10.0);
    integrationWindowS->setValue(0.5);

    integrationRow->addWidget(new QLabel("Int. window (s)"));
    integrationRow->addWidget(integrationWindowS);

    auto exportButton = new QPushButton(this);
    exportButton->setObjectName("exportCsvButton");
    exportButton->setText(" EXPORT");
    exportButton->setToolTip("Export to csv");
    exportButton->setFixedSize(80, 32);
    connect(exportButton, &QPushButton::clicked, this, &SpectrumWidget::sigExportSpectrum);
    integrationRow->addWidget(exportButton);

    connect(integrationWindowS, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = integrationWindowS->value();
        emit sigIntegrationWindowChanged(value);
    });

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    mainLayout->addWidget(spacer);

}
