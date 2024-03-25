#ifndef IVGRAPHCONSUMER_H
#define IVGRAPHCONSUMER_H

#include <QObject>
#include "devicedataconsumer.h"
#include "ivchannel.h"

#define PCS_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define PCS_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */

class IvGraphConsumer : public DeviceDataConsumer
{
    Q_OBJECT
public:
    explicit IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    virtual ~IvGraphConsumer();

public slots:
    void onStartConsuming();
    void onStopConsuming();

//    void onSamplingRateChanged(e384CommLib::Measurement_t samplingRate);
//    void onDownsamplingRatioChanged(unsigned int downsamplingRatio);
    void onVoltageRangeChanged(e384CommLib::RangedMeasurement_t range);
    void onCurrentRangeChanged(e384CommLib::RangedMeasurement_t range);
    void onSelectChannels(bool flag);
//    void onBinsNuberChanged(int numberOfBins);

private:
    int nBins;
    int subSamplingRatio = 1;
    double binSize;
    std::vector<double> buffer;
    std::vector<IvChannel *> ivChannels;
    std::vector<std::vector<double>> currentValues;
    std::vector<bool> plottedChannels;
    QMutex voltageAxisMtx;
    QMutex currentAxisMtx;
    void forceAxisUpdate();
//    void updateVoltageAxis();
//    void updateCurrentAxis();
    void clearData();
    void run();
    void allocateData();
    int scaleToBins(double value);
    void calculateBinSize(int nBins);
    void emitPlotData();
signals:
    void setPlotData(std::vector<double> voltageValues, std::vector<std::vector<double>> currentValues, int dataSize);
};

#endif // IVGRAPHCONSUMER_H
