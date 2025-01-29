#ifndef EPISODICCONTROLLER_H
#define EPISODICCONTROLLER_H

#include <QPointF>

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "bigplotmodel.h"
#include "plotconsumer.h"
#include "application_status.h"
#include "plotmessage.h"
#include "centralwidgetcontroller.h"
#include "bigplotcontroller.h"
#include "episodicwidget.h"
#include "abfdatawriterconsumer.h"
#include "devicecontroller.h"

#define ECT_MAX_SAMPLES_PER_PLOT 8192

class CurveData;

class EpisodicController : public CentralWidgetController {
    Q_OBJECT

public:
    EpisodicController(ApplicationStatus* , DeviceDataProducer* , Measurement_t , BigPlotWidget* , BigPlotController*, MainWindow*, DeviceController*);
    ~EpisodicController();
    void stop() override;
    void start() override;
    PlotConsumer* getConsumer() override;

private:
    BigPlotModel* model = nullptr;
    EpisodicPlotConsumer * consumer = nullptr;
    BigPlot * plot = nullptr;
    EpisodicWidget* episodicWidget = nullptr;
    AbfDataWriterConsumer* abfDataWriterConsumer = nullptr;
    std::vector <std::vector <Curve *>> currentCurves;
    std::vector <std::vector <Curve *>> voltageCurves;
    std::vector <CurveData *> activeCurrentCurveData;
    std::vector <CurveData *> activeVoltageCurveData;
    int sweepIdx = -1;

    void clearCurves();
    void detachCurves(const std::vector <uint16_t> &channelIndexes) override;
    void attachCurves(const std::vector <uint16_t>& channelIndexes) override;

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onExpandTrace(bool flag) override;
    void onSetPlotData(PlotMessage plotMessage) override;
    void onRecordingRequest(bool flag);
    void onRecordingExecution(bool flag);

private slots:
    void onStartRecording();
    void onStopRecording();

signals:
    void durationChanged(Measurement_t duration);
    void sigStartRecording();
    void sigStopRecording();
};

class CurveData: public QwtSeriesData <QPointF> {
public:
    CurveData(int size);
    ~CurveData();

    QPointF sample(size_t idx) const override;
    size_t size() const override;

    QRectF boundingRect() const override;

    // void lock();
    // void unlock();

    void append(std::vector <double> newX, std::vector <double> newY);

private:
    std::vector <double> x;
    std::vector <double> y;
    mutable QRectF m_boundingRect; // Cache for bounding rectangle

    // QMutex mutex;
};

#endif // EPISODICCONTROLLER_H
