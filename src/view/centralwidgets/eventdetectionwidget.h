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
    EventDetectionWidget(double maxCutoffFrequency, Measurement minDuration, Measurement maxDuration, double defaultDurationBins, double defaultAmplitudeBins, double defaultSamplingRate, RangedMeasurement currentRange, double defaultMaxAmplitude, double defaultStdMultiplier, EventsDirection ed, QWidget* parent = nullptr);
    ~EventDetectionWidget();

    BasePlot* getPlot();
    void setEventsPerSecond(double);
    void setAvgLen(double);
    void setAvgAmplitude(double);
    void setTotalNumberOfEvents(uint32_t numEvents);

    void setDurationData(const QVector<QPointF>& points);
    void setAmplitudeData(const QVector<QPointF>& points);

    void setCurrentRange(RangedMeasurement cr);
    void setMaxSamplingRate(double srHalf);

    std::string getFileName();
    std::string getFilePath();
    void setCutoffFrequency(double sr);
    void setRecordingStatus(bool);

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
    QDoubleSpinBox* minDurationInus;
    QDoubleSpinBox* maxDurationInus;
    QSpinBox* amplitudeBins;
    QSpinBox* durationBins;
    QDoubleSpinBox* maxAmplitude;
    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* startRecordingButton;
    QPushButton* stopRecordingButton;
    QPushButton* browseBtn;
    QLabel* numberOfEventsLabel; 
    QLabel* totalNumberOfEventsLabel; 
    QLabel* avgLenLabel;
    QLabel* avgAmplitudeLabel;
    QDoubleSpinBox* cutoffFrequencySpinbox;
    QDoubleSpinBox* stdMultiplierSpinbox;
    std::string amplitudeUom;
    QDir* directory;
    QLabel* eventsDirectionLabel;
    QComboBox* comboBox;

    EventsDirection eventsDirection;

    void setLabel(std::string text, QwtTextLabel* label, QwtPlot::Axis axis);
    void setAndFormatText(std::string text, QwtTextLabel* label, Qt::AlignmentFlag = Qt::AlignLeft);
    void changeFilePath();
    void changeFileName();
    void setMaxAmplitude(double value);
    void setDuration(Measurement d);

signals:
    void startPressed();
    void stopPressed();
    void minDurationChanged(Measurement);
    void maxDurationChanged(Measurement);
    void maxAmplitudeChanged(double);
    void amplitudeBinsChanged(int);
    void durationBinsChanged(int);
    void cutoffFrequencyChanged(double);
    void stdMultiplierChanged(double);
    void sigEventDirectionChanged(EventsDirection);
    void recordingStarted();
    void recordingStopped();

private slots:
    void onComboBoxIndexChanged(int index);
};

#endif // EVENTDETECTIONWIDGET_H