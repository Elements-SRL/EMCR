#ifndef CALIBRATIONCONSUMER_H
#define CALIBRATIONCONSUMER_H

#define CCS_CALIB_INTERVAL_IN_S 1
#define CCS_CALIB_INTERVAL_TO_REMOVE_IN_S 0.1

#include "modeldevice.h"
#include "devicedataconsumer.h"

class CalibrationConsumer : public DeviceDataConsumer {
    Q_OBJECT
public:
    CalibrationConsumer(ModelDevice * mDev, DeviceDataProducer * producer);
    ~CalibrationConsumer();

public slots:
    void onStartConsuming() override;
    void onStopConsuming() override;
    void onPerformCalibration(vector<uint16_t> channelsToCalibrateIdxs);

    /*! \todo not really needed */
    void onSamplingRateChanged(Measurement_t samplingRate) override;
    void onVoltageRangeChanged(RangedMeasurement_t range) override;
    void onCurrentRangeChanged(RangedMeasurement_t range) override;

private:
    bool consumptionStopped = false;
    bool exitedDataConsumingLoop = false;
    QMutex consumptionMtx;
    QWaitCondition exitedDataConsumingLoopCv;

    int deviceUnderCalibrationType; /*! \todo NOT SURE IF NEEDED */
    Measurement_t calibrationSamplingRate;
    std::vector <RangedMeasurement_t> vcCurrentRangesArray;
    std::vector <Measurement_t> calibrationVoltStep;
    std::vector <Measurement_t> calibratonResistances;
    std::vector<std::vector<double_t>> gainADC; // vettore di 2 vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> offsetADC; // vettore di 2 vettori_di_offset (Uno per range)
    std::vector <double_t> offsetDAC; // vettore di offset (questo non dipende dal range)
    vector<uint16_t> channelToCalibIdxs; /*! \todo se vogliamo calibrare solo una scheda e non tutti i canali insieme. Calibrazione per schedda ancora da gestire */
    QVector <double> buffer;
    int samplesToremove;
    QVector <double> currentSum;
    QVector<QVector <double>> currentMeans;



    void run() override;

    void selectSelectAllChannels(bool selectValue); /*! \todo probabilmente non serve, non selezioniamo roba da GUI. Almmento la lasciamo */
    void turnAllChannelsOnOff(bool onValue);
    void selectSelectChannels(vector<uint16_t> channelIndexes, vector<bool> selectValues); /*! \todo probabilmente non serve, non selezioniamo roba da GUI. Almmento la lasciamo */
    void turnChannelsOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues);

    void leastSquareSimple(vector<double> x, vector<double> y, double &slope, double &offset);
//    void calibrateAdcGain();
};

#endif // CALIBRATIONCONSUMER_H

/* OCCHIO CHE NEI messageDispatcher device specifici abbiamo questa roba*/
//    /*! VC current gain */
//    calibVcCurrentGainRange.step = 1.0/1024.0;
//    calibVcCurrentGainRange.min = 0;//SHORT_MIN * calibVcCurrentGainRange.step;
//    calibVcCurrentGainRange.max = SHORT_MAX * calibVcCurrentGainRange.step;
//    calibVcCurrentGainRange.prefix = UnitPfxNone;
//    calibVcCurrentGainRange.unit = "";
//    selectedCalibVcCurrentGainVector.resize(currentChannelsNum);
//    Measurement_t defaultCalibVcCurrentGain = {1.57014, calibVcCurrentGainRange.prefix, calibVcCurrentGainRange.unit}; /*! \todo FCON qui c'è il valor medio per i 200nA */

//    /*! VC current offset */
//    calibVcCurrentOffsetRanges = vcCurrentRangesArray;
//    selectedCalibVcCurrentOffsetVector.resize(currentChannelsNum);
//    Measurement_t defaultCalibVcCurrentOffset = {0.0, calibVcCurrentOffsetRanges[defaultVcCurrentRangeIdx].prefix, calibVcCurrentOffsetRanges[defaultVcCurrentRangeIdx].unit};

