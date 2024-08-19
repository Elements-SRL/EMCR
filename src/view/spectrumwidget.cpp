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

    auto containerWidget = new QWidget(this); // Create a container widget
    auto mainVl = new QVBoxLayout(containerWidget); // Set layout on the container widget
    splitter->addWidget(containerWidget);

    auto buttonsHl = new QHBoxLayout();

    auto startButton = new QPushButton(this);
    startButton->setIcon(QIcon(QPixmap(":/imgs/start protocol.png")));
    startButton->setToolTip("Start the spectrum analysis if it was previously stopped");
    buttonsHl->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &SpectrumWidget::startPressed);

    auto stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(QPixmap(":/imgs/stop protocol.png")));
    stopButton->setToolTip("Stop the spectrum analysis if it was previously strted");
    buttonsHl->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &SpectrumWidget::stopPressed);

    mainVl->addLayout(buttonsHl);

    mainVl->addWidget(new QLabel("Integration window [s]"));

    integrationWindowS = new QDoubleSpinBox;
    integrationWindowS->setRange(0.5, 10.0);
    integrationWindowS->setValue(0.5);

    mainVl->addWidget(integrationWindowS);

    connect(integrationWindowS, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = integrationWindowS->value();
        emit integrationWindowChanged(value);
    });
}
