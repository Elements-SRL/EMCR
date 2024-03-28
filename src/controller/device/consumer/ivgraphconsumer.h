#ifndef IVGRAPHCONSUMER_H
#define IVGRAPHCONSUMER_H

#include <QObject>
#include "devicedataconsumer.h"
#include "ivchannel.h"
#include "plotconsumer.h"
#include "plotmessage.h"

#define PCS_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define PCS_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */

class IvGraphConsumer : public PlotConsumer
{
    Q_OBJECT
public:
    IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~IvGraphConsumer();
    void forceAxisUpdate() override;

public slots:
    void onVoltageRangeChanged(RangedMeasurement_t range) override;
//    void onBinsNuberChanged(int numberOfBins);

private:
    int nBins;
    int subSamplingRatio = 1;
    double binSize;
    std::vector<double> buffer;
    std::vector<IvChannel *> ivChannels;
    std::vector<double *> currentValues;
    double * voltageData = nullptr;
    QMutex voltageAxisMtx;
    QMutex currentAxisMtx;
    int scaleToBins(double value);
    void calculateBinSize();
    std::vector<int> indexes;

protected:
    void clearData() override;
    void run() override;
    void allocateData() override;
    void emitPlotData() override;
};

#endif // IVGRAPHCONSUMER_H
