#ifndef CALIBRATIONCONSUMER_H
#define CALIBRATIONCONSUMER_H

#define CCS_CALIB_INTERVAL_IN_S 1
#define CCS_CALIB_INTERVAL_TO_REMOVE_IN_S 0.1
#define CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ 5
#define CCS_DAC_OFFSET_MINIMIZATION_MAX_TRY 1
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
    void copyToAllVectors();
    void updateCalibParams();
    QString getCalibrationPath();

public slots:
    void onStartConsuming() override;
    void onStopConsuming() override;
    void onPerformCalibration(std::vector<uint16_t> channelsToCalibrateIdxs);
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

    DeviceTypes_t deviceUnderCalibrationType;
    int numOfBoards;
    int numOfChannelsOnBoard;
    int numOfChannels;
    Measurement_t calibrationSamplingRate;
    std::vector <RangedMeasurement_t> vcCurrentRangesArray;
    std::vector <RangedMeasurement_t> vcVoltageRangesArray;
    std::vector <std::vector <Measurement_t>> calibrationVoltSteps;
    std::vector <Measurement_t> calibratonResistances;
    bool areCalibResistOnBoard;
    CalibrationData_t calibData;
    std::vector<std::vector<double_t>> gainADC; // vettore di vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> offsetADC; // vettore di vettori_di_offset (Uno per range)
    std::vector<std::vector<double_t>> gainDAC; // vettore di gain (Uno per range)
    std::vector<std::vector<double_t>> offsetDAC; // vettore di offset (questo non dipende dal range)

    std::vector<std::vector<double_t>> allGainADC; // vettore di vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> allOffsetADC; // vettore di vettori_di_offset (Uno per range)
    std::vector<std::vector<double_t>> allGainDAC; // vettore di vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> allOffsetDAC; // vettore di offset (questo non dipende dal range)

    std::vector<bool> suspectChannelIdxs;
    double gainThreshForSuspect = 2.0;

    std::vector<uint16_t> channelToCalibIdxs; // se vogliamo calibrare solo una scheda e non tutti i canali insieme.
    int totalChannelsUnderCalibNum;
    QVector <double> buffer;
    int samplesToremove;
    QVector <double> currentSum;
    QVector<QVector <double>> currentMeans;
    int rangeIdx;
    std::vector<bool> someTrue;
    std::vector<bool> someFalse;
    double multiplierCurrent = 1.0;
    uint16_t defaultVcCurrRangeIdx;
    bool waitForModelCellChanged = false;
    bool waitForFirstModelCellChecked = false;
    QMutex popUpWindowMtx;

    /*! \todo FORSE MEGLIO METTERLI NEL MSGDISPATCHER DEVICE-SPECIFIC*/
    Measurement_t defaultAdcGainValue;
    Measurement_t defaultAdcOffsetValue;
    Measurement_t defaultDacGainValue;
    Measurement_t defaultDacOffsetValue;
    Measurement_t defaultCcAdcGainValue;
    Measurement_t defaultCcAdcOffsetValue;
    Measurement_t defaultCcDacGainValue;
    Measurement_t defaultCcDacOffsetValue;

    std::vector<QString> boardSerialNums;
    QString calibrationFilesFolder = CCS_CALIBRATION_DEFAULT_PATH;
    QString myCsvSeparator = ",";

    /*! \note MPAC new fields for calibration in current clamp*/
    std::vector <RangedMeasurement_t> ccCurrentRangesArray;
    std::vector <RangedMeasurement_t> ccVoltageRangesArray;
    QVector <double> voltageSum;
    QVector<QVector <double>> voltageMeans;
    std::vector<std::vector<double_t>> ccGainADC; // vettore di vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> ccOffsetADC; // vettore di vettori_di_offset (Uno per range)
    std::vector<std::vector<double_t>> ccGainDAC; // vettore di vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> ccOffsetDAC; // vettore di vettori_di_offset (Uno per range)

    std::vector<std::vector<double_t>> ccAllGainADC; // vettore di vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> ccAllOffsetADC; // vettore di vettori_di_offset (Uno per range)
    std::vector<std::vector<double_t>> ccAllGainDAC; // vettore di vettori_di_gain (Uno per range)
    std::vector<std::vector<double_t>> ccAllOffsetDAC; // vettore di vettori_di_offset (Uno per range)

    std::vector <std::vector <Measurement_t>> ccCalibrationVoltSteps;
    std::vector <std::vector <Measurement_t>> ccCalibrationCurrSteps;
    std::vector <Measurement_t> ccCalibratonResistances;
    std::vector <Measurement_t> ccCalibratonResisForCcAdcOffset; // only for ccVoltageOffset (ADC)

    double multiplierVoltage = 1.0;



    void run() override;

    /*! SOME UTILITY FUNCTIONS*/
    void selectAllChannels(bool selectValue); /*! \todo probabilmente non serve, non selezioniamo roba da GUI. Almmento la lasciamo */
    void turnAllChannelsOnOff(bool onValue);
    void turnAllStimulaOnOff(bool onValue);
    void selectSomeChannels(std::vector<uint16_t> channelIndexes, std::vector<bool> selectValues); /*! \todo probabilmente non serve, non selezioniamo roba da GUI. Almmento la lasciamo */
    void turnAllCalSwOnOff(bool onValue);
    void turnSomeChannelsOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void turnSomeStimulaOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void turnSomeCalSwOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);

    void turnAllVcSwOnOff(bool onValue);
    void turnAllCcSwOnOff(bool onValue);
    void turnAllVcCcSelOnOff(bool onValue);
    void turnAllCcStimulaOnOff(bool onValue);
    void turnSomeVcSwOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void turnSomeCcSwOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void turnSomeVcCcSelOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void turnSomeCcStimulaOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void setSourceForVoltageChannel(uint16_t source);
    void setSourceForCurrentChannel(uint16_t source);    void setVcConfiguration(std::vector<uint16_t> channelIndexes, std::vector<bool> someTrue, std::vector<bool> someFalse);
    void setCcConfiguration(std::vector<uint16_t> channelIndexes, std::vector<bool> someTrue, std::vector<bool> someFalse);

    /*! REAL CALIBRATION FUNCITIONS*/
    void leastSquareSimple(std::vector<double> x, std::vector<double> y, double &slope, double &offset);
    void calibrateAdcGain(int thisActualRangeIdx);
    void calibrateAdcOffset(RangedMeasurement_t thisActualRange);
    void calibrateDacGain();
    void calibrateDacOffset(RangedMeasurement_t thisVcCurrentActualRange, int thisVcCurrentActualRangeIdx);

    void calibrateCcAdcGain(int thisActualRangeIdx);
    void calibrateCcDacGain(int thisActualRangeIdx);
    void calibrateCcAdcOffset(RangedMeasurement_t thisActualRange);
    void calibrateCcDacOffset(RangedMeasurement_t thisActualRange);


    /*! Interactions with CSV files*/
    void mainSaveOnCsv();
    void prepareStuffToSaveOnCsv(QString path, QString fileNameRoot, std::vector<uint16_t> chanSubset);
    void saveCsv(std::vector<uint16_t> chanSubset, QTextStream &stream, bool vcTccF);
    void loadDefaultCalibParams(int channelsNum, bool forVc, bool forCc);
    void extractBoardCalibDataFromCsv(QTextStream &boardStream, bool vcTccF);
    QString getCsvData(std::vector<uint16_t> chanSubset, bool vcTccF);
    QString suspectChannelsMsg(std::vector<uint16_t> chanToCalibIdxs);

    void convertToMeasurement(std::vector<std::vector<Measurement_t>> &gainDacMeas,
                              std::vector<std::vector<Measurement_t>> &gainAdcMeas,
                              std::vector<std::vector<Measurement_t>> &offsetAdcMeas,
                              std::vector<std::vector<Measurement_t>> &offsetDacMeas,
                              std::vector<std::vector<Measurement_t>> &ccGainAdcMeas,
                              std::vector<std::vector<Measurement_t>> &ccOffsetAdcMeas,
                              std::vector<std::vector<Measurement_t>> &ccGainDacMeas,
                              std::vector<std::vector<Measurement_t>> &ccOffsetDacMeas
                              );

    void convertFromMeasurement(std::vector<std::vector<Measurement_t>> &gainDacMeas,
                                std::vector<std::vector<Measurement_t>> &gainAdcMeas,
                                std::vector<std::vector<Measurement_t>> &offsetAdcMeas,
                                std::vector<std::vector<Measurement_t>> &offsetDacMeas,
                                std::vector<std::vector<Measurement_t>> &ccGainAdcMeas,
                                std::vector<std::vector<Measurement_t>> &ccOffsetAdcMeas,
                                std::vector<std::vector<Measurement_t>> &ccGainDacMeas,
                                std::vector<std::vector<Measurement_t>> &ccOffsetDacMeas
                                );

signals:
    void sigCalibLoadingMsg(QString calibLoadMsg);
    void sigManualCalibDoneMsg(QString manualCalibDoneMsg);
    void sigNeedToChangeModelCellMsg(QString needToChangeModelCellMsg);
    void sigNeedToCheckFirstModelCellMsg(QString needToCheckFirstModelCellMsg);
};

#endif // CALIBRATIONCONSUMER_H
