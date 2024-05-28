#include "eventdetectionwidget.h"
#include <QVBoxLayout>
#include <qwt_scale_engine.h>
#include <QLabel>

EventDetectionWidget::EventDetectionWidget(double maxCutoffFrequency, double defaultMinDurationInSeconds, double defaultMaxDurationInSeconds, double defaultDurationBins, double defaultAmplitudeBins, double defaultSamplingRate, RangedMeasurement currentRange, double defaultMaxAmplitude, double defaultStdMultiplier, QWidget* parent)
    : QWidget(parent)
{
    // Initialize QLabel widgets for displaying information
    numberOfEventsLabel = new QLabel("Number of Events: ");
    avgLenLabel = new QLabel("Average Duration: ");
    avgAmplitudeLabel = new QLabel("Average Amplitude: ");
    totalNumberOfEventsLabel = new QLabel("Total Number of Events: ");

    QGridLayout* gridLayout = new QGridLayout(this);

    // Upper Left Histogram
    upperLeftHistogram = new QwtPlotBarChart("Upper Left Histogram");
    upperLetPlot = new BasePlot("Duration Histogram", "s", "count", this);
    upperLeftHistogram->attach(upperLetPlot);

    // Bottom Right Histogram
    bottomRightHistogram = new QwtPlotBarChart("Amplitudes Histogram");
    bottomRightPlot = new BasePlot("Amplitue Histogram", "count", "A", this);
    bottomRightPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, true);

    // Customize the Y-axis scale draw to invert labels

    // Set orientation to horizontal
    bottomRightHistogram->setOrientation(Qt::Horizontal);
    // Attach the bar chart to the plot
    bottomRightHistogram->attach(bottomRightPlot);

    // Bottom Left Plot
    bottomLeftPlot = new BasePlot("Events", "s", "A", this);

    // Input fields for upper right corner
    QLabel* minDurationLabel = new QLabel("Minimum event duration");
    QLabel* maxDurationLabel = new QLabel("Maximum event duration");
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
    maxAmplitude->setValue(defaultMaxAmplitude);
    QLabel* cutoffFrequencyLabel = new QLabel("Cutoff frequency");
    cutoffFrequencySpinbox = new QDoubleSpinBox();
    cutoffFrequencySpinbox->setSuffix("Hz");
    cutoffFrequencySpinbox->setMinimum(LOW_CUTOFF_FREQUENCY);
    cutoffFrequencySpinbox->setMaximum(maxCutoffFrequency);
    cutoffFrequencySpinbox->setValue(defaultSamplingRate);

    QLabel* stdMultiplierLabel = new QLabel("Std multiplier");
    stdMultiplierSpinbox = new QDoubleSpinBox();
    stdMultiplierSpinbox->setMinimum(1.0);
    stdMultiplierSpinbox->setMaximum(maxCutoffFrequency);
    stdMultiplierSpinbox->setValue(defaultStdMultiplier);


    // Upper part
    gridLayout->addWidget(upperLeftHistogram->plot(), 0, 0);

    QWidget* upperRightWidget = new QWidget(this);
    QHBoxLayout * upperRightLayout = new QHBoxLayout(upperRightWidget);
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
    inputLayout->addWidget(stdMultiplierLabel);
    inputLayout->addWidget(stdMultiplierSpinbox);
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

    upperRightLayout->addWidget(statsWidget);
    upperRightLayout->addWidget(inputsWidget);
    gridLayout->addWidget(upperRightWidget, 0, 1);

    // Bottom part
    gridLayout->addWidget(bottomLeftPlot, 1, 0);
    gridLayout->addWidget(bottomRightHistogram->plot(), 1, 1);

    setCurrentRange(currentRange);

    //layout->addLayout(bottomLayout);
    connect(minDurationInMs, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {emit minDurationChanged(value * 1.0e-6); });
    connect(maxDurationInMs, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {emit maxDurationChanged(value * 1.0e-6); });
    connect(startButton, &QPushButton::clicked, this, &EventDetectionWidget::startPressed);
    connect(stopButton, &QPushButton::clicked, this, &EventDetectionWidget::stopPressed);
    connect(amplitudeBins, QOverload<int>::of(&QSpinBox::valueChanged), this, &EventDetectionWidget::amplitudeBinsChanged);
    connect(durationBins, QOverload<int>::of(&QSpinBox::valueChanged), this, &EventDetectionWidget::durationBinsChanged);
    connect(maxAmplitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EventDetectionWidget::maxAmplitudeChanged);
    connect(cutoffFrequencySpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EventDetectionWidget::cutoffFrequencyChanged);
    connect(stdMultiplierSpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EventDetectionWidget::stdMultiplierChanged);
}

EventDetectionWidget::~EventDetectionWidget(){
    // Cleanup
    delete upperLeftHistogram;
    delete bottomRightHistogram;
    delete minDurationInMs;
    delete maxDurationInMs;
    delete bottomLeftPlot;
}

BasePlot* EventDetectionWidget::getPlot() {
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
    avgAmplitudeLabel->setText("Average Amplitude: " + QString::number(avgAmplitude) + " " + QString::fromStdString(amplitudeUom));
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

void EventDetectionWidget::setCurrentRange(RangedMeasurement cr) {
    amplitudeUom = cr.getFullUnit();
    maxAmplitude->setSuffix(QString::fromStdString(amplitudeUom));
    maxAmplitude->setMaximum(cr.max);
    bottomLeftPlot->setLabel(amplitudeUom, QwtPlot::Axis::yLeft);
    bottomRightPlot->setLabel(amplitudeUom, QwtPlot::Axis::xBottom);
}
