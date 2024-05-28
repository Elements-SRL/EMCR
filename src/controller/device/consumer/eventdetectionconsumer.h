#ifndef EVENTDETECTIONCONSUMER_H
#define EVENTDETECTIONCONSUMER_H

#include <QObject>
#include "devicedataconsumer.h"
#include "ivchannel.h"
#include "plotconsumer.h"
#include "plotmessage.h"
#include "firstorderiirfilter.h"
#include "eventdetector.h"

#define PCS_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define PCS_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */
constexpr double MINIMUM_DATA_FOR_ANALYSIS = 0.1; //0.1s

class EventDetectionConsumer : public PlotConsumer
{
    Q_OBJECT
public:
    EventDetectionConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer, uint32_t minEventDuration_, uint32_t maxEventDuration_, double highCutoffFrequency);
    ~EventDetectionConsumer();
    void forceAxisUpdate() override;

    void setMinEventDurationInSamples(uint32_t);
    void setMaxEventDurationInSamples(uint32_t);
    void setHighCutoffFrequency(double newValue);

public slots:
    void onVoltageRangeChanged(RangedMeasurement_t range) override;
    void onSamplingRateChanged(Measurement_t samplingRate) override;

private:
    double highCutoffFrequency;
    double threshold = -1;
    int subSamplingRatio = 1;
    double binSize;
    std::vector<double> doubleBuffer;

    //this is used to pass data to the event saver
    std::vector<int16_t> intBuffer;
    std::vector<int16_t> baseline;
    std::vector<std::vector<int16_t>> currentValuesInt;
    std::vector<std::vector<double>> currentValuesDouble;
    std::vector<std::vector<double>> voltageValues;

    std::vector<int> dataSize;
    QMutex voltageAxisMtx;
    QMutex currentAxisMtx;
    uint32_t minEventSamples;
    uint32_t maxEventSamples;

    //Event stuff
    std::vector<EventDetector *> eventDetectionChannels;
    uint64_t timeCounter = 0;

protected:
    void clearData() override;
    void run() override;
    void allocateData() override;
    void emitPlotData() override;
};

#endif // EVENTDETECTIONCONSUMER_H
