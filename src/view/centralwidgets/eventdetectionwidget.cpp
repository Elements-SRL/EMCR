#include "eventdetectionwidget.h"
#include <qwt_column_symbol.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <QVBoxLayout>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>

EventDetectionWidget::EventDetectionWidget(QWidget* parent)
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
    inputField1 = new QLineEdit();
    inputField2 = new QLineEdit();

    // Set up layout
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Upper part
    QHBoxLayout* upperLayout = new QHBoxLayout;
    upperLayout->addWidget(upperLeftHistogram->plot());
    QVBoxLayout* inputLayout = new QVBoxLayout;
    inputLayout->addWidget(inputField1);
    inputLayout->addWidget(inputField2);
    inputLayout->addWidget(numberOfEventsLabel);
    inputLayout->addWidget(totalNumberOfEventsLabel);
    inputLayout->addWidget(avgLenLabel);
    inputLayout->addWidget(avgAmplitudeLabel);
    upperLayout->addLayout(inputLayout);
    layout->addLayout(upperLayout);
    // Add QLabel widgets to display information in the upper right part
    
    // Bottom part
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(bottomLeftPlot);
    bottomLayout->addWidget(bottomRightHistogram->plot());
    layout->addLayout(bottomLayout);

    setLayout(layout);
}

EventDetectionWidget::~EventDetectionWidget(){
    // Cleanup
    delete upperLeftHistogram;
    delete bottomRightHistogram;
    delete inputField1;
    delete inputField2;
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
