#ifndef PLOTCONSUMER_H
#define PLOTCONSUMER_H

#include <QFile>
#include <QTextStream>

#include "messagedispatcher.h"
#include "devicedataconsumer.h"

#define PCS_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define PCS_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */

class PlotConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    PlotConsumer(MessageDispatcher * msgDisp, DeviceDataProducer * producer);
    virtual ~PlotConsumer();

    void forceAxisUpdate();
    void setMaxSamplesPerPlot(int samples);

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;

    void onDurationChanged(Measurement_t duration);
    void onSelectChannels(bool flag);

protected:
    typedef enum {
        TriggerStep0,
        TriggerStep1,
        JustTriggered,
        Triggered
    } TriggerStatus_t;

    virtual void allocateData() = 0;
    virtual void clearData() = 0;
    virtual void emitPlotData() = 0;
    void updateTimeAxis();
    void computeTimeAxis();
    void updateRangeAxis();

    QVector <double *> voltageValues;
    QVector <double *> currentValues;

    double * timeValues = nullptr;

    QVector <double> buffer;

    Measurement hold = {0.0, UnitPfxNone, "V"};
    Measurement totalDuration = {0.0, UnitPfxNone, "s"};
    double sweepTriggerValue = 0.0;
    bool sweepTriggerRising = true;
    bool sweepTriggerEnable = false;

    bool pushedDurationFlag = false;

    bool consumptionStopped = false;
    bool exitedDataConsumingLoop = false;
    QMutex consumptionMtx;
    QWaitCondition exitedDataConsumingLoopCv;

    double pushedDuration = 1.0;
    double sweepDuration = 1.0;

    QMutex timeAxisMtx;
    QMutex rangeAxisMtx;

    QVector <bool> plottedChannels;
    int plottedChannelsNum = 0;

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
    void voltageRangeUpdated(RangedMeasurement_t range);
    void currentRangeUpdated(RangedMeasurement_t range);
    void durationUpdated(double duration);
    void plotDataUpdated();
};

class GapFreePlotConsumer : public PlotConsumer {
    Q_OBJECT

public:
    GapFreePlotConsumer(MessageDispatcher * msgDisp, DeviceDataProducer * producer);
    ~GapFreePlotConsumer();

protected:
    void run() override;

    void allocateData() override;
    void clearData() override;
    void emitPlotData() override;

signals:
    void setPlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber);
};

#endif // PLOTCONSUMER_H
