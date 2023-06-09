#include "calibrationconsumer.h"
#include "messagedispatcher.h"
#include "errormanager.h"

#include <QTime>
#include <QDebug>

CalibrationConsumer::CalibrationConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer){

    std::vector <Measurement_t> aaa;

    std::string tempString;
    mDev->getMessageDispatcher()->getCalibMappingFileDir(tempString);
    calibrationFilesFolder = QString().fromStdString(tempString);

    mDev->getMessageDispatcher()->getCalibMappingFilePath(tempString);
    calibrationMappingFilePath = QString().fromStdString(tempString);;


    mDev->getVcCurrentRangesFeatures(vcCurrentRangesArray, defaultVcCurrRangeIdx);
    mDev->getVcVoltageRangesFeatures(vcVoltageRangesArray);
    mDev->getCcCurrentRangesFeatures(ccCurrentRangesArray);
    mDev->getCcVoltageRangesFeatures(ccVoltageRangesArray);

    mDev->getMessageDispatcher()->getDeviceType(mDev->getSerialNumber().toStdString(), deviceUnderCalibrationType);
    mDev->getBoardsNumberFeatures(numOfBoards);
    numOfChannelsOnBoard = currentChannelsNum/numOfBoards;

    suspectChannelIdxs.resize(currentChannelsNum);
    fill(suspectChannelIdxs.begin(), suspectChannelIdxs.end(), false);

    mDev->getMessageDispatcher()->getCalibDefaultVcAdcGain(defaultAdcGainValue);    //1.0;
    mDev->getMessageDispatcher()->getCalibDefaultVcAdcOffset(defaultAdcOffsetValue); // 0.0;
    mDev->getMessageDispatcher()->getCalibDefaultVcDacGain(defaultDacGainValue); // 1.0;
    mDev->getMessageDispatcher()->getCalibDefaultVcDacOffset(defaultDacOffsetValue); // 0.0;
    mDev->getMessageDispatcher()->getCalibDefaultCcAdcGain(defaultCcAdcGainValue);    //1.0;
    mDev->getMessageDispatcher()->getCalibDefaultCcAdcOffset(defaultCcAdcOffsetValue); // 0.0;
    mDev->getMessageDispatcher()->getCalibDefaultCcDacGain(defaultCcDacGainValue); // 1.0;
    mDev->getMessageDispatcher()->getCalibDefaultCcDacOffset(defaultCcDacOffsetValue); // 0.0;

    mDev->getCalibDataFeatures(calibData);
    calibrationVoltSteps = calibData.vcCalibStepsArrays;
    calibratonResistances = calibData.vcCalibResArray;
    areCalibResistOnBoard = calibData.areCalibResistOnBoard;
    canInputsBeOpened = calibData.canInputsBeOpened;
    ccCalibrationVoltSteps = calibData.ccCalibVoltStepsArrays;
    ccCalibrationCurrSteps = calibData.ccCalibCurrStepsArrays;
    ccCalibratonResistances = calibData.ccCalibResArray;
    ccCalibratonResisForCcAdcOffset = calibData.ccCalibResForCcAdcOffsetArray; // only for ccVoltageOffset (ADC)

    gainADC.resize(vcCurrentRangesArray.size());
    offsetADC.resize(vcCurrentRangesArray.size());

    allGainADC.resize(vcCurrentRangesArray.size());
    allOffsetADC.resize(vcCurrentRangesArray.size());

    for(int i = 0; i< vcCurrentRangesArray.size(); i++){
        allGainADC[i].resize(currentChannelsNum);
        allOffsetADC[i].resize(currentChannelsNum);
    }

    gainDAC.resize(vcVoltageRangesArray.size());
    offsetDAC.resize(vcVoltageRangesArray.size());
    allGainDAC.resize(vcVoltageRangesArray.size());
    allOffsetDAC.resize(vcVoltageRangesArray.size());
    for(int i = 0; i< vcVoltageRangesArray.size(); i++){
        allGainDAC[i].resize(currentChannelsNum);
        allOffsetDAC[i].resize(currentChannelsNum);
    }


    ccGainADC.resize(ccVoltageRangesArray.size());
    ccOffsetADC.resize(ccVoltageRangesArray.size());

    ccAllGainADC.resize(ccVoltageRangesArray.size());
    ccAllOffsetADC.resize(ccVoltageRangesArray.size());

    for(int i = 0; i< ccVoltageRangesArray.size(); i++){
        ccAllGainADC[i].resize(currentChannelsNum);
        ccAllOffsetADC[i].resize(currentChannelsNum);
    }

    ccGainDAC.resize(ccCurrentRangesArray.size());
    ccOffsetDAC.resize(ccCurrentRangesArray.size());

    ccAllGainDAC.resize(ccCurrentRangesArray.size());
    ccAllOffsetDAC.resize(ccCurrentRangesArray.size());

    for(int i = 0; i< ccCurrentRangesArray.size(); i++){
        ccAllGainDAC[i].resize(currentChannelsNum);
        ccAllOffsetDAC[i].resize(currentChannelsNum);
    }

    boardSerialNums.resize(numOfBoards);

    /*! EXAMPLE OF boardMapping.csv*/
//    1,EL_board_1
//    2,EL_board_2
//    3,EL_board_3
//    4,EL_board_4
//    5,EL_board_5
//    6,EL_board_6
//    7,EL_board_7
//    8,EL_board_8
//    9,EL_board_9
//    10,EL_board_10
//    11,EL_board_11
//    12,EL_board_12
//    13,EL_board_13
//    14,EL_board_14
//    15,EL_board_15
//    16,EL_board_16
//    17,EL_board_17
//    18,EL_board_18
//    19,EL_board_19
//    20,EL_board_20
//    21,EL_board_21
//    22,EL_board_22
//    23,EL_board_23
//    24,EL_board_24

}

CalibrationConsumer::~CalibrationConsumer(){

}

QString CalibrationConsumer::getCalibrationDir(){
    return calibrationFilesFolder;
}

QString CalibrationConsumer::getCalibrationMappingFilePath(){
    return calibrationMappingFilePath;
}

