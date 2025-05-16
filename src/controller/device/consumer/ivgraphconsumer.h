#ifndef IVGRAPHCONSUMER_H
#define IVGRAPHCONSUMER_H

#include <QObject>

#include "ivchannel.h"
#include "plotconsumer.h"
#include "plotmessage.h"

#define IVC_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */

class IvGraphConsumer : public PlotConsumer {
    Q_OBJECT

public:
    IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~IvGraphConsumer();
    void forceAxisUpdate() override;
    void calculateBinSize();

public slots:
    void onVoltageRangeChanged() override;
//    void onBinsNuberChanged(int numberOfBins);

private:
    int nBins = 3201;
    int subSamplingRatio = 1;
    double binSize = 1.0;
    std::vector<double> buffer;
    std::vector<IvChannel *> ivChannels;
    std::vector<double *> currentValues;
    std::vector<double *> voltageData;
    std::vector<int> dataSize;
    QMutex voltageAxisMtx;
    QMutex currentAxisMtx;
    int scaleToBins(int channelIdx, double value);
    std::vector<double> voltageBins;
    std::vector<int> officialDataSize;

protected:
    void clearData();
    void run() override;
    void allocateData();
    void emitPlotData() override;
    void updateRangeAxis() override;
};

#endif // IVGRAPHCONSUMER_H
