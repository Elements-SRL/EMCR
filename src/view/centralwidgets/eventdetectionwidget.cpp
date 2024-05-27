#include "eventdetectionwidget.h"
#include <qwt_column_symbol.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <QVBoxLayout>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>

EventDetectionWidget::EventDetectionWidget(double maxCutoffFrequency, double defaultMinDurationInSeconds, double defaultMaxDurationInSeconds, double defaultDurationBins, double defaultAmplitudeBins, double defaultSamplingRate, QWidget* parent)
    : QWidget(parent)
{
    
    // Initialize QLabel widgets for displaying information
    numberOfEventsLabel = new QLabel("Number of Events: ");
    avgLenLabel = new QLabel("Average Duration: ");
    avgAmplitudeLabel = new QLabel("Average Amplitude: ");
    totalNumberOfEventsLabel = new QLabel("Total Number of Events: ");


    // Upper Left Histogram
    upperLeftHistogram = new QwtPlotBarChart("Upper Left Histogram");
    upperLeftHistogram->attach(new QwtPlot());

    // Bottom Right Histogram
    bottomRightHistogram = new QwtPlotBarChart("Amplitudes Histogram");
    bottomRightPlot = new QwtPlot();
    bottomRightPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, true);

    // Customize the Y-axis scale draw to invert labels

    // Set orientation to horizontal
    bottomRightHistogram->setOrientation(Qt::Horizontal);
    // Attach the bar chart to the plot
    bottomRightHistogram->attach(bottomRightPlot);

    // Bottom Left Plot
    bottomLeftPlot = new QwtPlot();
    bottomLeftPlot->setCanvasBackground(Qt::white);

    // Input fields for upper right corner
    QLabel* minDurationLabel = new QLabel("Minimum event duration (in us)");
    QLabel* maxDurationLabel = new QLabel("Maximum event duration (in us)");
    minDurationInMs = new QDoubleSpinBox();
    minDurationInMs->setSuffix("us");
    maxDurationInMs = new QDoubleSpinBox();
    maxDurationInMs->setSuffix("us");
    QLabel* amplitudeBinsLabel = new QLabel("Number of amplitude bins");
    amplitudeBins = new QSpinBox();
    QLabel* durationBinsLabel = new QLabel("Number of duration bins");
    durationBins = new QSpinBox();
    QLabel* maxBinAmplitudeLabel = new QLabel("Max bins amplitude");
    maxAmplitude = new QDoubleSpinBox();
    startButton = new QPushButton("Start");
    stopButton = new QPushButton("Stop");
    QLabel* cutoffFrequencyLabel = new QLabel("Cutoff frequency");
    cutoffFrequencySpinbox = new QDoubleSpinBox();
    cutoffFrequencySpinbox->setSuffix("Hz");
    cutoffFrequencySpinbox->setMinimum(LOW_CUTOFF_FREQUENCY);
    cutoffFrequencySpinbox->setMaximum(maxCutoffFrequency);
    cutoffFrequencySpinbox->setValue(defaultSamplingRate);

    // Set up layout
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Upper part
    QHBoxLayout* upperLayout = new QHBoxLayout;
    upperLayout->addWidget(upperLeftHistogram->plot());

    //INPUTS
    QWidget* inputsWidget = new QWidget(this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputsWidget);
    inputLayout->addWidget(minDurationLabel);
    inputLayout->addWidget(minDurationInMs);
    minDurationInMs->setMinimum(ZERO);
    minDurationInMs->setMaximum(100000.0);
    minDurationInMs->setValue(defaultMinDurationInSeconds * 1.0e6);
    inputLayout->addWidget(maxDurationLabel);
    inputLayout->addWidget(maxDurationInMs);
    maxDurationInMs->setMinimum(ZERO);
    maxDurationInMs->setMaximum(100000.0);
    maxDurationInMs->setValue(defaultMaxDurationInSeconds * 1.0e6);
    inputLayout->addWidget(amplitudeBinsLabel);
    inputLayout->addWidget(amplitudeBins);
    amplitudeBins->setMinimum(2);
    amplitudeBins->setMaximum(5000);
    amplitudeBins->setValue(defaultAmplitudeBins);
    inputLayout->addWidget(durationBinsLabel);
    inputLayout->addWidget(durationBins);
    durationBins->setMinimum(2);
    durationBins->setMaximum(5000);
    durationBins->setValue(defaultDurationBins);
    inputLayout->addWidget(maxBinAmplitudeLabel);
    inputLayout->addWidget(maxAmplitude);
    maxAmplitude->setMinimum(ZERO);
    inputLayout->addWidget(cutoffFrequencyLabel);
    inputLayout->addWidget(cutoffFrequencySpinbox);
    //ADD MULTIPLIER OF THE STD FOR TH AND EXPLENATION

    //STATS
    QWidget* statsWidget = new QWidget(this);
    QVBoxLayout* statsLayout = new QVBoxLayout(statsWidget);
    statsLayout->addWidget(numberOfEventsLabel);
    statsLayout->addWidget(totalNumberOfEventsLabel);
    statsLayout->addWidget(avgLenLabel);
    statsLayout->addWidget(avgAmplitudeLabel);
    statsLayout->addWidget(startButton);
    statsLayout->addWidget(stopButton);

    upperLayout->addWidget(statsWidget);
    upperLayout->addWidget(inputsWidget);
    layout->addLayout(upperLayout);
    
    // Bottom part
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(bottomLeftPlot);
    bottomLayout->addWidget(bottomRightHistogram->plot());
    layout->addLayout(bottomLayout);
    connect(minDurationInMs, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {emit minDurationChanged(value * 1.0e-6); });
    connect(maxDurationInMs, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {emit maxDurationChanged(value * 1.0e-6); });
    connect(startButton, &QPushButton::clicked, this, &EventDetectionWidget::startPressed);
    connect(stopButton, &QPushButton::clicked, this, &EventDetectionWidget::stopPressed);
    connect(amplitudeBins, QOverload<int>::of(&QSpinBox::valueChanged), this, &EventDetectionWidget::amplitudeBinsChanged);
    connect(durationBins, QOverload<int>::of(&QSpinBox::valueChanged), this, &EventDetectionWidget::durationBinsChanged);
    connect(cutoffFrequencySpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EventDetectionWidget::cutoffFrequencyChanged);
}

