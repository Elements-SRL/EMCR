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
    EventDetectionConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer);
    ~EventDetectionConsumer();
    void forceAxisUpdate() override;

public slots:
    void onVoltageRangeChanged(RangedMeasurement_t range) override;
    void onSamplingRateChanged(Measurement_t samplingRate) override;

private:

    double threshold = -1;
    int nBins;
    int subSamplingRatio = 1;
    double binSize;
    std::vector<double> buffer;
    std::vector<std::vector<double>> currentValues;
    std::vector<double*> voltageData;
    std::vector<int> dataSize;
    QMutex voltageAxisMtx;
    QMutex currentAxisMtx;
    int scaleToBins(double value);
    std::vector<double> voltageBins;
    std::vector<int> officialDataSize;
    void calculateBinSize();
    
    //Event stuff
    std::vector<EventDetector *> eventDetectionChannels;
    void processEvent(std::pair<int, int> evtBegingEnd, uint32_t chIdx, std::vector<double>& eventBuffer);
protected:
    void clearData() override;
    void run() override;
    void allocateData() override;
    void emitPlotData() override;
};

#endif // EVENTDETECTIONCONSUMER_H
