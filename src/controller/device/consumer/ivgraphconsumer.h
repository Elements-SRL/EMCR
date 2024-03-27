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
    void onSelectChannels(bool flag);
//    void onBinsNuberChanged(int numberOfBins);

private:
    int nBins;
    int subSamplingRatio = 1;
    double binSize;
    std::vector<double> buffer;
    std::vector<IvChannel *> ivChannels;
    std::vector<double *> currentValues;
    std::vector<double> voltageData;
    QMutex voltageAxisMtx;
    QMutex currentAxisMtx;
    void clearData() override;
    void run() override;
    void allocateData() override;
    void onStartConsuming() override;
    int scaleToBins(double value);
    void calculateBinSize(int nBins);

    void emitPlotData() override;
};

#endif // IVGRAPHCONSUMER_H
