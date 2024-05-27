#ifndef EVENTDETECTIONWIDGET_H
#define EVENTDETECTIONWIDGET_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_histogram.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <QLineEdit>
#include <QLabel>
#include <qwt_plot_barchart.h>
#include <qspinbox.h>
#include <qpushbutton.h>

class EventDetectionWidget : public QWidget
{
    Q_OBJECT

public:
    EventDetectionWidget(QWidget* parent = nullptr);
    ~EventDetectionWidget();

    QwtPlot* getPlot();
    void setEventsPerSecond(double);
    void setAvgLen(double);
    void setAvgAmplitude(double);
    void setTotalNumberOfEvents(uint32_t numEvents);

    void setDurationData(const QVector<QPointF>& points);
    void setAmplitudeData(const QVector<QPointF>& points);

private:
    // Widgets for different parts of the layout
    QwtPlotBarChart* upperLeftHistogram;
    QwtPlotBarChart* bottomRightHistogram;
    QwtPlot* bottomRightPlot;
    //QwtPlotHistogram* bottomRightHistogram;
    QDoubleSpinBox* minDurationInMs;
    QDoubleSpinBox* maxDurationInMs;
    QSpinBox* amplitudeBins;
    QSpinBox* durationBins;
    QDoubleSpinBox* maxAmplitude;
    QPushButton* startButton;
    QPushButton* stopButton;
    QwtPlot* bottomLeftPlot;
    QLabel* numberOfEventsLabel; 
    QLabel* totalNumberOfEventsLabel; 
    QLabel* avgLenLabel;
    QLabel* avgAmplitudeLabel;

signals:
    void startPressed();
    void stopPressed();
    void minDurationChanged(double);
    void maxDurationChanged(double);
};

#endif // EVENTDETECTIONWIDGET_H