#include "eventdetectionwidget.h"
#include <qwt_column_symbol.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <QVBoxLayout>

EventDetectionWidget::EventDetectionWidget(QWidget* parent)
    : QWidget(parent)
{
    // Upper Left Histogram
    upperLeftHistogram = new QwtPlotHistogram("Upper Left Histogram");
    upperLeftHistogram->attach(new QwtPlot());
    QVector<QwtIntervalSample> samples1;
    for (double x = 1; x <= 10; ++x)
        samples1.push_back(QwtIntervalSample(x, x * x, x + 0.5));
    upperLeftHistogram->setData(new QwtIntervalSeriesData(samples1));

    // Bottom Right Histogram
    bottomRightHistogram = new QwtPlotHistogram("Bottom Right Histogram");
    bottomRightHistogram->attach(new QwtPlot());
    QVector<QwtIntervalSample> samples2;
    for (double x = 1; x <= 10; ++x)
        samples2.push_back(QwtIntervalSample(x, x * x, x + 0.5));
    bottomRightHistogram->setData(new QwtIntervalSeriesData(samples2));

    // Upper Right Plot
    upperRightPlot = new QwtPlot();
    upperRightPlot->setTitle("Upper Right Plot");
    upperRightPlot->setCanvasBackground(Qt::white);

    // Bottom Left Plot
    bottomLeftPlot = new QwtPlot();
    bottomLeftPlot->setTitle("Bottom Left Plot");
    bottomLeftPlot->setCanvasBackground(Qt::white);

    // Set up layout
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Upper part
    QHBoxLayout* upperLayout = new QHBoxLayout;
    upperLayout->addWidget(upperLeftHistogram->plot());
    upperLayout->addWidget(upperRightPlot);
    layout->addLayout(upperLayout);

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
    delete upperRightPlot;
    delete bottomLeftPlot;
}
