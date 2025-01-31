#ifndef PLOTCONSUMER_H
#define PLOTCONSUMER_H

#include <QFile>
#include <QTextStream>

#include <qwt_series_data.h>

#include "devicedataconsumer.h"
#include "plotmessage.h"

#define PCS_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define PCS_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */

class PlotConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    PlotConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    virtual ~PlotConsumer();

    virtual void forceAxisUpdate() = 0;
    void setProtocolId(unsigned int protocolId);

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;

    void onDurationChanged(Measurement_t duration);
    void plotAllChannels(bool flag);
    void onPlotSelectedChannels(bool flag);

signals:
    void setPlotData(PlotMessage plotMessage);

protected:
    typedef enum {
        TriggerStep0,
        TriggerStep1,
        JustTriggered,
        Triggered
    } TriggerStatus_t;

    virtual void emitPlotData() = 0;
    virtual void updateRangeAxis();

    std::vector<double *> voltageValues;
    std::vector<double *> currentValues;

    double * timeValues = nullptr;

    unsigned int protocolId = -1;

    std::vector <double> buffer;

    Measurement hold = {0.0, UnitPfxNone, "V"};
    Measurement totalDuration = {0.0, UnitPfxNone, "s"};
    double sweepTriggerValue = 0.0;
    bool sweepTriggerRising = true;
    bool sweepTriggerEnable = false;

    bool pushedDurationFlag = false;

    double pushedDuration = 1.0;
    double xAxisDuration = 1.0;

    QMutex timeAxisMtx;
    QMutex rangeAxisMtx;

    std::vector <uint16_t> expandedChannels;

    int maxSamples = 256;
    int dataSize = 0;
    /*! Having gapFreeTimeIdx as a property of the class ensures that when a new protocol starts the plot does not reset the x axis */
    int gapFreeTimeIdx = 0;
    /*! Having triggerBufferIdx and triggerLastIdx as properties of the class ensures that when a new protocol starts it can correctly trigger the plot */
    int triggerBufferIdx = 0;
    int triggerLastIdx;

    int subSamplingRatio = 1;
    int subSamplingIdx = 0;

    QFile logFile;
    QTextStream logStream;

signals:
    void plotDataUpdated();
};

class GapFreePlotConsumer : public PlotConsumer {
    Q_OBJECT

public:
    GapFreePlotConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~GapFreePlotConsumer();

    virtual void forceAxisUpdate() override;
    void setMaxSamplesPerPlot(int samples);

protected:
    void run() override;
    void allocateData();
    void clearData();
    void emitPlotData() override;

private:
    void updateTimeAxis();
    void computeTimeAxis();
};

class EpisodicPlotConsumer : public PlotConsumer {
    Q_OBJECT

public:
    EpisodicPlotConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~EpisodicPlotConsumer();

    void lockCurves();
    void unlockCurves();

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

protected:
    void run() override;
    void allocateData();
    void clearData();
    // void updateRangeAxis() override;

    // int timeIdx = 0;
    // int dataSize = 0;

    // int subSamplingRatio = 1;
    // int subSamplingIdx = 0;

    // QVector <CurveData *> activeCurrentCurveData;
    // QVector <CurveData *> activeVoltageCurveData;

private:
    void updateTimeAxis();
    void computeTimeAxis();
    void lockCurveData();
    void unlockCurveData();

    EpisodicDataHook * episodicHook = nullptr;

    std::vector <double> episodicTimeValues;
    std::vector <std::vector <double>> episodicCurrentValues;
    std::vector <std::vector <double>> episodicVoltageValues;

    Measurement_t pushedSweepDuration;
//     QRecursiveMutex curvesMtx;

// signals:
//     void plotDataUpdated(bool, bool, bool);
//     void repaintPlot();
};

#endif // PLOTCONSUMER_H