void CalibrationConsumer::run(){
    consumptionStopped = false;
    exitedDataConsumingLoop = false;


//    totalChannelsUnderCalibNum = 2*channelToCalibIdxs.size();

    if(calibrationVoltSteps.size() > 0) {
        currentSum.resize(channelToCalibIdxs.size());
        currentSum.fill(0.0);
        currentMeans.resize(calibrationVoltSteps[0].size());
    }

    if(ccCalibrationCurrSteps.size() > 0) {
        voltageSum.resize(channelToCalibIdxs.size());
        voltageSum.fill(0.0);
        voltageMeans.resize(ccCalibrationCurrSteps[0].size());
    }

    for(int i = 0; i < channelToCalibIdxs.size(); i++){
        someTrue.push_back(true);
        someFalse.push_back(false);
    }

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    while(true){
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        /*! seleziona la più bassa sampling rate possibile*/
        std::vector <Measurement_t> samplingRates;
        mDev->getSamplingRatesFeatures(samplingRates);
        mDev->getMessageDispatcher()->setSamplingRate(calibData.samplingRateIdx, true);
        mDev->setSamplingRate(samplingRates[calibData.samplingRateIdx]);

        uint16_t bbb; // buffer variable used sometimes.

        /*! MPAC imposto qui la condizione di VC se serve*/
        setVcConfiguration(channelToCalibIdxs, someTrue, someFalse);

        /*! spegne lo stimolo e stacco il carico su tutti i canali per concentire all'utente di cambiare la model cell se c'è quella sbagliata*/
        turnAllStimulaOnOff(false);
        turnAllChannelsOnOff(false);
        if(areCalibResistOnBoard){
            turnAllCalSwOnOff(false);
        }

        /*! \todo FCON sostituire con flag e messaggi ottenuti dal CalibrationData_t */
        QString msg;
        if(deviceUnderCalibrationType == Device384PatchClamp
            #ifdef DEBUG
                || deviceUnderCalibrationType == Device384FakePatchClamp
            #endif
           ){
             msg = "Make sure you mounted the " + QString::fromStdString(ccCalibratonResisForCcAdcOffset[0].niceLabel()) + " model cell for the CCVoffset calibration.\nPress OK to continue.\n";
        } else if(deviceUnderCalibrationType == Device384Nanopores
                 #ifdef DEBUG
                      || deviceUnderCalibrationType == Device384Fake
                 #endif
                  ){
            msg = "Calibration will start in the range " + QString::fromStdString(vcCurrentRangesArray[0].niceLabel())+ ". Make sure you mounted the " + QString::fromStdString(calibratonResistances[0].niceLabel()) + " model cell.\nPress OK to continue.\n";
        }

        waitForFirstModelCellChecked = true;
        emit sigNeedToCheckFirstModelCellMsg(msg);
        while(true){
            QMutexLocker myLock(&popUpWindowMtx);
            if(!waitForFirstModelCellChecked){
                break;
            }
            myLock.unlock();
            QThread::msleep(10);
        }

        /*! FOR: START ciclo sui range di corrente in VC*/
        for(int jjj = 0; jjj <vcCurrentRangesArray.size(); jjj++){
            rangeIdx = jjj;

            /*! setto il range di corrente per Voltage Clamp*/
            std::vector <RangedMeasurement_t> rangeInfo;
            mDev->getVcCurrentRangesFeatures(rangeInfo, bbb);
            mDev->getMessageDispatcher()->setVCCurrentRange(rangeIdx, true);
            multiplierCurrent = rangeInfo[rangeIdx].multiplier();

            /*! spegne lo stimolo e stacco il carico su tutti i canali */
            /*! le condizioni di VC per 384PatchClamp erano state settate all'inizio*/
            turnAllStimulaOnOff(false);
            turnAllChannelsOnOff(false);
            if(areCalibResistOnBoard){
                turnAllCalSwOnOff(false);
            }

             /*! START CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            calibrateAdcGain(rangeIdx);
            /*! END CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

            /*! a questo punto tutti i canali hanno carico staccato  e stimolo spento*/

            /*! START CALCOLO ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    qui il carico deve essere staccato, staccato al  punto precedente, lo si può fare anche in maniera esplicita qui */
            calibrateAdcOffset(vcCurrentRangesArray[rangeIdx]);
            /*! END CALCOLO ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            if(rangeIdx < vcCurrentRangesArray.size()-1){
                /*! \todo FCON sostituire con flag e messaggi ottenuti dal CalibrationData_t */
                QString msg;
                if(deviceUnderCalibrationType == Device384PatchClamp
                        #ifdef DEBUG
                            || deviceUnderCalibrationType == Device384FakePatchClamp
                        #endif
                        ){
                     msg= "The next current range for VC calibration is " + QString::fromStdString(vcCurrentRangesArray[rangeIdx+1].niceLabel())+"\nPress OK.";
                } else if(deviceUnderCalibrationType == Device384Nanopores
                         #ifdef DEBUG
                              || deviceUnderCalibrationType == Device384Fake
                         #endif
                         ){
                    msg = "Need to mount the model cell " + QString::fromStdString(calibratonResistances[rangeIdx+1].niceLabel()) + " for current range " + QString::fromStdString(vcCurrentRangesArray[rangeIdx+1].niceLabel())+"\nPress OK only once the model cell has been changed.\n";
                }

                waitForModelCellChanged = true;
                emit sigNeedToChangeModelCellMsg(msg);
                while(true){
                    QMutexLocker myLock(&popUpWindowMtx);
                    if(!waitForModelCellChanged){
                        break;
                    }
                    myLock.unlock();
                    QThread::msleep(10);
                }
            }
        /*! FOR: END ciclo sui range*/
        }

        for(int jjj = 0; jjj <vcVoltageRangesArray.size(); jjj++){
            rangeIdx = jjj;

            /*! setto il range di tensione per Voltage Clamp*/
            std::vector <RangedMeasurement_t> rangeInfo;
            mDev->getVcVoltageRangesFeatures(rangeInfo);
            mDev->getMessageDispatcher()->setVCVoltageRange(rangeIdx, true);
            multiplierVoltage = rangeInfo[rangeIdx].multiplier();

            /*! \note MPAC: qui ho bisogno anche di un multiplier di corrente, visto che leggerò
             *  correnti dall'ADC in VC. Per comodità prendo il primo range di vcCurrentRanges*/
            std::vector <RangedMeasurement_t> rangeInfoAdditional;
            mDev->getVcCurrentRangesFeatures(rangeInfoAdditional, bbb);
            multiplierCurrent = rangeInfoAdditional[0].multiplier();
            mDev->getMessageDispatcher()->setVCCurrentRange(0, true);

            /*! \todo 20230529 MPAC: questa funzione è solo uno stub che riempie la struttura gainDAC di 1.0*/
            calibrateDacGain();
            /*! START CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

            /*! 20230529 MPAC: rengeIdx serve solo  a memorizzare nella posizione giusta del vettore offsetDac. Mi serve selezionare un range
            di VC Current (e relativo indice e resistenza di calibrazione) per la lettura della corrente dovuta alla tensione residua*/
            calibrateDacOffset(vcCurrentRangesArray[0], 0);
            /*! END CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
        }

        /*! Qui inizia la parte di calibrazione in CC*/
        /*! Open all IN_SW*/
        /*! \todo FCON sostituire con flag e messaggi ottenuti dal CalibrationData_t */
        if(deviceUnderCalibrationType == Device384PatchClamp
                #ifdef DEBUG
                    || deviceUnderCalibrationType == Device384FakePatchClamp
                #endif
                ){
            /*! \note MPAC non mi porto direttamente in condizioni CC, perchè il primo stepCCVgain e' un po' diverso dagli altri*/
            /*! Open input switch
            Close VC switch
            Close CC switch
            Disable current stimulus
            Set VC_CC_SEL = 0
            Set source 0 for voltage source(VC DAC as voltage readout), ossia buttali tutti a zero
            Set source 0 for current source (VC ADC as current readout), ossia buttali tutti a zero */
            turnSomeCalSwOnOff(channelToCalibIdxs, someFalse);
            turnSomeVcSwOnOff(channelToCalibIdxs, someTrue);
            turnSomeCcSwOnOff(channelToCalibIdxs, someTrue);
            turnSomeCcStimulaOnOff(channelToCalibIdxs, someFalse);
            turnSomeVcCcSelOnOff(channelToCalibIdxs, someFalse);
            setSourceForVoltageChannel(1);
            setSourceForCurrentChannel(0);

            /*! \note CCVgain ADC*/

            /*! FOR: START ciclo sui range di tensione in CC*/
            for(int jjj = 0; jjj <ccVoltageRangesArray.size(); jjj++){
                rangeIdx = jjj;

                /*! setto il range di tensione per Current Clamp*/
                std::vector <RangedMeasurement_t> rangeInfo;
                mDev->getCcVoltageRangesFeatures(rangeInfo);
                mDev->getMessageDispatcher()->setCCVoltageRange(rangeIdx, true);
                multiplierVoltage = rangeInfo[rangeIdx].multiplier();

                /*! START CALCOLO CC ADC GAIN (CCVgain)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
               calibrateCcAdcGain(rangeIdx);
               /*! END CALCOLO CC ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            }
            /*! END FOR: START ciclo sui range di tensione in CC*/

            /*! \note A questo punto posso impostare la configurazione CC e non toglierla più */
            /*! \note a questo punto potrei evitare di abilitare e disabilitare in continuazione gli stimoli in corrente*/
            setCcConfiguration(channelToCalibIdxs, someTrue, someFalse);

            /*! \note CCIgain DAC*/
            /*! FOR: START ciclo sui range di corrente in CC*/
            for(int jjj = 0; jjj <ccCurrentRangesArray.size(); jjj++){
                rangeIdx = jjj;

                /*! setto il range di tensione per Current Clamp*/
                std::vector <RangedMeasurement_t> rangeInfo;
                mDev->getCcCurrentRangesFeatures(rangeInfo);
                mDev->getMessageDispatcher()->setCCCurrentRange(rangeIdx, true);
                multiplierCurrent = rangeInfo[rangeIdx].multiplier();

                /*! \note MPAC: qui ho bisogno anche di un multiplier di tensione, visto che leggerò
                 *  tensioni dall'ADC in CC. Per comodità prendo il primo range di ccVoltageRanges*/
                std::vector <RangedMeasurement_t> rangeInfoAdditional;
                mDev->getCcVoltageRangesFeatures(rangeInfoAdditional);
                multiplierVoltage = rangeInfoAdditional[0].multiplier();

                /*! START CALCOLO CC DAC GAIN (CCIgain)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
               calibrateCcDacGain(rangeIdx);
               /*! END CALCOLO CC DAC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            }

            /*! \note MPAC: faccio il giochino di staccare i CAL_SW e attaccare gli IN_SW, in modo da sfruttare per questo passaggio le resistenze piccole montate sulla model cell da 120 kOhm*/
            turnSomeCalSwOnOff(channelToCalibIdxs, someFalse);
            turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);

            /*! \note CCVoffset ADC*/
            /*! FOR: START ciclo sui range di tensione in CC*/
            for(int jjj = 0; jjj <ccVoltageRangesArray.size(); jjj++){
                rangeIdx = jjj;

                /*! setto il range di tensione per Current Clamp*/
                std::vector <RangedMeasurement_t> rangeInfo;
                mDev->getCcVoltageRangesFeatures(rangeInfo);
                mDev->getMessageDispatcher()->setCCVoltageRange(rangeIdx, true);
                multiplierVoltage = rangeInfo[rangeIdx].multiplier();

                /*! START CALCOLO CC ADC OFFSET (CCVoffset)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
               calibrateCcAdcOffset(ccVoltageRangesArray[rangeIdx]);
               /*! END CALCOLO CC ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            }

            /*! \note MPAC: risistemo gli switch, CAL_SW attaccati e IN_SW staccati*/
            turnSomeCalSwOnOff(channelToCalibIdxs, someTrue);
            turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);

            /*! \note CCIoffset DAC*/
            /*! FOR: START ciclo sui range di corrente in CC*/
            for(int jjj = 0; jjj <ccCurrentRangesArray.size(); jjj++){
                rangeIdx = jjj;

                /*! setto il range di corrente per Current Clamp*/
                std::vector <RangedMeasurement_t> rangeInfo;
                mDev->getCcCurrentRangesFeatures(rangeInfo);
                mDev->getMessageDispatcher()->setCCCurrentRange(rangeIdx, true);
                multiplierCurrent = rangeInfo[rangeIdx].multiplier();

                /*! \note MPAC: qui ho bisogno anche di un multiplier di tensione, visto che leggerò
                 *  tensioni dall'ADC in CC. Per comodità prendo il primo range di ccVoltageRanges*/
                std::vector <RangedMeasurement_t> rangeInfoAdditional;
                mDev->getCcVoltageRangesFeatures(rangeInfoAdditional);
                multiplierVoltage = rangeInfoAdditional[0].multiplier();

                /*! START CALCOLO CC ADC OFFSET (CCVoffset)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
               calibrateCcDacOffset(ccVoltageRangesArray[0], 0);
               /*! END CALCOLO CC ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            }

        }

        someFalse.clear();
        someTrue.clear();

        /*! AGGIORNO LE STRUTTURA IN CUI TENGO TUTTI I VALORI DI CALIBRAZIONE*/
        for(int zzz = 0; zzz < vcCurrentRangesArray.size(); zzz++){
            for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
                allGainADC[zzz][channelToCalibIdxs[xxx]] = gainADC[zzz][xxx];
                allOffsetADC[zzz][channelToCalibIdxs[xxx]] = offsetADC[zzz][xxx];
                if(abs(allGainADC[zzz][xxx]) > gainThreshForSuspect){
                    suspectChannelIdxs[channelToCalibIdxs[xxx]] = true;
                }
            }
        }
        for(int zzz = 0; zzz < vcVoltageRangesArray.size(); zzz++){
            for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
                allGainDAC[zzz][channelToCalibIdxs[xxx]] = gainDAC[zzz][xxx];
                allOffsetDAC[zzz][channelToCalibIdxs[xxx]] = offsetDAC[zzz][xxx];
                /*! \todo FCON qui andrebbero aggiornati anche i gain del DAC */
            }
        }
        for(int zzz = 0; zzz < ccVoltageRangesArray.size(); zzz++){
            for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
                ccAllGainADC[zzz][channelToCalibIdxs[xxx]] = ccGainADC[zzz][xxx];
                ccAllOffsetADC[zzz][channelToCalibIdxs[xxx]] = ccOffsetADC[zzz][xxx];
                if(abs(ccAllGainADC[zzz][xxx]) > gainThreshForSuspect){
                    suspectChannelIdxs[channelToCalibIdxs[xxx]] = true;
                }
            }
        }
        for(int zzz = 0; zzz < ccCurrentRangesArray.size(); zzz++){
            for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
                ccAllOffsetADC[zzz][channelToCalibIdxs[xxx]] = ccOffsetADC[zzz][xxx];
                ccAllOffsetDAC[zzz][channelToCalibIdxs[xxx]] = ccOffsetDAC[zzz][xxx];
            }
        }


        /*! salvo queste info su CSV la cui struttura deve essere ancora decisa. Forse un file per ciascuna scheda*/
        mainSaveOnCsv();

        /*! ALLA FINE DI TUTTO SETTO IL VC CURRENT RANGE DI DEFAULT*/
        mDev->getMessageDispatcher()->setVCCurrentRange(defaultVcCurrRangeIdx, true);

        /*! \todo MPAC: al momento non ci sono problemi perchè per VcI, CcV e CcI abbiamo solo 1 range.
         *   Qui pero' bisognera' aggiungere il setting dei range di default anche per questi 3 casi*/

        /*! Invio dati a FPGA con massageDispatcher*/
        updateCalibParams();

        /*! \todo FCON questa cosa va gestita un po' meglio */
        consumptionLock.relock();
        consumptionStopped = true;
        consumptionLock.unlock();
    }

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void CalibrationConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        this->start();
    }
}

