#include "eventdetectionwidget.h"
#include <QVBoxLayout>
#include <qwt_scale_engine.h>
#include <QLabel>
#include <QBoxLayout>
#include <QGroupBox>
#include <QSettings>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QLineEdit>
#include "globaldefines.h"
#include "eventdetector.h"

EventDetectionWidget::EventDetectionWidget(double maxCutoffFrequency,
                                           Measurement minDuration,
                                           Measurement maxDuration,
                                           double defaultDurationBins,
                                           double defaultAmplitudeBins,
                                           double defaultSamplingRate,
                                           RangedMeasurement currentRange,
                                           double defaultMaxAmplitude,
                                           double defaultStdMultiplier,
                                           EventsDirection ed,
                                           QWidget* parent): QWidget(parent)
{

    this->setObjectName("eventDetection");
    this->setWindowTitle("EventDetection");
    eventsDirection = ed;

    numberOfEventsLabel = new QLabel("Events/s ");
    numberOfEventsLabel->setObjectName("statsLbl");

    avgLenLabel = new QLabel("Average Duration ");
    avgLenLabel->setObjectName("statsLbl");

    avgAmplitudeLabel = new QLabel("Average Amplitude ");
    avgAmplitudeLabel->setObjectName("statsLbl");

    totalNumberOfEventsLabel = new QLabel("Total events ");
    totalNumberOfEventsLabel->setObjectName("statsLbl");

    numberOfEventsValue = new QLabel("-");
    numberOfEventsValue->setObjectName("statsVal");
    avgLenValue = new QLabel("-");
    avgLenValue->setObjectName("statsVal");
    avgAmplitudeValue = new QLabel("-");
    avgAmplitudeValue->setObjectName("statsVal");
    totalNumberOfEventsValue = new QLabel("-");
    totalNumberOfEventsValue->setObjectName("statsVal");

    QGridLayout* gridLayout = new QGridLayout(this);

    // --- Upper Left Histogram
    upperLeftHistogram = new QwtPlotBarChart("Events Duration Histogram");
    upperLetPlot = new BasePlot("Events Duration Histogram", "us", "count", this);
    upperLeftHistogram->attach(upperLetPlot);
     
    // --- Bottom Right Histogram
    bottomRightHistogram = new QwtPlotBarChart("Events Amplitude Histogram");
    bottomRightPlot = new BasePlot("Events Amplitude Histogram", "count", "A", this);
    switch (eventsDirection) {
    case DOWN:
        bottomRightPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, true);
        break;
    case UP:
        break;
    case BOTH:
        //NOT IMPLEMENTED YET
        break;
    default:
        break;
    }

    // Customize the Y-axis scale draw to invert labels

    // Set orientation to horizontal
    bottomRightHistogram->setOrientation(Qt::Horizontal);
    // Attach the bar chart to the plot
    bottomRightHistogram->attach(bottomRightPlot);

    // Bottom Left Plot
    bottomLeftPlot = new BasePlot("Events", "us", "A", this);

    // Input fields for upper right corner
    QLabel* minDurationLabel = new QLabel("Minimum event duration");
    QLabel* maxDurationLabel = new QLabel("Maximum event duration");
    minDurationInus = new QDoubleSpinBox();
    minDurationInus->setSuffix(" us");
    maxDurationInus = new QDoubleSpinBox();
    maxDurationInus->setSuffix(" us");
    QLabel* amplitudeBinsLabel = new QLabel("Number of amplitude bins");
    amplitudeBins = new QSpinBox();
    QLabel* durationBinsLabel = new QLabel("Number of duration bins");
    durationBins = new QSpinBox();
    QLabel* maxBinAmplitudeLabel = new QLabel("Max amplitude");
    maxAmplitude = new QDoubleSpinBox();


    // Top right side panel.
    // Panel is made by two sub-panels.
    QWidget* upperRightWidget = new QWidget(this);
    QHBoxLayout * upperRightLayout = new QHBoxLayout(upperRightWidget);

    auto controlsSxWg = new QWidget(this);
    auto controlsSxLy = new QVBoxLayout(controlsSxWg);

    auto controlsDxWg = new QWidget(this);
    auto controlsDxLy= new QVBoxLayout(controlsDxWg);


    // Two vertical panels side by side
    upperRightLayout->addWidget(controlsSxWg);
    upperRightLayout->addWidget(controlsDxWg);

    // --- LEFT Subpanel ---

    // 1. START/STOP analysis + event direction
    // auto controlsTitle = new QLabel("CONTROLS", controlsSxWg);
    // controlsTitle->setObjectName("controlsTitle");

    auto controlsButtons = new QWidget(this);
    controlsButtons->setObjectName("controlsButtons");
    auto controlsBtnRow = new QHBoxLayout(controlsButtons);
    controlsBtnRow->setAlignment(Qt::AlignLeft);

    startButton = new QPushButton(this);
    startButton->setObjectName("startButton");
    startButton->setText(" START");
    startButton->setToolTip("Start the event detection");
    controlsBtnRow->addWidget(startButton);

    stopButton = new QPushButton(this);
    stopButton->setObjectName("stopButton");
    stopButton->setText(" STOP");
    stopButton->setToolTip("Stop the event detection");
    controlsBtnRow->addWidget(stopButton);

    // Vertical Separator
    QFrame* vline = new QFrame(this);
    vline->setObjectName("lineSeparator");
    vline->setFrameShape(QFrame::VLine);
    controlsBtnRow->addWidget(vline);

    comboBox = new QComboBox(this);
    comboBox->addItem("Events go down");
    comboBox->addItem("Events go up");
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EventDetectionWidget::onComboBoxIndexChanged);
    controlsBtnRow->addWidget(comboBox);

    // 2. STATISTICS
    auto statsWg = new QWidget(this);
    statsWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    auto statsCol = new QVBoxLayout(statsWg);
    statsCol->setAlignment(Qt::AlignLeft);
    statsCol->setSpacing(6);

    auto statsTitle = new QLabel("MONITORING", controlsSxWg);
    statsTitle->setObjectName("controlsTitle");
    statsCol->addWidget(statsTitle);

    QFrame* statsContainer = new QFrame();
    statsContainer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    statsContainer->setObjectName("statsContainer");
    QVBoxLayout * statsContainerLy = new QVBoxLayout(statsContainer);
    statsCol->addWidget(statsContainer);

    auto addStatsRow = [&](QVBoxLayout * outLayout, QLabel * statName, QLabel * statValue){
        auto hbl = new QHBoxLayout();
        hbl->addWidget(statName);
        hbl->addStretch();
        hbl->addWidget(statValue);
        outLayout->addLayout(hbl);
    };

    addStatsRow(statsContainerLy, numberOfEventsLabel, numberOfEventsValue);
    addStatsRow(statsContainerLy, totalNumberOfEventsLabel, totalNumberOfEventsValue);
    addStatsRow(statsContainerLy, avgLenLabel, avgLenValue);
    addStatsRow(statsContainerLy, avgAmplitudeLabel, avgAmplitudeValue);

    maxAmplitude->setValue(defaultMaxAmplitude);
    QLabel* cutoffFrequencyLabel = new QLabel("Cutoff frequency");
    cutoffFrequencySpinbox = new QDoubleSpinBox();
    cutoffFrequencySpinbox->setSuffix(" Hz");
    cutoffFrequencySpinbox->setMinimum(LOW_CUTOFF_FREQUENCY);
    cutoffFrequencySpinbox->setMaximum(maxCutoffFrequency);
    cutoffFrequencySpinbox->setValue(defaultSamplingRate);

    QLabel* stdMultiplierLabel = new QLabel("Std multiplier");
    auto stdMultiplierTooltip = "The noise level is calculated using the standard deviation of the input signal.\nThis multiplier (N) helps differentiate between noise and events.\nTypically, a value of 3 is sufficient to filter out noise.\nSetting N too high may result in missing true events,\nwhile setting it too low may lead to noise being misclassified as events.";
    stdMultiplierLabel->setToolTip(stdMultiplierTooltip);
    stdMultiplierSpinbox = new QDoubleSpinBox();
    stdMultiplierSpinbox->setMinimum(1.0);
    stdMultiplierSpinbox->setMaximum(maxCutoffFrequency);
    stdMultiplierSpinbox->setValue(defaultStdMultiplier);
    stdMultiplierSpinbox->setToolTip(stdMultiplierTooltip);

    // 3. RECORDING
    recordingWg = new RecordingWidget(this,
                                      GLB_EVENT_DETECTION_RECORD_NAME_TAG,
                                      GLB_EVENT_DETECTION_RECORD_PATH_TAG,
                                      EVENT_DETECTION_DEFAULT_RECORD_NAME,
                                      EVENT_DETECTION_DEFAULT_RECORD_PATH);

    //controlsSxLy->addWidget(controlsTitle);
    controlsSxLy->addWidget(controlsButtons);
    controlsSxLy->addWidget(statsWg);
    controlsSxLy->addWidget(recordingWg);

    // --- RIGHT Subpanel

    // INPUTS/SETTINGS
    QWidget* inputsWidget = new QWidget(this);
    QVBoxLayout* inputCol= new QVBoxLayout(inputsWidget);

    // auto settingsTitle = new QLabel("SETTINGS", inputsWidget);
    // settingsTitle->setObjectName("controlsTitle");

    inputCol->addWidget(minDurationLabel);
    inputCol->addWidget(minDurationInus);
    minDurationInus->setMinimum(ZERO);
    minDurationInus->setMaximum(100000.0);
    minDuration.convertValue(UnitPfxMicro);
    minDurationInus->setValue(minDuration.value);
    inputCol->addWidget(maxDurationLabel);
    inputCol->addWidget(maxDurationInus);
    maxDurationInus->setMinimum(ZERO);
    maxDurationInus->setMaximum(100000.0);
    maxDuration.convertValue(UnitPfxMicro);
    maxDurationInus->setValue(maxDuration.value);
    inputCol->addWidget(amplitudeBinsLabel);
    inputCol->addWidget(amplitudeBins);
    amplitudeBins->setMinimum(2);
    amplitudeBins->setMaximum(5000);
    amplitudeBins->setValue(defaultAmplitudeBins);
    inputCol->addWidget(durationBinsLabel);
    inputCol->addWidget(durationBins);
    durationBins->setMinimum(2);
    durationBins->setMaximum(5000);
    durationBins->setValue(defaultDurationBins);
    inputCol->addWidget(maxBinAmplitudeLabel);
    inputCol->addWidget(maxAmplitude);
    maxAmplitude->setMinimum(ZERO);
    inputCol->addWidget(cutoffFrequencyLabel);
    inputCol->addWidget(cutoffFrequencySpinbox);
    inputCol->addWidget(stdMultiplierLabel);
    inputCol->addWidget(stdMultiplierSpinbox);
    //ADD MULTIPLIER OF THE STD FOR TH AND EXPLENATION

    // controlsDxLy->addWidget(settingsTitle);
    controlsDxLy->addWidget(inputsWidget);

    // --- GRID Layout compostition
    gridLayout->addWidget(upperLeftHistogram->plot(), 0, 0);
    gridLayout->addWidget(upperRightWidget, 0, 1);
    gridLayout->addWidget(bottomLeftPlot, 1, 0);
    gridLayout->addWidget(bottomRightHistogram->plot(), 1, 1);

    setMaxAmplitude(defaultMaxAmplitude);
    setCurrentRange(currentRange);
    setDuration(maxDuration);


    // --- CONNECTIONS
    connect(minDurationInus, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = minDurationInus->value();
        Measurement durationMeasurement = { value, UnitPfx::UnitPfxMicro, "s" };
        emit minDurationChanged(durationMeasurement);
        });
    connect(maxDurationInus, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = maxDurationInus->value();
        bottomLeftPlot->setAxisScale(QwtPlot::Axis::xBottom, 0.0, value);
        Measurement durationMeasurement = { value, UnitPfx::UnitPfxMicro, "s" };
        setDuration(durationMeasurement);
        emit maxDurationChanged(durationMeasurement);
        });
    connect(startButton, &QPushButton::clicked, this, &EventDetectionWidget::startPressed);
    connect(stopButton, &QPushButton::clicked, this, &EventDetectionWidget::stopPressed);
    connect(recordingWg, &RecordingWidget::sigRecordingStarted, this, &EventDetectionWidget::recordingStarted);
    connect(recordingWg, &RecordingWidget::sigRecordingStopped, this, &EventDetectionWidget::recordingStopped);
    connect(amplitudeBins, &QSpinBox::editingFinished, this, [=]() {
        const auto value = amplitudeBins->value();
        emit amplitudeBinsChanged(value);
        });
    connect(durationBins, &QSpinBox::editingFinished, this, [=]() {
        const auto value = durationBins->value();
        emit durationBinsChanged(value);
        });
    connect(maxAmplitude, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = maxAmplitude->value();
        setMaxAmplitude(value);
        emit maxAmplitudeChanged(value);
        });
    connect(cutoffFrequencySpinbox, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = cutoffFrequencySpinbox->value();
        emit cutoffFrequencyChanged(value);
        });
    connect(stdMultiplierSpinbox, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = stdMultiplierSpinbox->value();
        emit stdMultiplierChanged(value);
        });
}

