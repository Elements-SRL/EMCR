#ifndef EVENTDETECTIONWIDGET_H
#define EVENTDETECTIONWIDGET_H

#include <QWidget>
#include <qlabel.h>
#include <qwt_plot_barchart.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include "e384commlib_global_addendum.h"
#include "baseplot.h"
#include <QDir>
#include <QComboBox>
#include "eventsdirection.h"

using namespace e384CommLib;

constexpr double LOW_CUTOFF_FREQUENCY = 100.0;
constexpr double ZERO = 0.0;

class EventDetectionWidget : public QWidget
{
    Q_OBJECT

public:
    EventDetectionWidget(double maxCutoffFrequency, double defaultMinDuration, double defaultMaxDuration, double defaultDurationBins, double defaultAmplitudeBins, double defaultSamplingRate, RangedMeasurement currentRange, double defaultMaxAmplitude, double defaultStdMultiplier, EventsDirection ed, QWidget* parent = nullptr);
    ~EventDetectionWidget();

    BasePlot* getPlot();
    void setEventsPerSecond(double);
    void setAvgLen(double);
    void setAvgAmplitude(double);
    void setTotalNumberOfEvents(uint32_t numEvents);

    void setDurationData(const QVector<QPointF>& points);
    void setAmplitudeData(const QVector<QPointF>& points);

    void setCutoffFrequency(double maxCutoffFrequency);
    void setCurrentRange(RangedMeasurement cr);

    std::string getFileName();
    std::string getFilePath();

private:
    QLineEdit* recordPathLineEdit;
    QLineEdit* fileNameLineEdit;
    // Widgets for different parts of the layout
    QwtPlotBarChart* upperLeftHistogram;
    QwtPlotBarChart* bottomRightHistogram;
    
    BasePlot* upperLetPlot;
    BasePlot* bottomLeftPlot;
    BasePlot* bottomRightPlot;
    
    //QwtPlotHistogram* bottomRightHistogram;
    QDoubleSpinBox* minDurationInMs;
    QDoubleSpinBox* maxDurationInMs;
    QSpinBox* amplitudeBins;
    QSpinBox* durationBins;
    QDoubleSpinBox* maxAmplitude;
    QPushButton* startButton;
    QPushButton* stopButton;
    QLabel* numberOfEventsLabel; 
    QLabel* totalNumberOfEventsLabel; 
    QLabel* avgLenLabel;
    QLabel* avgAmplitudeLabel;
    QDoubleSpinBox* cutoffFrequencySpinbox;
    QDoubleSpinBox* stdMultiplierSpinbox;
    std::string amplitudeUom;
    QDir* directory;
    QComboBox* comboBox;

    EventsDirection eventsDirection;

    void setLabel(std::string text, QwtTextLabel* label, QwtPlot::Axis axis);
    void setAndFormatText(std::string text, QwtTextLabel* label, Qt::AlignmentFlag = Qt::AlignLeft);
    void emitFilePath();
    void emitFileName();

signals:
    void startPressed();
    void stopPressed();
    void minDurationChanged(double);
    void maxDurationChanged(double);
    void maxAmplitudeChanged(double);
    void amplitudeBinsChanged(int);
    void durationBinsChanged(int);
    void cutoffFrequencyChanged(double);
    void stdMultiplierChanged(double);
    void sigFileNameChanged();
    void sigRecordPathChanged();
    void sigEventDirectionChanged(EventsDirection);

private slots:
    void onComboBoxIndexChanged(int index);
};

#endif // EVENTDETECTIONWIDGET_H