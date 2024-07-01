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

public slots:
    void onIntegrationWindowChanged(Measurement_t window);

protected:
    void updateFrequencyAxis();
    void computeFrequencyAxis();
    virtual void updateRangeAxis() override;
    void clearData() override;
    void run() override;
    void allocateData() override;
    void emitPlotData() override;

private:
    bool pushedIntegrationWindowFlag = false;
    double pushedIntegrationWindowS = 0.5;
    double integrationWindowS = 0.5;
    int integrationRounds = 1;
    int integrationRoundIdx = 0;
    int binIndex = 0;
    double normalizationFactor = 1.0;

    int nBins;
    int n2Bins;
    double * frequencyValues = nullptr;
    std::vector <double *> fftIn;
    std::vector <std::complex <double> *> fftOut;
    std::vector <fftw_plan> fftwPlans;
    std::vector<double *> currentSpectrumValues;
};

#endif // SPECTRUMCONSUMER_H
