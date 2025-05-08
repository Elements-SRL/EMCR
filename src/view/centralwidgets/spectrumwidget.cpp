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

    auto containerWidget = new QWidget(this);
    auto mainVl = new QVBoxLayout(containerWidget);
    splitter->addWidget(containerWidget);

    splitter->setHandleWidth(20);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);

    auto buttonsHl = new QHBoxLayout();

    auto startButton = new QPushButton(this);
    startButton->setIcon(QIcon(QPixmap(":/imgs/start protocol.png")));
    startButton->setToolTip("Start the spectrum analysis if it was previously stopped");
    startButton->setIconSize(QSize(30, 30));
    startButton->setFixedSize(32, 32);
    buttonsHl->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &SpectrumWidget::sigStartPressed);

    auto stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(QPixmap(":/imgs/stop protocol.png")));
    stopButton->setToolTip("Stop the spectrum analysis if it was previously strted");
    stopButton->setIconSize(QSize(30, 30));
    stopButton->setFixedSize(32, 32);
    buttonsHl->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &SpectrumWidget::sigStopPressed);

    auto exportButton = new QPushButton(this);
    exportButton->setIcon(QIcon(QPixmap(":/imgs/export protocol.png")));
    exportButton->setToolTip("Export to csv");
    exportButton->setIconSize(QSize(30, 30));
    exportButton->setFixedSize(32, 32);
    buttonsHl->addWidget(exportButton);
    connect(exportButton, &QPushButton::clicked, this, &SpectrumWidget::sigExportSpectrum);

    {
        auto spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        buttonsHl->addWidget(spacer);
    }

    mainVl->addLayout(buttonsHl);

    mainVl->addWidget(new QLabel("Integration window [s]"));

    integrationWindowS = new QDoubleSpinBox;
    integrationWindowS->setRange(0.5, 10.0);
    integrationWindowS->setValue(0.5);

    mainVl->addWidget(integrationWindowS);

    connect(integrationWindowS, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = integrationWindowS->value();
        emit sigIntegrationWindowChanged(value);
    });

    auto zoomButtonsHl = new QHBoxLayout();

    auto autoZoomButton = new QPushButton(this);
    autoZoomButton->setIcon(QIcon(QPixmap(":/imgs/zoom full.png")));
    autoZoomButton->setToolTip("Auto zoom");
    autoZoomButton->setIconSize(QSize(30, 30));
    autoZoomButton->setFixedSize(32, 32);
    zoomButtonsHl->addWidget(autoZoomButton);
    connect(autoZoomButton, &QPushButton::clicked, this, &SpectrumWidget::sigAutoZoom);

    {
        auto spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        zoomButtonsHl->addWidget(spacer);
    }

    mainVl->addLayout(zoomButtonsHl);

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    mainVl->addWidget(spacer);

}
