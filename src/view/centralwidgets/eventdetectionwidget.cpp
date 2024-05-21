#include "eventdetectionwidget.h"
#include <qwt_column_symbol.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <QVBoxLayout>

EventDetectionWidget::EventDetectionWidget(QWidget* parent)
    : QWidget(parent)
{
    // Initialize QLabel widgets for displaying information
    numberOfEventsLabel = new QLabel("Number of Events: ");
    avgLenLabel = new QLabel("Average Length: ");
    totalNumberOfEventsLabel = new QLabel("Total Number of Events: ");

    // Upper Left Histogram
    upperLeftHistogram = new QwtPlotHistogram("Upper Left Histogram");
    upperLeftHistogram->attach(new QwtPlot());

    // Bottom Right Histogram
    bottomRightHistogram = new QwtPlotHistogram("Bottom Right Histogram");
    bottomRightHistogram->attach(new QwtPlot());

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

void EventDetectionWidget::setNumberOfEvents(uint32_t numEvents) {
    numberOfEventsLabel->setText("Number of Events: " + QString::number(numEvents));
}

void EventDetectionWidget::setAvgLen(double avgLength) {
    avgLenLabel->setText("Average Length: " + QString::number(avgLength));
}

void EventDetectionWidget::setTotalNumberOfEvents(uint32_t numEvents) {
    totalNumberOfEventsLabel->setText("Total Number of Events: " + QString::number(numEvents));
}