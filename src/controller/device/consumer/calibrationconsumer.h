#ifndef CALIBRATIONCONSUMER_H
#define CALIBRATIONCONSUMER_H

#define CCS_CALIB_INTERVAL_IN_S 1
#define CCS_CALIB_INTERVAL_TO_REMOVE_IN_S 0.1
#define CCS_DAC_OFFSET_MINIMIZATION_MAX_TRY 3
#define CCS_CALIBRATION_DEFAULT_PATH "C:/EMCR_calib_folder/"

#include "modeldevice.h"
#include "devicedataconsumer.h"
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

class CalibrationConsumer : public DeviceDataConsumer {
    Q_OBJECT
public:
    CalibrationConsumer(ModelDevice * mDev, DeviceDataProducer * producer);
    ~CalibrationConsumer();

    void loadInitialCalibParams(QString path, QString mappingFileName);
    void updateCalibParams();

public slots:
    void onStartConsuming() override;
    void onStopConsuming() override;
    void onPerformCalibration(vector<uint16_t> channelsToCalibrateIdxs);
    void onModelCellChanged(bool modelCellChanged);
    void onFirstModelMounted(bool firstModelCellMounted);

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
    int numOfBoards;
    int numOfChannelsOnBoard;
    int numOfChannels;
    Measurement_t calibrationSamplingRate;
    std::vector <RangedMeasurement_t> vcCurrentRangesArray;
    std::vector <RangedMeasurement_t> vcVoltageRangesArray;
    std::vector <std::vector <Measurement_t>> calibrationVoltSteps;
    std::vector <Measurement_t> calibratonResistances;
    CalibrationData_t calibData;
    std::vector<std::vector<double_t>> gainADC; // vettore di 2 vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> offsetADC; // vettore di 2 vettori_di_offset (Uno per range)
    std::vector <double_t> offsetDAC; // vettore di offset (questo non dipende dal range)

    std::vector<std::vector<double_t>> allGainADC; // vettore di 2 vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> allOffsetADC; // vettore di 2 vettori_di_offset (Uno per range)
    std::vector <double_t> allOffsetDAC; // vettore di offset (questo non dipende dal range)

    std::vector<bool> suspectChannelIdxs;
    double gainThreshForSuspect = 2.0;

    vector<uint16_t> channelToCalibIdxs; // se vogliamo calibrare solo una scheda e non tutti i canali insieme.
    int totalChannelsUnderCalibNum;
    QVector <double> buffer;
    int samplesToremove;
    QVector <double> currentSum;
    QVector<QVector <double>> currentMeans;
    int rangeIdx;
    vector<bool> someTrue;
    vector<bool> someFalse;
    double multiplierCurrent = 1.0;
    uint16_t defaultVcCurrRangeIdx;
    bool waitForModelCellChanged = false;
    bool waitForFirstModelCellChecked = false;
    QMutex popUpWindowMtx;

    /*! \todo FORSE MEGLIO METTERLI NEL MSGDISPATCHER DEVICE-SPECIFIC*/
    Measurement_t defaultAdcGainValue;
    Measurement_t defaultAdcOffsetValue;
    Measurement_t defaultDacOffsetValue;

    vector<QString> boardSerialNums;
    QString calibrationFilesFolder = CCS_CALIBRATION_DEFAULT_PATH;
    QString myCsvSeparator = ",";


    void run() override;

    /*! SOME UTILITY FUNCTIONS*/
    void selectAllChannels(bool selectValue); /*! \todo probabilmente non serve, non selezioniamo roba da GUI. Almmento la lasciamo */
    void turnAllChannelsOnOff(bool onValue);
    void turnAllStimulaOnOff(bool onValue);
    void selectSomeChannels(vector<uint16_t> channelIndexes, vector<bool> selectValues); /*! \todo probabilmente non serve, non selezioniamo roba da GUI. Almmento la lasciamo */
    void turnSomeChannelsOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues);
    void turnSomeStimulaOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues);

    /*! REAL CALIBRATION FUNCITIONS*/
    void leastSquareSimple(vector<double> x, vector<double> y, double &slope, double &offset);
    void calibrateAdcGain(int thisActualRangeIdx);
    void calibrateAdcOffset(RangedMeasurement_t thisActualRange);
    void calibrateDacOffset(RangedMeasurement_t thisActualRange);

    /*! Interactions with CSV files*/
    void mainSaveOnCsv();
    void prepareStuffToSaveOnCsv(QString path, QString fileName, vector<uint16_t> chanSubset);
    void saveCsv(vector<uint16_t> chanSubset, QTextStream &stream);
    void loadDefaultCalibParams(int channelsNum);
    void extractBoardCalibDataFromCsv(QTextStream &boardStream);
    QString getCsvData(vector<uint16_t> chanSubset);
    QString suspectChannelsMsg(vector<uint16_t> chanToCalibIdxs);

    void convertToMeasurement(vector<vector<Measurement_t>> &gainAdcMeas, vector<vector<Measurement_t>> &offsetAdcMeas, vector<Measurement_t> &offsetDacMeas);

signals:
    void sigCalibLoadingMsg(QString calibLoadMsg);
    void sigManualCalibDoneMsg(QString manualCalibDoneMsg);
    void sigNeedToChangeModelCellMsg(QString needToChangeModelCellMsg);
    void sigNeedToCheckFirstModelCellMsg(QString needToCheckFirstModelCellMsg);
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

