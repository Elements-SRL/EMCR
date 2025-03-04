#ifndef EPISODICCONTROLLER_H
#define EPISODICCONTROLLER_H

#include <QPointF>

#include <qwt_plot_directpainter.h>

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
#include <memory>
#include "bigplotviewcontroller.h"

class CurveData;

class EpisodicController : public CentralWidgetController {
    Q_OBJECT

public:
    EpisodicController(ApplicationStatus* , DeviceDataProducer* , Measurement_t , BigPlotWidget* , MainWindow*, DeviceController*);
    ~EpisodicController();
    void stop() override;
    void start() override;
    PlotConsumer* getConsumer() override;
    std::vector <DeviceDataConsumer *> getConsumers() override;

private:
    std::unique_ptr<BigPlotViewController> bpvc;
    EpisodicPlotConsumer * consumer = nullptr;
    EpisodicWidget* episodicWidget = nullptr;
    AbfDataWriterConsumer* abfDataWriterConsumer = nullptr;
    std::vector <std::vector <Curve *>> currentCurves;
    std::vector <std::vector <Curve *>> voltageCurves;
    std::vector <CurveData *> activeCurrentCurveData;
    std::vector <CurveData *> activeVoltageCurveData;
    int sweepIdx = -1;
    int persistentSweepsNum = 1000000000;
    int activeSweepPlottedPoints = 0;

    QwtPlotDirectPainter * episodicPainter;

    QElapsedTimer updateDataTimer;

    int lastUpdateTimeMs;
    int currentTimeMs;

    void clearCurves();
    void detachCurves(const std::vector <uint16_t> &channelIndexes) override;
    void attachCurves(const std::vector <uint16_t>& channelIndexes) override;
    void paintPlots(bool newSweepFlag);

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag) override;
    void onSetPlotData(PlotMessage plotMessage) override;
    void onRecordingRequest(bool flag);
    void onRecordingExecution(bool flag);
    void onProtocolStarted(unsigned int protocolId, ProtocolWidget * protocol) override;

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