EventDetectionWidget::~EventDetectionWidget(){
    // Cleanup
    delete upperLeftHistogram;
    delete bottomRightHistogram;
    delete minDurationInus;
    delete maxDurationInus;
    delete bottomLeftPlot;
}

BasePlot* EventDetectionWidget::getPlot() {
    return bottomLeftPlot;
}

void EventDetectionWidget::setEventsPerSecond(double eventsPerSecond) {
    numberOfEventsValue->setText(QString::number(eventsPerSecond) + "   ev/s");
}

void EventDetectionWidget::setAvgLen(double avgLength) {
    std::string unit = "    s";
    if (avgLength < 0.001) {
        unit = "    ms";
        avgLength *= 1000;
    }
    avgLenValue->setText(QString::number(avgLength) + QString::fromStdString(unit));
}

void EventDetectionWidget::setTotalNumberOfEvents(uint32_t numEvents) {
    totalNumberOfEventsValue->setText( QString::number(numEvents) + "        ");
}

void EventDetectionWidget::setAvgAmplitude(double avgAmplitude) {
    avgAmplitudeValue->setText(QString::number(avgAmplitude) + "    " + QString::fromStdString(amplitudeUom));
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

std::string EventDetectionWidget::getFileName() {
    QSettings settings;
    auto filename = settings.value(GLB_EVENT_DETECTION_RECORD_NAME_TAG).toString().toStdString();
    return filename;
}

std::string EventDetectionWidget::getFilePath() {
    QSettings settings;
    return QDir::toNativeSeparators(settings.value(GLB_EVENT_DETECTION_RECORD_PATH_TAG).toString()).toStdString();
}

void EventDetectionWidget::onComboBoxIndexChanged(int index){
    EventsDirection direction = static_cast<EventsDirection>(index);
    eventsDirection = direction;
    switch (eventsDirection)
    {
    case DOWN:
        bottomRightPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, true);
        break;
    case UP:
        bottomRightPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, false);
        break;
    case BOTH:
        //NOT IMPLEMENTED YET
        break;
    default:
        break;
    }
    setMaxAmplitude(maxEventAmplitude);
    emit sigEventDirectionChanged(direction);
}

