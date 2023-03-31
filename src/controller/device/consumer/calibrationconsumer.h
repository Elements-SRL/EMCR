#ifndef CALIBRATIONCONSUMER_H
#define CALIBRATIONCONSUMER_H

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
    std::vector<std::vector<double_t>> gainADC;
    std::vector<std::vector<double_t>> offsetADC;
    std::vector <double_t> offsetDAC;



    void run() override;
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

