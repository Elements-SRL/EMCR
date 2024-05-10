#ifndef EVENTDETECTIONCONSUMER_H
#define EVENTDETECTIONCONSUMER_H

#include <QObject>
#include "devicedataconsumer.h"
#include "ivchannel.h"
#include "plotconsumer.h"
#include "plotmessage.h"
#include "firstorderiirfilter.h"
#include "eventdetectionchannel.h"

#define PCS_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define PCS_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */


class EventDetectionConsumer : public PlotConsumer
{
    Q_OBJECT
public:
    EventDetectionConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer);
    ~EventDetectionConsumer();
    void forceAxisUpdate() override;

public slots:
    void onVoltageRangeChanged(RangedMeasurement_t range) override;
    //    void onBinsNuberChanged(int numberOfBins);

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
    std::vector<EventDetectionChannel *> eventDetectionChannels;

protected:
    void clearData() override;
    void run() override;
    void allocateData() override;
    void emitPlotData() override;
};

#endif // EVENTDETECTIONCONSUMER_H