void CalibrationConsumer::onStopConsuming() {
    if (this->isRunning()) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = true;
        while (!exitedDataConsumingLoop) {
            exitedDataConsumingLoopCv.wait(&consumptionMtx, 100); // recheck
        }
    }

    if (hook != nullptr) {
        delete hook;
        hook = nullptr;
    }
}

void CalibrationConsumer::calibrateAdcGain(int thisActualRangeIdx){
    /*! Representation of voltage steps without any prefix, ...*/
    std::vector<double> x; /*! voltage steps*/
    x.resize(calibrationVoltSteps[thisActualRangeIdx].size());
    for(int i = 0; i< calibrationVoltSteps[thisActualRangeIdx].size(); i++){
        x[i] = calibrationVoltSteps[thisActualRangeIdx][i].getNoPrefixValue();
    }

    /*! attacca il carico  e accende lo stimolo su tutti i canali  o quelli della scheda selezionata*/
    if(areCalibResistOnBoard){
        turnSomeCalSwOnOff(channelToCalibIdxs, someTrue);
    } else {
        turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);
    }

    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    /*! FOR: START ciclo sugli step di tensione*/
    for(int voltStepIdx = 0; voltStepIdx <calibrationVoltSteps[thisActualRangeIdx].size(); voltStepIdx++){
        QThread::sleep(1);
        currentMeans[voltStepIdx].resize(channelToCalibIdxs.size());

        /*! setta la Vhold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
        std::vector<Measurement_t> someVoltSteps;
        for(int i = 0; i < channelToCalibIdxs.size(); i++){
            someVoltSteps.push_back(calibrationVoltSteps[thisActualRangeIdx][voltStepIdx]);
            mDev->getChannels()[channelToCalibIdxs[i]]->setVhold(calibrationVoltSteps[thisActualRangeIdx][voltStepIdx]);
        }
        mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

        /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;//channelToCalibIdxs.size();  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ*samplesToremove);
        int actualBufferSize = buffer.size();
        buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
        actualBufferSize = buffer.size();

        int timeSamples = actualBufferSize/totalChannelsNum;

         /*! faccio media delle sole correnti */
        int channelIdx;
        for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
            for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
                if(channelToCalibIdxs.size()==currentChannelsNum){
                    channelIdx = bufferIdx+voltageChannelsNum+currentIdx;
                } else {
                    /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                    channelIdx = bufferIdx+voltageChannelsNum+currentIdx + channelToCalibIdxs[0];
                }
                currentSum[currentIdx] += buffer[channelIdx]*multiplierCurrent;
            }
        }

        for(int i = 0; i < currentSum.size(); i++){
            currentMeans[voltStepIdx][i] = currentSum[i]/((double)timeSamples);
        }

        buffer.clear(); /*! is resized in getDataChunk()*/
        currentSum.clear();
        currentSum.resize(channelToCalibIdxs.size());
        currentSum.fill(0.0);
    }
    /*! FOR: END ciclo sugli step di tensione*/

    /*! UNA VOLTA CHE HO TUTTE CORRENTI MEDIE PER CIASCUN Vstep PER CIASCUN CANALE, FACCIO MINIMI QUADRATI */
    /*! FOR: START ciclo sui canali*/
    std::vector<double> y; /*! average currents*/
    y.resize(calibrationVoltSteps[thisActualRangeIdx].size());
    std::vector<double> usefulAdcGain;
    usefulAdcGain.resize(channelToCalibIdxs.size());
    double usefulSlope;
    double uselessOffset;

    for(int chIdx = 0; chIdx < channelToCalibIdxs.size(); chIdx++){
        for(int i = 0; i< calibrationVoltSteps[thisActualRangeIdx].size(); i++){
            y[i] = currentMeans[i][chIdx];
        }
        /*! calcolo slope con minimi quadrati che sarebbe 1/Rest*/
        leastSquareSimple(x, y, usefulSlope, uselessOffset);

        /*! il gain sarebbe Rest/Rcalib, i.e. 1(Rcalib * slope)*/
        usefulAdcGain[chIdx] = 1/(usefulSlope * calibratonResistances[rangeIdx].getNoPrefixValue());
        y.clear();
        y.resize(calibrationVoltSteps[thisActualRangeIdx].size());
    }
    /*! FOR: END ciclo sui canali*/
    gainADC[rangeIdx] = usefulAdcGain;

    for(int i = 0; i< calibrationVoltSteps[thisActualRangeIdx].size(); i++){
        currentMeans[i].clear();
    }

    /*! spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    if(areCalibResistOnBoard){
        turnSomeCalSwOnOff(channelToCalibIdxs, someFalse);
    } else {
        turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
    }
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);

}

void CalibrationConsumer::calibrateAdcOffset(RangedMeasurement_t thisActualRange){
    /*!  applico  0V ai canali selezionati*/
    std::vector<Measurement_t> someVoltSteps;
    for(int i = 0; i < channelToCalibIdxs.size(); i++){
        someVoltSteps.push_back({0.0, UnitPfxMilli, "V"});
       mDev->getChannels()[channelToCalibIdxs[i]]->setVhold({0.0, UnitPfxMilli, "V"});
    }
    mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

    /*! accende lo stimolo su tutti i canali  o su quelli della scheda selezionata*/
    /*! gli switch di ingresso sono staccati dal passo precedente*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
    la cui struttura è ancora da definire */
    sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
    minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
    samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
    QThread::sleep(1);
    hook->flush(); /*! Remove old buffered data */
    while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

    /*! butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
//    buffer.remove(0, 5*samplesToremove);
    buffer.remove(0, CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ*samplesToremove);
    int actualBufferSize = buffer.size();
    buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
    actualBufferSize = buffer.size();

    int timeSamples = actualBufferSize/totalChannelsNum;

     /*! \todo  faccio media delle sole correnti */
    int channelIdx;
    for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
        for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
            if(channelToCalibIdxs.size()==currentChannelsNum){
                channelIdx = bufferIdx+voltageChannelsNum+currentIdx;
            } else {
                /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                channelIdx = bufferIdx+voltageChannelsNum+currentIdx + channelToCalibIdxs[0];
            }
            currentSum[currentIdx] += buffer[channelIdx]*multiplierCurrent;
        }
    }

    std::vector<double> usefulAdcOffset;
    usefulAdcOffset.resize(channelToCalibIdxs.size());

    /*! moltiplico la corrente media per i GAIN calacolati al passo precedente e dovrei avere già l'offset di ADC*/
    for(int i = 0; i < currentSum.size(); i++){
        usefulAdcOffset[i] =-(gainADC[rangeIdx][i] * (currentSum[i]/((double)timeSamples) - thisActualRange.getMin().getNoPrefixValue()) + thisActualRange.getMin().getNoPrefixValue());

    }

    buffer.clear(); /*! is resized in getDataChunk()*/
    currentSum.clear();
    currentSum.resize(channelToCalibIdxs.size());
    currentSum.fill(0.0);

    offsetADC[rangeIdx] = usefulAdcOffset;

    /*! spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
    if(areCalibResistOnBoard){
        turnSomeCalSwOnOff(channelToCalibIdxs, someFalse);
    } else {
        turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
    }
}

void CalibrationConsumer::calibrateDacGain(){
    std::vector<double> usefulDacGain;
    usefulDacGain.resize(channelToCalibIdxs.size());
    for(int i = 0; i< usefulDacGain.size(); i++){
        usefulDacGain[i] = 1.0;
    }

    gainDAC[rangeIdx] = usefulDacGain;

}

void CalibrationConsumer::calibrateDacOffset(RangedMeasurement_t thisActualRange, int thisVcCurrentActualRangeIdx){
    int numTries = 0;
    std::vector<bool> needsFurtherCalibration;
    needsFurtherCalibration.resize(channelToCalibIdxs.size());

    std::vector<double> adcCompensatedCurrent;
    adcCompensatedCurrent.resize(channelToCalibIdxs.size());

    /*! applico  0V ai canali selezionati*/
    std::vector<Measurement_t> someVoltSteps;
    for(int i = 0; i < channelToCalibIdxs.size(); i++){
       someVoltSteps.push_back({0.0, UnitPfxMilli, "V"});
       mDev->getChannels()[channelToCalibIdxs[i]]->setVhold({0.0, UnitPfxMilli, "V"});
       needsFurtherCalibration[i] = true;
    }
    mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

    /*! accende lo stimolo e attacca gli switch di ingresso su tutti i canali  o su quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);
    if(areCalibResistOnBoard){
        turnSomeCalSwOnOff(channelToCalibIdxs, someTrue);
    } else {
        turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);
    }

//    currentMeans[0].resize(channelToCalibIdxs.size());

    std::vector<double> usefulDacOffset;
    usefulDacOffset.resize(channelToCalibIdxs.size());

    while(numTries < CCS_DAC_OFFSET_MINIMIZATION_MAX_TRY){
        /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        QThread::sleep(1);
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!  butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ*samplesToremove);
        int actualBufferSize = buffer.size();
        buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
        actualBufferSize = buffer.size();

        int timeSamples = actualBufferSize/totalChannelsNum;

         /*!  faccio media delle sole correnti */
        int channelIdx;
        for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
            for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
                if(channelToCalibIdxs.size()==currentChannelsNum){
                    channelIdx = bufferIdx+voltageChannelsNum+currentIdx;
                } else {
                    /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                    channelIdx = bufferIdx+voltageChannelsNum+currentIdx + channelToCalibIdxs[0];
                };
                currentSum[currentIdx] += buffer[channelIdx]*multiplierCurrent;
            }
        }

        /*! moltiplico la corrente media per i GAIN ADC  e sottraggo offset ADC calacolati per tenere conto delle calibrazioni precedenti*/
        for(int i = 0; i < currentSum.size(); i++){
            adcCompensatedCurrent[i] = (gainADC[thisVcCurrentActualRangeIdx][i] * (currentSum[i]/((double)timeSamples) - thisActualRange.getMin().getNoPrefixValue()) + thisActualRange.getMin().getNoPrefixValue()) + offsetADC[thisVcCurrentActualRangeIdx][i];

            if (adcCompensatedCurrent[i] == 0.0){
               needsFurtherCalibration[i] = false;
           } else {
               /*! sottraggo allo step di tensione attualmente applicato*/
                double poffi = calibratonResistances[thisVcCurrentActualRangeIdx].getNoPrefixValue(); // Ohm
                double bubbi = adcCompensatedCurrent[i]*poffi - someVoltSteps[i].getNoPrefixValue(); // V
                someVoltSteps[i].value = bubbi/someVoltSteps[i].multiplier(); //mV perchè divido V per 1e-3
           }
           usefulDacOffset[i] = -(someVoltSteps[i].getNoPrefixValue()); //V
        }

        /*! mandi via messageDispatcher i valori aggiornati di voltage step per vedere se la lettura sui canali mi diventa finalmetne 0 */
        mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

        buffer.clear(); /*! is resized in getDataChunk()*/
        currentSum.clear();
        currentSum.resize(channelToCalibIdxs.size());
        currentSum.fill(0.0);
        offsetDAC[rangeIdx] = usefulDacOffset;

        numTries++;
    }

    /*!  spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
    if(areCalibResistOnBoard){
        turnSomeCalSwOnOff(channelToCalibIdxs, someFalse);
    } else {
        turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
    }
}


/*! RECHECK: this can be used to pass specific params from the calibration GUI to the calibration thread, e.g. calibrate only one board
More functions will be needed, e.g. to load calibration from  a csv file */
void CalibrationConsumer::onPerformCalibration(std::vector<uint16_t> channelsToCalibrateIdxs){
    /*! \todo usa come esempio l'abf writer*/
    this->onStopConsuming();
    this->channelToCalibIdxs = channelsToCalibrateIdxs;
    this->onStartConsuming();
}

