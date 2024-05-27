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
constexpr double LOW_CUTOFF_FREQUENCY = 100.0;
constexpr double ZERO = 0.0;

class EventDetectionWidget : public QWidget
{
    Q_OBJECT

public:
    EventDetectionWidget(double maxCutoffFrequency, double defaultMinDuration, double defaultMaxDuration, double defaultDurationBins, double defaultAmplitudeBins, double defaultSamplingRate, QWidget* parent = nullptr);
    ~EventDetectionWidget();

    QwtPlot* getPlot();
    void setEventsPerSecond(double);
    void setAvgLen(double);
    void setAvgAmplitude(double);
    void setTotalNumberOfEvents(uint32_t numEvents);

    void setDurationData(const QVector<QPointF>& points);
    void setAmplitudeData(const QVector<QPointF>& points);

    void setCutoffFrequency(double maxCutoffFrequency);

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
    QDoubleSpinBox* cutoffFrequencySpinbox;

signals:
    void startPressed();
    void stopPressed();
    void minDurationChanged(double);
    void maxDurationChanged(double);
    void amplitudeBinsChanged(int);
    void durationBinsChanged(int);
    void cutoffFrequencyChanged(double);
};

#endif // EVENTDETECTIONWIDGET_H