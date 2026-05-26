#ifndef SPECTRUMCONSUMER_H
#define SPECTRUMCONSUMER_H

#include <complex.h>

#include <QObject>

#include "pocketfft_hdronly.h"

#include "plotconsumer.h"
#include "plotmessage.h"

#define SPC_MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */
#define SPC_MIN_DATA_BATCH_DURATION_S (0.01) /*!< 0.01s */
#define SPC_MAX_SAMPLES (2048)

class SpectrumConsumer : public PlotConsumer {
    Q_OBJECT

public:
    SpectrumConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~SpectrumConsumer();

    void forceAxisUpdate() override;

public slots:
    void onIntegrationWindowChanged(Measurement_t window);

protected:
    void updateFrequencyAxis();
    void computeFrequencyAxis();
    virtual void updateRangeAxis() override;
    void clearData();
    void run() override;
    void allocateData();
    void emitPlotData() override;

private:
    bool pushedIntegrationWindowFlag = false;
    double pushedIntegrationWindowS = 0.5;
    double integrationWindowS = 0.5;
    int integrationRounds = 1;
    int integrationRoundIdx = 0;
    int binIndex = 0;
    double normalizationFactor = 1.0;
    int channelsOffset = 0;

    int maxSamples2 = 2048;
    int nBins = 0;
    int n2Bins = 0;
    double df = 1.0;
    double * frequencyValues = nullptr;
    pocketfft::shape_t shape{static_cast<size_t>(nBins)};
    pocketfft::stride_t stride_in{sizeof(double)};
    pocketfft::stride_t stride_out{sizeof(std::complex<double>)};
    pocketfft::shape_t axes{0};
    std::vector <double *> fftIn;
    std::vector <std::complex <double> *> fftOut;
    std::vector <double *> currentSpectrumValues;
    std::vector <double *> irmsValues;

signals:
    void sigRangeUpdate(RangedMeasurement_t newRange);
};

#endif // SPECTRUMCONSUMER_H
