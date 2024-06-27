#ifndef SPECTRUMCONSUMER_H
#define SPECTRUMCONSUMER_H

#include <complex.h>

#include <QObject>

#include <fftw3.h>

#include "plotconsumer.h"
#include "plotmessage.h"

#define SPC_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define SPC_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */

class SpectrumConsumer : public PlotConsumer {
    Q_OBJECT

public:
    SpectrumConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~SpectrumConsumer();
    void forceAxisUpdate() override;
    void updateFrequencyAxis();

public slots:
    void onCurrentRangeChanged(RangedMeasurement_t range) override;

private:
    int nBins;
    int n2Bins;
    int subSamplingRatio = 1;
    double binSize;
    std::vector<int> dataSize;
    double * frequencyValues = nullptr;
    std::vector <double *> fftIn;
    std::vector <std::complex <double> *> fftOut;
    fftw_plan fftwPlan;

protected:
    void clearData() override;
    void run() override;
    void allocateData() override;
    void emitPlotData() override;
};

#endif // SPECTRUMCONSUMER_H