void EventDetectionWidget::setMaxAmplitude(double value) {
    const auto padding = value * 0.05;
    maxEventAmplitude = value;
    if (this->eventsDirection == EventsDirection::DOWN) {
        bottomLeftPlot->setAxisScale(QwtPlot::Axis::yLeft, -value - padding, +padding);
    }
    else if (this->eventsDirection == EventsDirection::UP) {
        bottomLeftPlot->setAxisScale(QwtPlot::Axis::yLeft, -padding, value + padding);
    }
}

void EventDetectionWidget::setCurrentRange(RangedMeasurement cr) {
    amplitudeUom = cr.getFullUnit();
    maxAmplitude->setSuffix(" " + QString::fromStdString(amplitudeUom));
    maxAmplitude->setMaximum(cr.max);
    bottomLeftPlot->setLabel(amplitudeUom, QwtPlot::Axis::yLeft);
    bottomRightPlot->setLabel(amplitudeUom, QwtPlot::Axis::yLeft);
}

void EventDetectionWidget::setDuration(Measurement d) {
    //Durations are assumed in us
    const auto maxDuration = d.value * 3;
    d.convertValue(UnitPfxMicro);
    bottomLeftPlot->setAxisScale(QwtPlot::Axis::xBottom, 0, maxDuration);
    bottomLeftPlot->setLabel(d.getFullUnit(), QwtPlot::Axis::xBottom);
    upperLetPlot->setAxisScale(QwtPlot::Axis::xBottom, 0, d.value);
    upperLetPlot->setLabel(d.getFullUnit(), QwtPlot::Axis::xBottom);
}

void EventDetectionWidget::setMaxSamplingRate(double srHalf) {
    cutoffFrequencySpinbox->setMaximum(srHalf);
}

void EventDetectionWidget::setCutoffFrequency(double sr) {
    cutoffFrequencySpinbox->setValue(sr);
}