void CalibrationConsumer::onSamplingRateChanged(Measurement_t samplingRate){};
void CalibrationConsumer::onVoltageRangeChanged(RangedMeasurement_t range){};
void CalibrationConsumer::onCurrentRangeChanged(RangedMeasurement_t range){};

void CalibrationConsumer::selectAllChannels(bool selectValue){
    uint16_t numOfChannelsToUpadate = this->mDev->getChannels().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        this->mDev->getChannels()[i]->setSelected(selectValue);
    }
}

void CalibrationConsumer::turnAllChannelsOnOff(bool onValue){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    for (int i = 0; i < currentChannelsNum; i++){
        this->mDev->getChannels()[i]->setOn(onValue);
        channelIndexes[i] = i;
        onValues[i] = onValue;
    }
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
}

void CalibrationConsumer::turnAllStimulaOnOff(bool onValue){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    for (int i = 0; i < currentChannelsNum; i++){
        this->mDev->getChannels()[i]->setInStimActive(onValue);
        channelIndexes[i] = i;
        onValues[i] = onValue;
    }
    this->mDev->getMessageDispatcher()->enableStimulus(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllCalSwOnOff(bool onValue){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->turnCalSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllVcSwOnOff(bool onValue){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->turnVcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllCcSwOnOff(bool onValue){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->turnCcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllVcCcSelOnOff(bool onValue){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->turnVcCcSelOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllCcStimulaOnOff(bool onValue){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->enableCcStimulus(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::selectSomeChannels(std::vector<uint16_t> channelIndexes, std::vector<bool> selectValues){
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setSelected(selectValues[i]);
    }
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnSomeChannelsOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setOn(onValues[i]);
    }
}

void CalibrationConsumer::turnSomeStimulaOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->enableStimulus(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setInStimActive(onValues[i]);
    }
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeCalSwOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnCalSwOn(channelIndexes, onValues, true);
//    for (int i = 0; i < channelIndexes.size(); i++){
//        this->mDev->getChannels()[channelIndexes[i]]->setInStimActive(onValues[i]);
//        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
//    }
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeVcSwOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnVcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeCcSwOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnCcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeVcCcSelOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnVcCcSelOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeCcStimulaOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->enableCcStimulus(channelIndexes, onValues, true);
}

void CalibrationConsumer::setSourceForVoltageChannel(uint16_t source){
    this->mDev->getMessageDispatcher()->setSourceForVoltageChannel(source, true);
}

void CalibrationConsumer::setSourceForCurrentChannel(uint16_t source){
    this->mDev->getMessageDispatcher()->setSourceForCurrentChannel(source, true);
}



void CalibrationConsumer::setVcConfiguration(std::vector<uint16_t> channelIndexes, std::vector<bool> someTrue, std::vector<bool> someFalse){
    /*! \todo FCON molto specifico per il patch clamp */
    mDev->getMessageDispatcher()->turnVoltageReaderOn(false, false);
    mDev->getMessageDispatcher()->turnCurrentReaderOn(true, false);
    turnSomeCalSwOnOff(channelIndexes, someTrue);
    turnSomeVcSwOnOff(channelIndexes, someTrue);
    turnSomeCcSwOnOff(channelIndexes, someFalse);
    turnSomeCcStimulaOnOff(channelIndexes, someFalse);
    turnSomeVcCcSelOnOff(channelIndexes, someTrue);
    setSourceForVoltageChannel(0);
    setSourceForCurrentChannel(0);
}

void CalibrationConsumer::setCcConfiguration(std::vector<uint16_t> channelIndexes, std::vector<bool> someTrue, std::vector<bool> someFalse){
    /*! \todo FCON molto specifico per il patch clamp */
    mDev->getMessageDispatcher()->turnCurrentReaderOn(false, false);
    mDev->getMessageDispatcher()->turnVoltageReaderOn(true, false);
    mDev->getMessageDispatcher()->setDebugBit(0, 7, true);

    turnSomeCalSwOnOff(channelIndexes, someTrue);
    turnSomeVcSwOnOff(channelIndexes, someFalse);
    turnSomeCcSwOnOff(channelIndexes, someTrue);
    turnSomeCcStimulaOnOff(channelIndexes, someTrue);
    turnSomeVcCcSelOnOff(channelIndexes, someFalse);
    setSourceForVoltageChannel(1);
    setSourceForCurrentChannel(1);
}

void CalibrationConsumer::leastSquareSimple(std::vector<double> x, std::vector<double> y, double &slope, double &offset){
    double xsum=0,x2sum=0,ysum=0,xysum=0;                //variables for sums/sigma of xi,yi,xi^2,xiyi etc
    int n = x.size();
    for (int i = 0 ; i < x.size(); i++){
        xsum=xsum+x[i];                        //calculate sigma(xi)
        ysum=ysum+y[i];                        //calculate sigma(yi)
        x2sum=x2sum+pow(x[i],2);                //calculate sigma(x^2i)
        xysum=xysum+x[i]*y[i];                    //calculate sigma(xi*yi)
    }
    slope=(n*xysum-xsum*ysum)/(n*x2sum-xsum*xsum);            //calculate slope
    offset=(x2sum*ysum-xsum*xysum)/(x2sum*n-xsum*xsum);            //calculate intercept

}

void CalibrationConsumer::mainSaveOnCsv(){
    QString fileName;
    QString msg;
    if(channelToCalibIdxs.size() == currentChannelsNum){
        for(int i = 0; i < numOfBoards; i++){
            /*! calibro tutte le board*/
            std::vector<uint16_t>::iterator first = channelToCalibIdxs.begin() + numOfChannelsOnBoard*i; // incluso
            std::vector<uint16_t>::iterator last = channelToCalibIdxs.begin() + numOfChannelsOnBoard*i + (numOfChannelsOnBoard); // escluso
            std::vector<uint16_t> chanSubsetToCalibIdxs(first, last);
            fileName = boardSerialNums[i];// + QString(".csv");
            prepareStuffToSaveOnCsv(calibrationFilesFolder, fileName, chanSubsetToCalibIdxs);
        }
        msg = "All boards manual calibration successfull!\n";

    } else {
        /*! calibro solo una board*/
        fileName = boardSerialNums[channelToCalibIdxs[0]/numOfChannelsOnBoard];// + QString(".csv");
        prepareStuffToSaveOnCsv(calibrationFilesFolder, fileName, channelToCalibIdxs);
        msg = "Board " + QString("%1").arg(1+channelToCalibIdxs[0]/numOfChannelsOnBoard) +" manual calibration successfull!\n";
    }
    msg = msg + suspectChannelsMsg(channelToCalibIdxs);
    emit sigManualCalibDoneMsg(msg);
}

QString CalibrationConsumer::suspectChannelsMsg(std::vector<uint16_t> chanToCalibIdxs){
    QString msgSusp = "";
    std::vector<int> listOfSuspectIdxs;
    for(int j = 0; j < chanToCalibIdxs.size(); j++){
        if (suspectChannelIdxs[chanToCalibIdxs[j]]){
            listOfSuspectIdxs.push_back(chanToCalibIdxs[j]);
            msgSusp = msgSusp + QString("%1").arg(chanToCalibIdxs[j]+1) + ", ";
        }
    }
    if(listOfSuspectIdxs.size() > 0){
        msgSusp.chop(2);
        msgSusp = QString("Recheck the following channels: ")+ msgSusp;
    }
    return msgSusp;
}

void CalibrationConsumer::prepareStuffToSaveOnCsv(QString dir, QString fileNameRoot, std::vector<uint16_t> chanSubset){
    QString fileName = fileNameRoot + QString(".csv");
    QFile outFile(dir + fileName);
    QTextStream stream;
    if (QDir().exists(dir)) {
        outFile.open(QFile::WriteOnly);
        if (outFile.isOpen()) {
            stream.setDevice(&outFile);
            this->saveCsv(chanSubset, stream, true);
        }
        outFile.close();
    } else {
        if (QDir().mkpath(dir)) {
            if (outFile.open(QFile::WriteOnly )) {
                stream.setDevice(&outFile);
                this->saveCsv(chanSubset, stream, true);
                outFile.close();
            }
        }
    }

    /*! \todo FCON sostituire con flag e messaggi ottenuti dal CalibrationData_t */
    if(deviceUnderCalibrationType == Device384PatchClamp
            #ifdef DEBUG
                || deviceUnderCalibrationType == Device384FakePatchClamp
            #endif
            ){
        QString fileName = fileNameRoot + QString("_cc.csv");
        QFile outFile(dir + fileName);
        QTextStream stream;
        if (QDir().exists(dir)) {
            outFile.open(QFile::WriteOnly);
            if (outFile.isOpen()) {
                stream.setDevice(&outFile);
                this->saveCsv(chanSubset, stream, false);
            }
            outFile.close();
        } else {
            if (QDir().mkpath(dir)) {
                if (outFile.open(QFile::WriteOnly )) {
                    stream.setDevice(&outFile);
                    this->saveCsv(chanSubset, stream, false);
                    outFile.close();
                }
            }
        }
    }
}

void CalibrationConsumer::saveCsv(std::vector<uint16_t> chanSubset, QTextStream &stream, bool vcTccF){
    stream << this->getCsvData(chanSubset, vcTccF);
}

QString CalibrationConsumer::getCsvData(std::vector<uint16_t> chanSubset, bool vcTccF){
    QString ret;
    QTextStream stream(&ret);


    if(vcTccF){
        stream << QString("%1").arg(boardSerialNums[chanSubset[0]/numOfChannelsOnBoard]) << " Voltage Clamp\n";
        /*! loop on VC current ranges*/
        for(int i = 0; i < vcCurrentRangesArray.size(); i++){
            stream << QString("%1").arg(vcCurrentRangesArray[i].max) << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(gainADC[i].size()==currentChannelsNum){
                    /*! All channels calibration*/
                    stream << QString("%1").arg(gainADC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    /*! One board channels calibration*/
                    stream << QString("%1").arg(gainADC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(offsetADC[i].size()==currentChannelsNum){
                    /*! All channels calibration*/
                    stream << QString("%1").arg(offsetADC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    /*! One board channels calibration*/
                    stream << QString("%1").arg(offsetADC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
        }

        /*! loop on VC voltage ranges*/
        for(int i = 0; i < vcVoltageRangesArray.size(); i++){
            stream << QString("%1").arg(vcVoltageRangesArray[i].max) << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(gainDAC[i].size()==currentChannelsNum){
                    /*! All channels calibration*/
                    stream << QString("%1").arg(gainDAC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    /*! One board channels calibration*/
                    stream << QString("%1").arg(gainDAC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(offsetDAC[i].size()==currentChannelsNum){
                    stream << QString("%1").arg(offsetDAC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    stream << QString("%1").arg(offsetDAC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
        }
    } else {
        stream << QString("%1").arg(boardSerialNums[chanSubset[0]/numOfChannelsOnBoard]) << " Current Clamp\n";
        /*! loop on CC voltage ranges (ADC)*/
        for(int i = 0; i < ccVoltageRangesArray.size(); i++){
            stream << QString("%1").arg(ccVoltageRangesArray[i].max) << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(ccGainADC[i].size()==currentChannelsNum){
                    /*! All channels calibration*/
                    stream << QString("%1").arg(ccGainADC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    /*! One board channels calibration*/
                    stream << QString("%1").arg(ccGainADC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(ccOffsetADC[i].size()==currentChannelsNum){
                    /*! All channels calibration*/
                    stream << QString("%1").arg(ccOffsetADC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    /*! One board channels calibration*/
                    stream << QString("%1").arg(ccOffsetADC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
        }

        /*! loop on CC current ranges (DAC)*/
        for(int i = 0; i < ccCurrentRangesArray.size(); i++){
            stream << QString("%1").arg(ccCurrentRangesArray[i].max) << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(ccGainDAC[i].size()==currentChannelsNum){
                    /*! All channels calibration*/
                    stream << QString("%1").arg(ccGainDAC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    /*! One board channels calibration*/
                    stream << QString("%1").arg(ccGainDAC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
            for(int j = 0; j < chanSubset.size(); j++){
                if(ccOffsetDAC[i].size()==currentChannelsNum){
                    /*! All channels calibration*/
                    stream << QString("%1").arg(ccOffsetDAC[i][chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
                } else {
                    /*! One board channels calibration*/
                    stream << QString("%1").arg(ccOffsetDAC[i][j], 0, 'e', 3) << myCsvSeparator;
                }
            }
            stream << "\n";
        }
    }

    return ret;
}

void CalibrationConsumer::loadDefaultCalibParams(int channelsNum, bool forVc, bool forCc){
    if(forVc){
        for(int i = 0; i < vcCurrentRangesArray.size(); i++){
            for(int j = 0; j < channelsNum; j++){
                gainADC[i].push_back(defaultAdcGainValue.getNoPrefixValue());
                offsetADC[i].push_back(defaultAdcOffsetValue.getNoPrefixValue());
            }
        }

        for(int i = 0; i < vcVoltageRangesArray.size(); i++){
            for(int j = 0; j < channelsNum; j++){
                gainDAC[i].push_back(defaultDacGainValue.getNoPrefixValue());
                offsetDAC[i].push_back(defaultDacOffsetValue.getNoPrefixValue());
            }
        }
    }

    if(forCc){
        for(int i = 0; i < ccVoltageRangesArray.size(); i++){
            for(int j = 0; j < channelsNum; j++){
                ccGainADC[i].push_back(defaultCcAdcGainValue.getNoPrefixValue());
                ccOffsetADC[i].push_back(defaultCcAdcOffsetValue.getNoPrefixValue());
            }
        }

        for(int i = 0; i < ccCurrentRangesArray.size(); i++){
            for(int j = 0; j < channelsNum; j++){
                ccGainDAC[i].push_back(defaultCcDacGainValue.getNoPrefixValue());
                ccOffsetDAC[i].push_back(defaultCcDacOffsetValue.getNoPrefixValue());
            }
        }
    }
}

void CalibrationConsumer::loadInitialCalibParams(QString dir, QString mappingFileName){
    QStringList mappingStringList;
    QStringList boardStringList;
    std::vector<bool> calibratedWithDefaultParams;
    std::vector<bool> calibratedWithDefaultParamsCc;
    QString msg = "";

    /*! all'inizio devo caricare i valori di calibrazione per tutti i canali (o dai file se li trovo o dai valori di default) */
    channelToCalibIdxs.resize(currentChannelsNum);
    for(int i = 0; i< currentChannelsNum; i++){
        channelToCalibIdxs[i] = i;
    }

    CalibrationParams_t calibrationParams;
    std::vector<std::string> calibrationFileNames;
    std::vector<std::vector<bool>> calibLoadOkFlags;
    ErrorCodes_t error = mDev->getMessageDispatcher()->getCalibParams(calibrationParams);
    mDev->getMessageDispatcher()->getCalibFileNames(calibrationFileNames);
    mDev->getMessageDispatcher()->getCalibFilesFlags(calibLoadOkFlags);

    for(int i = 0; i < calibrationFileNames.size(); i++){
        boardSerialNums[i] = QString::fromStdString(calibrationFileNames[i]);
    }

    if (error != Success) {
        if(error == ErrorCalibrationDirMissing){
            msg = "Calibration directory " + dir + " not found.\nDefault calibration parameters were loaded.";
            emit sigCalibLoadingMsg(msg);
//        } else if (error == ErrorCalibrationMappingCorrupted){
//            msg = "Wrong mapping in " + mappingFileName + ".\nCalibration is in an unstable state.\nRecheck the mapping file, push disconnect, close and restart, EMCR.\nIf needed, repeat the calibration procedure.";
//            emit sigCalibLoadingMsg(msg);
        } else if(error == ErrorCalibrationMappingNotOpened){
            msg = "Calibration mapping file " + mappingFileName + " not found.\nDefault calibration parameters were loaded.";
            emit sigCalibLoadingMsg(msg);
        } else if(error == ErrorCalibrationMappingWrongNumbering){
            msg = "Wrong board numbering in " + mappingFileName + ".\nBoards should be numbered from 1 to " + QString("%1").arg(numOfBoards) + ".\nRecheck the mapping file, push disconnect, close and restart, EMCR.\nIf needed, repeat the calibration procedure.";
            emit sigCalibLoadingMsg(msg);
        } else if (error == ErrorCalibrationFileMissing || error == ErrorCalibrationFileCorrupted) {
           for(int k = 0; k < calibLoadOkFlags[0].size(); k++){
               if(calibLoadOkFlags[0][k] == false){
                   msg = msg + " VC - Board " + QString("%1").arg(k+1) + " calibrated with default parameters\n";
               } else {
                   msg = msg + " VC - Board " + QString("%1").arg(k+1) + " calibration paramteres loaded from file " + QString::fromStdString(calibrationFileNames[k]) +".csv\n";
               }
           }

           if(deviceUnderCalibrationType == Device384PatchClamp
           #ifdef DEBUG
               || deviceUnderCalibrationType == Device384FakePatchClamp
           #endif
           ){
               for(int k = 0; k < calibLoadOkFlags[1].size(); k++){
                   if(calibLoadOkFlags[1][k] == false){
                       msg = msg + " CC - Board " + QString("%1").arg(k+1) + " calibrated with default parameters\n";
                   } else {
                       msg = msg + " CC - Board " + QString("%1").arg(k+1) + " calibration paramteres loaded from file " + QString::fromStdString(calibrationFileNames[k]) +"_cc.csv\n";
                   }
               }
           }
            emit sigCalibLoadingMsg(msg);

        } else {
            emit sigCalibLoadingMsg(error);
        }
    } else {
        QString msg = "Calibration parameters loaded successfully.\n";
        emit sigCalibLoadingMsg(msg);
    }

    convertFromMeasurement(calibrationParams.allGainDacMeas,
                           calibrationParams.allGainAdcMeas,
                           calibrationParams.allOffsetAdcMeas,
                           calibrationParams.allOffsetDacMeas,
                           calibrationParams.ccAllGainAdcMeas,
                           calibrationParams.ccAllOffsetAdcMeas,
                           calibrationParams.ccAllGainDacMeas,
                           calibrationParams.ccAllOffsetDacMeas);
}

/*! This conversion is needed to send the calibration parameters contained in gainADC, offsetADC anf offsetDAC to the FPGA via MessageDispatcher
gainADC, offsetADC anf offsetDAC contain  the parameters corresponding to all the channels, despite a single board calibration was started.
 WE PREPARE AS MEASUREMENTS THE CALIB PARAMS FOR ALL THA CHANNELS*/
void CalibrationConsumer::convertToMeasurement(std::vector<std::vector<Measurement_t>> &gainDacMeas,
                                               std::vector<std::vector<Measurement_t>> &gainAdcMeas,
                                               std::vector<std::vector<Measurement_t>> &offsetAdcMeas,
                                               std::vector<std::vector<Measurement_t>> &offsetDacMeas,
                                               std::vector<std::vector<Measurement_t>> &ccGainAdcMeas,
                                               std::vector<std::vector<Measurement_t>> &ccOffsetAdcMeas,
                                               std::vector<std::vector<Measurement_t>> &ccGainDacMeas,
                                               std::vector<std::vector<Measurement_t>> &ccOffsetDacMeas
                                               ){
    /*! loop over ranges */
    for(int iii = 0; iii < vcCurrentRangesArray.size(); iii++){
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            gainAdcMeas[iii].push_back({allGainADC[iii][jjj], UnitPfxNone, ""});
            offsetAdcMeas[iii].push_back({allOffsetADC[iii][jjj], UnitPfxNone, "A"});
        }
    }

    for(int iii = 0; iii < vcVoltageRangesArray.size(); iii++){
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            gainDacMeas[iii].push_back({allGainDAC[iii][jjj], UnitPfxNone, ""});
            offsetDacMeas[iii].push_back({allOffsetDAC[iii][jjj], UnitPfxNone, "V"});
        }
    }

    /*! loop over ranges */
    for(int iii = 0; iii < ccVoltageRangesArray.size(); iii++){
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            ccGainAdcMeas[iii].push_back({ccAllGainADC[iii][jjj], UnitPfxNone, ""});
            ccOffsetAdcMeas[iii].push_back({ccAllOffsetADC[iii][jjj], UnitPfxNone, "V"});
        }
    }

    for(int iii = 0; iii < ccCurrentRangesArray.size(); iii++){
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            ccGainDacMeas[iii].push_back({ccAllGainDAC[iii][jjj], UnitPfxNone, ""});
            ccOffsetDacMeas[iii].push_back({ccAllOffsetDAC[iii][jjj], UnitPfxNone, "A"});
        }
    }
}

void CalibrationConsumer::convertFromMeasurement(std::vector<std::vector<Measurement_t>> &gainDacMeas,
                                                 std::vector<std::vector<Measurement_t>> &gainAdcMeas,
                                                 std::vector<std::vector<Measurement_t>> &offsetAdcMeas,
                                                 std::vector<std::vector<Measurement_t>> &offsetDacMeas,
                                                 std::vector<std::vector<Measurement_t>> &ccGainAdcMeas,
                                                 std::vector<std::vector<Measurement_t>> &ccOffsetAdcMeas,
                                                 std::vector<std::vector<Measurement_t>> &ccGainDacMeas,
                                                 std::vector<std::vector<Measurement_t>> &ccOffsetDacMeas
                                                 ){
    /*! loop over ranges */
    allGainADC.resize(vcCurrentRangesArray.size());
    allOffsetADC.resize(vcCurrentRangesArray.size());
    gainADC.resize(vcCurrentRangesArray.size());
    offsetADC.resize(vcCurrentRangesArray.size());
    for(int iii = 0; iii < vcCurrentRangesArray.size(); iii++){
        allGainADC[iii].resize(currentChannelsNum);
        allOffsetADC[iii].resize(currentChannelsNum);
        gainADC[iii].resize(currentChannelsNum);
        offsetADC[iii].resize(currentChannelsNum);
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            allGainADC[iii][jjj] = gainAdcMeas[iii][jjj].getNoPrefixValue();
            allOffsetADC[iii][jjj] = offsetAdcMeas[iii][jjj].getNoPrefixValue();
            gainADC[iii][jjj] = gainAdcMeas[iii][jjj].getNoPrefixValue();
            offsetADC[iii][jjj] = offsetAdcMeas[iii][jjj].getNoPrefixValue();
        }
    }

    allGainDAC.resize(vcVoltageRangesArray.size());
    allOffsetDAC.resize(vcVoltageRangesArray.size());
    gainDAC.resize(vcVoltageRangesArray.size());
    offsetDAC.resize(vcVoltageRangesArray.size());
    for(int iii = 0; iii < vcVoltageRangesArray.size(); iii++){
        allGainDAC[iii].resize(currentChannelsNum);
        allOffsetDAC[iii].resize(currentChannelsNum);
        gainDAC[iii].resize(currentChannelsNum);
        offsetDAC[iii].resize(currentChannelsNum);
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            allGainDAC[iii][jjj] = gainDacMeas[iii][jjj].getNoPrefixValue();
            allOffsetDAC[iii][jjj] = offsetDacMeas[iii][jjj].getNoPrefixValue();
            gainDAC[iii][jjj] = gainDacMeas[iii][jjj].getNoPrefixValue();
            offsetDAC[iii][jjj] = offsetDacMeas[iii][jjj].getNoPrefixValue();
        }
    }

    /*! loop over ranges */
    ccAllGainADC.resize(ccVoltageRangesArray.size());
    ccAllOffsetADC.resize(ccVoltageRangesArray.size());
    ccGainADC.resize(ccVoltageRangesArray.size());
    ccOffsetADC.resize(ccVoltageRangesArray.size());
    for(int iii = 0; iii < ccVoltageRangesArray.size(); iii++){
        ccAllGainADC[iii].resize(currentChannelsNum);
        ccAllOffsetADC[iii].resize(currentChannelsNum);
        ccGainADC[iii].resize(currentChannelsNum);
        ccOffsetADC[iii].resize(currentChannelsNum);
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            ccAllGainADC[iii][jjj] = ccGainAdcMeas[iii][jjj].getNoPrefixValue();
            ccAllOffsetADC[iii][jjj] = ccOffsetAdcMeas[iii][jjj].getNoPrefixValue();
            ccGainADC[iii][jjj] = ccGainAdcMeas[iii][jjj].getNoPrefixValue();
            ccOffsetADC[iii][jjj] = ccOffsetAdcMeas[iii][jjj].getNoPrefixValue();
        }
    }

    ccAllGainDAC.resize(ccCurrentRangesArray.size());
    ccAllOffsetDAC.resize(ccCurrentRangesArray.size());
    ccGainDAC.resize(ccCurrentRangesArray.size());
    ccOffsetDAC.resize(ccCurrentRangesArray.size());
    for(int iii = 0; iii < ccCurrentRangesArray.size(); iii++){
        ccAllGainDAC[iii].resize(currentChannelsNum);
        ccAllOffsetDAC[iii].resize(currentChannelsNum);
        ccGainDAC[iii].resize(currentChannelsNum);
        ccOffsetDAC[iii].resize(currentChannelsNum);
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            ccAllGainDAC[iii][jjj] = ccGainDacMeas[iii][jjj].getNoPrefixValue();
            ccAllOffsetDAC[iii][jjj] = ccOffsetDacMeas[iii][jjj].getNoPrefixValue();
            ccGainDAC[iii][jjj] = ccGainDacMeas[iii][jjj].getNoPrefixValue();
            ccOffsetDAC[iii][jjj] = ccOffsetDacMeas[iii][jjj].getNoPrefixValue();
        }
    }
}

void CalibrationConsumer::copyToAllVectors() {
    allGainADC = gainADC;
    allOffsetADC = offsetADC;
    allGainDAC = gainDAC;
    allOffsetDAC = offsetDAC;
    ccAllGainADC = ccGainADC;
    ccAllOffsetADC = ccOffsetADC;
    ccAllGainDAC = ccGainDAC;
    ccAllOffsetDAC = ccOffsetDAC;
}

/*! \todo FCON recheck insieme a controllermain che updata calibration params quando si cambia range. Al momento funzion a perchè dopo la calibrazione di startup, non channelToCalibIdxs è mai vuoto
Ricontrollare se ci sono problemi alla prima chiamata controllerMain in onVcCurrentRangeSelected
*/
void CalibrationConsumer::updateCalibParams(){
    /*! \todo INVIARE NUOVI DATI DI CALIBRAZIONE A fpga DOPO AVERLI CONVERTITIT IN MEASUREMENT PER TUTTI I CANALI*/
    if(channelToCalibIdxs.size()==0){
        return;
    } else {
        std::vector<std::vector<Measurement_t>> allGainAdcMeas;
        std::vector<std::vector<Measurement_t>> allOffsetAdcMeas;
        std::vector<std::vector<Measurement_t>> allGainDacMeas;
        std::vector<std::vector<Measurement_t>> allOffsetDacMeas;
        std::vector<std::vector<Measurement_t>> ccAllGainAdcMeas;
        std::vector<std::vector<Measurement_t>> ccAllOffsetAdcMeas;
        std::vector<std::vector<Measurement_t>> ccAllGainDacMeas;
        std::vector<std::vector<Measurement_t>> ccAllOffsetDacMeas;
        std::vector<uint16_t> allChannelIndexes;

        /*! \note 20230524 MPAC: we convert ALL the calib params for ALL the channels into Measurements_t
        despite we could've calibrated a single board. We send to FPGA EVERYTHING EVERYTIME*/
        allGainAdcMeas.resize(vcCurrentRangesArray.size());
        allGainDacMeas.resize(vcVoltageRangesArray.size());
        allOffsetAdcMeas.resize(vcCurrentRangesArray.size());
        allOffsetDacMeas.resize(vcVoltageRangesArray.size());
        ccAllGainAdcMeas.resize(ccVoltageRangesArray.size());
        ccAllOffsetAdcMeas.resize(ccVoltageRangesArray.size());
        ccAllGainDacMeas.resize(ccCurrentRangesArray.size());
        ccAllOffsetDacMeas.resize(ccCurrentRangesArray.size());
        convertToMeasurement(allGainDacMeas, allGainAdcMeas, allOffsetAdcMeas, allOffsetDacMeas, ccAllGainAdcMeas, ccAllOffsetAdcMeas, ccAllGainDacMeas, ccAllOffsetDacMeas);


        /*! \note 20230524 MPAC: sends the updated params to the message dispatcher, for all the 384 channels, despite I could hae calibrated only one board*/
        CalibrationParams_t calibParamsForMesDis;
        calibParamsForMesDis.allGainDacMeas =     allGainDacMeas;
        calibParamsForMesDis.allGainAdcMeas =     allGainAdcMeas;
        calibParamsForMesDis.allOffsetAdcMeas =   allOffsetAdcMeas;
        calibParamsForMesDis.allOffsetDacMeas =   allOffsetDacMeas;
        calibParamsForMesDis.ccAllGainAdcMeas =   ccAllGainAdcMeas;
        calibParamsForMesDis.ccAllOffsetAdcMeas = ccAllOffsetAdcMeas;
        calibParamsForMesDis.ccAllGainDacMeas =   ccAllGainDacMeas;
        calibParamsForMesDis.ccAllOffsetDacMeas = ccAllOffsetDacMeas;

        mDev->getMessageDispatcher()->setCalibParams(calibParamsForMesDis);

        /*! \note MPAC: sends calib params to FPGA. This part could also be moved to the message dispatcher,as now it has an internal copy of the updated calib params*/
        for(int i = 0; i< currentChannelsNum; i++){
            allChannelIndexes.push_back(i);
        }

        if (mDev->getOngoingClampingModality() == ClampingModality_t::VOLTAGE_CLAMP) {
            mDev->getMessageDispatcher()->setCalibVcCurrentGain(allChannelIndexes, allGainAdcMeas[mDev->getVcCurrentRangeIdx()], true);
            mDev->getMessageDispatcher()->setCalibVcCurrentOffset(allChannelIndexes, allOffsetAdcMeas[mDev->getVcCurrentRangeIdx()], true);

            mDev->getMessageDispatcher()->setCalibVcVoltageGain(allChannelIndexes, allGainDacMeas[mDev->getVcVoltageRangeIdx()], true);
            mDev->getMessageDispatcher()->setCalibVcVoltageOffset(allChannelIndexes, allOffsetDacMeas[mDev->getVcVoltageRangeIdx()], true);
        }

        if (mDev->getOngoingClampingModality() == ClampingModality_t::CURRENT_CLAMP) {
            mDev->getMessageDispatcher()->setCalibCcVoltageGain(allChannelIndexes, ccAllGainAdcMeas[mDev->getCcVoltageRangeIdx()], true);
            mDev->getMessageDispatcher()->setCalibCcVoltageOffset(allChannelIndexes, ccAllOffsetAdcMeas[mDev->getCcVoltageRangeIdx()], true);

            mDev->getMessageDispatcher()->setCalibCcCurrentGain(allChannelIndexes, ccAllGainDacMeas[mDev->getCcCurrentRangeIdx()], true);
            mDev->getMessageDispatcher()->setCalibCcCurrentOffset(allChannelIndexes, ccAllOffsetDacMeas[mDev->getCcCurrentRangeIdx()], true);
        }
    }
}

void CalibrationConsumer::onModelCellChanged(bool modelCellChanged){
    if(modelCellChanged){
        QMutexLocker myLock(&popUpWindowMtx);
        waitForModelCellChanged = false;
    }
}

void CalibrationConsumer::onFirstModelMounted(bool firstModelCellMounted){
    if(firstModelCellMounted){
        QMutexLocker myLock(&popUpWindowMtx);
        waitForFirstModelCellChecked = false;
    }
}

void CalibrationConsumer::calibrateCcAdcGain(int thisActualRangeIdx){
    /*! Representation of voltage steps without any prefix, ...*/
    std::vector<double> x; /*! voltage steps*/
    x.resize(ccCalibrationVoltSteps[thisActualRangeIdx].size());
    for(int i = 0; i< ccCalibrationVoltSteps[thisActualRangeIdx].size(); i++){
        x[i] = ccCalibrationVoltSteps[thisActualRangeIdx][i].getNoPrefixValue();
    }

    /*! Ho settato la maggior parte degli switch prima di chiamare questafunzione, qui abilito
     *  solo lo stimolo in tensione e lo spengo alla fine della funzione*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    /*! FOR: START ciclo sugli step di tensione*/
    for(int voltStepIdx = 0; voltStepIdx <ccCalibrationVoltSteps[thisActualRangeIdx].size(); voltStepIdx++){
        voltageMeans[voltStepIdx].resize(channelToCalibIdxs.size());

        /*! setta la Vhold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
        std::vector<Measurement_t> someVoltSteps;
        for(int i = 0; i < channelToCalibIdxs.size(); i++){
            someVoltSteps.push_back(ccCalibrationVoltSteps[thisActualRangeIdx][voltStepIdx]);
           mDev->getChannels()[channelToCalibIdxs[i]]->setVhold(ccCalibrationVoltSteps[thisActualRangeIdx][voltStepIdx]);
        }
        mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

        /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;//channelToCalibIdxs.size();  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        QThread::sleep(2);
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ*samplesToremove);
        int actualBufferSize = buffer.size();
        buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
        actualBufferSize = buffer.size();

        int timeSamples = actualBufferSize/totalChannelsNum;

         /*! faccio media delle sole tensioni */
        int channelIdx;
        for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
            for (int voltageIdx = 0; voltageIdx < channelToCalibIdxs.size(); voltageIdx++) {
                /*! Mi focalizzo solo sulla prima parte del chunk di buffer, dal momento the ogni
                 *   istante temporale e' rappresentato da 384 tensioni seguite da 384 correnti */
                if(channelToCalibIdxs.size()==currentChannelsNum){
                    channelIdx = bufferIdx+voltageIdx;
                } else {
                    /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                    channelIdx = bufferIdx+voltageIdx + channelToCalibIdxs[0];
                }
                voltageSum[voltageIdx] += buffer[channelIdx]*multiplierVoltage;
            }
        }

        for(int i = 0; i < voltageSum.size(); i++){
            voltageMeans[voltStepIdx][i] = voltageSum[i]/((double)timeSamples);
        }

        buffer.clear(); /*! is resized in getDataChunk()*/
        voltageSum.clear();
        voltageSum.resize(channelToCalibIdxs.size());
        voltageSum.fill(0.0);
    }
    /*! FOR: END ciclo sugli step di tensione*/

    /*! UNA VOLTA CHE HO TUTTE TENSIONI MEDIE PER CIASCUN Vstep PER CIASCUN CANALE, FACCIO MINIMI QUADRATI */
    /*! FOR: START ciclo sui canali*/
    std::vector<double> y; /*! average currents*/
    y.resize(ccCalibrationVoltSteps[thisActualRangeIdx].size());
    std::vector<double> usefulCcAdcGain;
    usefulCcAdcGain.resize(channelToCalibIdxs.size());
    double usefulSlope;
    double uselessOffset;

    for(int chIdx = 0; chIdx < channelToCalibIdxs.size(); chIdx++){
        for(int i = 0; i< ccCalibrationVoltSteps[thisActualRangeIdx].size(); i++){
            y[i] = voltageMeans[i][chIdx];
        }
        /*! calcolo slope con minimi quadrati che sarebbe VADC/VDAC*/
        leastSquareSimple(x, y, usefulSlope, uselessOffset);

        /*! il gain sarebbe VDAC/VADC, i.e. 1/(slope) = 1/(VADC/VDAC)*/
        usefulCcAdcGain[chIdx] = 1/(usefulSlope);
        y.clear();
        y.resize(ccCalibrationVoltSteps[thisActualRangeIdx].size());
    }
    /*! FOR: END ciclo sui canali*/
    ccGainADC[thisActualRangeIdx] = usefulCcAdcGain;

    for(int i = 0; i< ccCalibrationVoltSteps[thisActualRangeIdx].size(); i++){
        voltageMeans[i].clear();
    }

    /*! spegne lo stimolo, i carichi erano già stati staccati fuori dalla funzione*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);

}

void CalibrationConsumer::calibrateCcDacGain(int thisActualRangeIdx){
    /*! Representation of current steps without any prefix, ...*/
    std::vector<double> x; /*! current steps*/
    x.resize(ccCalibrationCurrSteps[thisActualRangeIdx].size());
    for(int i = 0; i< ccCalibrationCurrSteps[thisActualRangeIdx].size(); i++){
        x[i] = ccCalibrationCurrSteps[thisActualRangeIdx][i].getNoPrefixValue();
    }

    /*! Ho settato la maggior parte degli switch prima di chiamare questafunzione, qui abilito
     *  solo lo stimolo in corrente e lo spengo alla fine della funzione*/
    turnSomeCcStimulaOnOff(channelToCalibIdxs, someTrue);
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    /*! FOR: START ciclo sugli step di corrente*/
    for(int currStepIdx = 0; currStepIdx <ccCalibrationCurrSteps[thisActualRangeIdx].size(); currStepIdx++){
        voltageMeans[currStepIdx].resize(channelToCalibIdxs.size());

        /*! setta la Chold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
        std::vector<Measurement_t> someCurrSteps;
        /*! \todo MPAC: non abbiamo una setChold nel modello, al momento non aggiorniamo il modello per questa cosa*/
        for(int i = 0; i < channelToCalibIdxs.size(); i++){
            someCurrSteps.push_back(ccCalibrationCurrSteps[thisActualRangeIdx][currStepIdx]);
           //mDev->getChannels()[channelToCalibIdxs[i]]->setChold(ccCalibrationCurrSteps[thisActualRangeIdx][currStepIdx]);
        }
        mDev->getMessageDispatcher()->setCurrentHoldTuner(channelToCalibIdxs, someCurrSteps, true);

        /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;//channelToCalibIdxs.size();  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        QThread::sleep(2);
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ*samplesToremove);
        int actualBufferSize = buffer.size();
        buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
        actualBufferSize = buffer.size();

        int timeSamples = actualBufferSize/totalChannelsNum;

         /*! faccio media delle sole tensioni */
        int channelIdx;
        for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
            for (int voltageIdx = 0; voltageIdx < channelToCalibIdxs.size(); voltageIdx++) {
                /*! Mi focalizzo solo sulla prima parte del chunk di buffer, dal momento the ogni
                 *   istante temporale e' rappresentato da 384 tensioni seguite da 384 correnti */
                if(channelToCalibIdxs.size()==currentChannelsNum){
                    channelIdx = bufferIdx+voltageIdx;
                } else {
                    /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                    channelIdx = bufferIdx+voltageIdx + channelToCalibIdxs[0];
                }
                voltageSum[voltageIdx] += buffer[channelIdx]*multiplierVoltage;
            }
        }

        for(int i = 0; i < voltageSum.size(); i++){
            // voltageSum conteine un element per canale, i.e. 384
            // qui compenso con i guadagni di tensione calcolati al punto precente
            voltageMeans[currStepIdx][i] = ccGainADC[thisActualRangeIdx][i] * voltageSum[i]/((double)timeSamples);
        }

        buffer.clear(); /*! is resized in getDataChunk()*/
        voltageSum.clear();
        voltageSum.resize(channelToCalibIdxs.size());
        voltageSum.fill(0.0);
    }
    /*! FOR: END ciclo sugli step di corrente*/

    /*! UNA VOLTA CHE HO TUTTE TENSIONI MEDIE PER CIASCUN Istep PER CIASCUN CANALE, FACCIO MINIMI QUADRATI */
    /*! FOR: START ciclo sui canali*/
    std::vector<double> y; /*! average currents*/
    y.resize(ccCalibrationCurrSteps[thisActualRangeIdx].size());
    std::vector<double> usefulCcDacGain;
    usefulCcDacGain.resize(channelToCalibIdxs.size());
    double usefulSlope;
    double uselessOffset;

    for(int chIdx = 0; chIdx < channelToCalibIdxs.size(); chIdx++){
        for(int i = 0; i< ccCalibrationCurrSteps[thisActualRangeIdx].size(); i++){
            y[i] = voltageMeans[i][chIdx];
        }
        /*! calcolo slope con minimi quadrati che sarebbe VADC/Itest_ma_ancora_da_calibrare = Restim*/
        leastSquareSimple(x, y, usefulSlope, uselessOffset);

        /*! il gain sarebbe slopeStimata/Rcalib = Restim/Rcalib, a cui devo moltiplicare la corrente applicata dal DAC, per avere il suo valore impostato da GUI*/
        /*! \todo FCON: recheck, diverso che in tabella*/
        usefulCcDacGain[chIdx] = ccCalibratonResistances[thisActualRangeIdx].getNoPrefixValue()/usefulSlope;
        y.clear();
        y.resize(ccCalibrationCurrSteps[thisActualRangeIdx].size());
    }
    /*! FOR: END ciclo sui canali*/
    ccGainDAC[thisActualRangeIdx] = usefulCcDacGain;

    for(int i = 0; i< ccCalibrationCurrSteps[thisActualRangeIdx].size(); i++){
        voltageMeans[i].clear();
    }

    /*! spegne lo stimolo, i carichi erano già stati staccati fuori dalla funzione*/
    turnSomeCcStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
}

void CalibrationConsumer::calibrateCcAdcOffset(RangedMeasurement_t thisActualRange){
    /*!  applico  0nA ai canali selezionati*/
    std::vector<Measurement_t> someCurrSteps;
    /*! \todo MPAC: non abbiamo una setChold nel modello, al momento non aggiorniamo il modello per questa cosa*/
    for(int i = 0; i < channelToCalibIdxs.size(); i++){
        someCurrSteps.push_back({0.0, UnitPfxNano, "A"});
       //mDev->getChannels()[channelToCalibIdxs[i]]->setChold({0.0, UnitPfxMilli, "V"});
    }
    mDev->getMessageDispatcher()->setCurrentHoldTuner(channelToCalibIdxs, someCurrSteps, true);

    /*! accende lo stimolo su tutti i canali  o su quelli della scheda selezionata*/
    /*! gli switch di ingresso sono staccati dal passo precedente*/
    turnSomeCcStimulaOnOff(channelToCalibIdxs, someTrue);
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    voltageMeans[0].resize(channelToCalibIdxs.size());

    /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
    la cui struttura è ancora da definire */
    sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
    minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
    samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
    QThread::sleep(2);
    hook->flush(); /*! Remove old buffered data */
    while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

    /*! butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
    buffer.remove(0, CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ*samplesToremove);
    int actualBufferSize = buffer.size();
    buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
    actualBufferSize = buffer.size();

    int timeSamples = actualBufferSize/totalChannelsNum;

     /*! \todo  faccio media delle sole correnti */
    int channelIdx;
    for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
        for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
            if(channelToCalibIdxs.size()==currentChannelsNum){
                channelIdx = bufferIdx+currentIdx;
            } else {
                /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                channelIdx = bufferIdx+currentIdx + channelToCalibIdxs[0];
            }
            voltageSum[currentIdx] += buffer[channelIdx]*multiplierVoltage;
        }
    }

    std::vector<double> usefulAdcOffset;
    usefulAdcOffset.resize(channelToCalibIdxs.size());

    /*! moltiplico la tensione media per i GAIN ADC calacolati al passo precedente e dovrei avere già l'offset di ADC*/
    for(int i = 0; i < voltageSum.size(); i++){
        usefulAdcOffset[i] = -(ccGainADC[rangeIdx][i] * (voltageSum[i]/((double)timeSamples) - thisActualRange.getMin().getNoPrefixValue()) + thisActualRange.getMin().getNoPrefixValue()); //V
    }

    buffer.clear(); /*! is resized in getDataChunk()*/
    voltageSum.clear();
    voltageSum.resize(channelToCalibIdxs.size());
    voltageSum.fill(0.0);

    ccOffsetADC[rangeIdx] = usefulAdcOffset;

    /*! spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeCcStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
}

void CalibrationConsumer::calibrateCcDacOffset(RangedMeasurement_t thisActualRange, int thisCcVoltageActualRangeIdx){
    int numTries = 0;
    std::vector<bool> needsFurtherCalibration;
    needsFurtherCalibration.resize(channelToCalibIdxs.size());

    std::vector<double> adcCompensatedVoltage;
    adcCompensatedVoltage.resize(channelToCalibIdxs.size());

    /*! applico  0nA ai canali selezionati*/
    std::vector<Measurement_t> someCurrSteps;
    for(int i = 0; i < channelToCalibIdxs.size(); i++){
       someCurrSteps.push_back({0.0, UnitPfxNano, "A"});
       // mDev->getChannels()[channelToCalibIdxs[i]]->setChold({0.0, UnitPfxNano, "A"});
       needsFurtherCalibration[i] = true;
    }
    mDev->getMessageDispatcher()->setCurrentHoldTuner(channelToCalibIdxs, someCurrSteps, true);

    /*! accende lo stimolo, i CAL_SW erano stati attaccati fuori dalla funzione*/
    turnSomeCcStimulaOnOff(channelToCalibIdxs, someTrue);
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    voltageMeans[0].resize(channelToCalibIdxs.size());

    std::vector<double> usefulDacOffset;
    usefulDacOffset.resize(channelToCalibIdxs.size());

    while(numTries < CCS_DAC_OFFSET_MINIMIZATION_MAX_TRY){
        /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        QThread::sleep(2);
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!  butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, CCS_CALIB_MULTIPLIER_FOR_INIT_ACQ*samplesToremove);
        int actualBufferSize = buffer.size();
        buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
        actualBufferSize = buffer.size();

        int timeSamples = actualBufferSize/totalChannelsNum;

         /*!  faccio media delle sole correnti */
        int channelIdx;
        for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
            for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
                if(channelToCalibIdxs.size()==currentChannelsNum){
                    channelIdx = bufferIdx+currentIdx;
                } else {
                    /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                    channelIdx = bufferIdx+currentIdx + channelToCalibIdxs[0];
                };
                voltageSum[currentIdx] += buffer[channelIdx]*multiplierVoltage;
            }
        }

        /*! moltiplico la corrente media per i GAIN ADC  e sottraggo offset ADC calacolati per tenere conto delle calibrazioni precedenti*/
        for(int i = 0; i < currentSum.size(); i++){
            adcCompensatedVoltage[i] = (ccGainADC[thisCcVoltageActualRangeIdx][i] * (voltageSum[i]/((double)timeSamples) - thisActualRange.getMin().getNoPrefixValue()) + thisActualRange.getMin().getNoPrefixValue()) + ccOffsetADC[thisCcVoltageActualRangeIdx][i];
            if (adcCompensatedVoltage[i] == 0.0){
               needsFurtherCalibration[i] = false;
           } else {
               /*! sottraggo allo step di corrente attualmente applicato*/
                double poffi = ccCalibratonResistances[thisCcVoltageActualRangeIdx].getNoPrefixValue(); // Ohm
                double bubbi = adcCompensatedVoltage[i]/poffi - someCurrSteps[i].getNoPrefixValue(); // A
                someCurrSteps[i].value = bubbi/someCurrSteps[i].multiplier(); //nA perchè divido I per 1e-9
           }
           usefulDacOffset[i] = -(someCurrSteps[i].getNoPrefixValue()); //A
        }

        /*! mandi via messageDispatcher i valori aggiornati di voltage step per vedere se la lettura sui canali mi diventa finalmetne 0 */
        mDev->getMessageDispatcher()->setCurrentHoldTuner(channelToCalibIdxs, someCurrSteps, true);

        buffer.clear(); /*! is resized in getDataChunk()*/
        voltageSum.clear();
        voltageSum.resize(channelToCalibIdxs.size());
        voltageSum.fill(0.0);

        ccOffsetDAC[rangeIdx] = usefulDacOffset;

        numTries++;
    }

    /*!  spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeCcStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
}