EventDetectionWidget::~EventDetectionWidget(){
    // Cleanup
    delete upperLeftHistogram;
    delete bottomRightHistogram;
    delete minDurationInMs;
    delete maxDurationInMs;
    delete bottomLeftPlot;
}

QwtPlot* EventDetectionWidget::getPlot() {
    return bottomLeftPlot;
}

void EventDetectionWidget::setEventsPerSecond(double eventsPerSecond) {
    numberOfEventsLabel->setText("Events per second: " + QString::number(eventsPerSecond) + " Event/s");
}

void EventDetectionWidget::setAvgLen(double avgLength) {
    std::string unit = " s";
    if (avgLength < 0.001) {
        unit = " ms";
        avgLength *= 1000;
    }
    avgLenLabel->setText("Average Length: " + QString::number(avgLength) + QString::fromStdString(unit));
}

void EventDetectionWidget::setTotalNumberOfEvents(uint32_t numEvents) {
    totalNumberOfEventsLabel->setText("Total Number of Events: " + QString::number(numEvents));
}

void EventDetectionWidget::setAvgAmplitude(double avgAmplitude) {
    avgAmplitudeLabel->setText("Average Amplitude: " + QString::number(avgAmplitude));
}

// Method to set data for the upper left histogram
void EventDetectionWidget::setDurationData(const QVector<QPointF>& points) {
    upperLeftHistogram->setSamples(points);
    upperLeftHistogram->plot()->replot();
}

void EventDetectionWidget::setAmplitudeData(const QVector<QPointF>& points) {
    // Set samples to the bar chart
    bottomRightHistogram->setSamples(points);
    //bottomRightHistogram->setSamples(samples );
    bottomRightHistogram->plot()->replot();
}

void EventDetectionWidget::setCutoffFrequency(double maxCutoffFrequency) {
    cutoffFrequencySpinbox->setMaximum(maxCutoffFrequency);
}
