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

EventDetectionWidget::EventDetectionWidget(double maxCutoffFrequency, double defaultMinDurationInSeconds, double defaultMaxDurationInSeconds, double defaultDurationBins, double defaultAmplitudeBins, double defaultSamplingRate, RangedMeasurement currentRange, double defaultMaxAmplitude, double defaultStdMultiplier, EventsDirection ed, QWidget* parent)
    : QWidget(parent)
{
    eventsDirection = ed;
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
    switch (eventsDirection)
    {
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
    QLabel* maxBinAmplitudeLabel = new QLabel("Max amplitude");
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
    auto stdMultiplierTooltip = "The noise level is calculated using the standard deviation of the input signal.\nThis multiplier (N) helps differentiate between noise and events.\nTypically, a value of 3 is sufficient to filter out noise.\nSetting N too high may result in missing true events,\nwhile setting it too low may lead to noise being misclassified as events.";
    stdMultiplierLabel->setToolTip(stdMultiplierTooltip);
    stdMultiplierSpinbox = new QDoubleSpinBox();
    stdMultiplierSpinbox->setMinimum(1.0);
    stdMultiplierSpinbox->setMaximum(maxCutoffFrequency);
    stdMultiplierSpinbox->setValue(defaultStdMultiplier);
    stdMultiplierSpinbox->setToolTip(stdMultiplierTooltip);


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

    //Events direction combobox
    comboBox = new QComboBox(this);
    // Add items to the combo box
    comboBox->addItem("Events go down");
    comboBox->addItem("Events go up");
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndexChanged(int)));
    statsLayout->addWidget(comboBox);

    statsLayout->addWidget(startButton);
    statsLayout->addWidget(stopButton);

    upperRightLayout->addWidget(statsWidget);
    upperRightLayout->addWidget(inputsWidget);
    gridLayout->addWidget(upperRightWidget, 0, 1);

    //Maybe manage the change of path and name sanding signals and creating new events files
    auto recordingGb = new QGroupBox(QString::fromStdString("Events Recording"));
    auto recordingVBoxLayout = new QVBoxLayout();
    auto qhBoxLayout = new QHBoxLayout();
    recordingVBoxLayout->addLayout(qhBoxLayout);

    fileNameLineEdit = new QLineEdit();
    recordPathLineEdit = new QLineEdit();
    QSettings settings;
    recordPathLineEdit->setText(settings.value(GLB_EVENT_DETECTION_RECORD_PATH_TAG, EVENT_DETECTION_DEFAULT_RECORD_PATH).toString());
    fileNameLineEdit->setText(settings.value(GLB_EVENT_DETECTION_RECORD_NAME_TAG, EVENT_DETECTION_DEFAULT_RECORD_NAME).toString());

    recordingGb->setLayout(recordingVBoxLayout);

    auto hboxRecordingPath = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxRecordingPath);

    auto hboxBrowseFile = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxBrowseFile);

    auto hboxFileName = new QHBoxLayout();
    recordingVBoxLayout->addLayout(hboxFileName);

    hboxRecordingPath->addWidget(new QLabel("Recording path:"), 0, 0);
    recordPathLineEdit->setReadOnly(true);
    hboxRecordingPath->addWidget(recordPathLineEdit);

    directory = new QDir(recordPathLineEdit->text());
    if (!directory->exists()) {
        // Create the directory
        if (directory->mkpath(".")) {
        }
    }
    auto browseBtn = new QPushButton("Change recordings directory");
    connect(browseBtn, &QPushButton::clicked, [=]() {
        // Open a directory selection dialog
        QString directoryPath = QFileDialog::getExistingDirectory(this,
        "Select Directory",
        directory->absolutePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // Check if the user selected a directory
    if (!directoryPath.isEmpty()) {
        auto recordingsDirectoryPath = directoryPath + "/";
        recordPathLineEdit->setText(recordingsDirectoryPath);
        directory = new QDir(recordingsDirectoryPath);
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

    statsLayout->addWidget(recordingGb);
    // Bottom part
    gridLayout->addWidget(bottomLeftPlot, 1, 0);
    gridLayout->addWidget(bottomRightHistogram->plot(), 1, 1);

    setCurrentRange(currentRange);

    //layout->addLayout(bottomLayout);
    connect(minDurationInMs, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = minDurationInMs->value();
        emit minDurationChanged(value * 1.0e-6); 
        });
    connect(maxDurationInMs, &QDoubleSpinBox::editingFinished, this, [=]() {
        const auto value = maxDurationInMs->value();
        emit maxDurationChanged(value * 1.0e-6);
        });
    connect(startButton, &QPushButton::clicked, this, &EventDetectionWidget::startPressed);
    connect(stopButton, &QPushButton::clicked, this, &EventDetectionWidget::stopPressed);
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
    connect(fileNameLineEdit, &QLineEdit::editingFinished, this, [=]() {
        emitFileName();
        });
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
    bottomRightPlot->setLabel(amplitudeUom, QwtPlot::Axis::yLeft);
}

std::string EventDetectionWidget::getFileName() {
    return fileNameLineEdit->text().toStdString();
}

std::string EventDetectionWidget::getFilePath() {
    return QDir::toNativeSeparators(recordPathLineEdit->text()).toStdString();
}

void EventDetectionWidget::emitFilePath() {
    QSettings settings;
    auto filePath = recordPathLineEdit->text();
    settings.setValue(GLB_EVENT_DETECTION_RECORD_PATH_TAG, filePath);
    emit sigRecordPathChanged();
}

void EventDetectionWidget::emitFileName() {
    QSettings settings;
    auto filename = fileNameLineEdit->text();
    settings.setValue(GLB_EVENT_DETECTION_RECORD_NAME_TAG, filename);
    emit sigFileNameChanged();
}

void EventDetectionWidget::onComboBoxIndexChanged(int index){
    EventsDirection direction = static_cast<EventsDirection>(index);
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
    emit sigEventDirectionChanged(direction);
}