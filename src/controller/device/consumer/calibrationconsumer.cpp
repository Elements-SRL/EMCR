#include "calibrationconsumer.h"
#include "messagedispatcher.h"
#include "errormanager.h"

#include <QTime>
#include <QDebug>

CalibrationConsumer::CalibrationConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer){

    std::vector <Measurement_t> aaa;
//    DeviceTypes_t ccc;

    mDev->getSamplingRatesFeatures(aaa);
    calibrationSamplingRate = aaa[0];

    mDev->getVcCurrentRangesFeatures(vcCurrentRangesArray, defaultVcCurrRangeIdx);
    mDev->getVcVoltageRangesFeatures(vcVoltageRangesArray);
    mDev->getCcCurrentRangesFeatures(ccCurrentRangesArray);
    mDev->getCcVoltageRangesFeatures(ccVoltageRangesArray);

    mDev->getMessageDispatcher()->getDeviceType(mDev->getSerialNumber().toStdString(), deviceUnderCalibrationType);
    mDev->getBoardsNumberFeatures(numOfBoards);
    numOfChannelsOnBoard = currentChannelsNum/numOfBoards;

    suspectChannelIdxs.resize(currentChannelsNum);
    fill(suspectChannelIdxs.begin(), suspectChannelIdxs.end(), false);

    mDev->getMessageDispatcher()->getCalibDefaultVcAdcGain(defaultAdcGainValue);    //1.57014;
    mDev->getMessageDispatcher()->getCalibDefaultVcAdcOffset(defaultAdcOffsetValue); // 0.0;
    mDev->getMessageDispatcher()->getCalibDefaultVcDacOffset(defaultDacOffsetValue); // 0.0;

    if(deviceUnderCalibrationType == Device384Nanopores
        #ifdef DEBUG
            || deviceUnderCalibrationType == Device384Fake
        #endif
            ){
        mDev->getCalibDataFeatures(calibData);
        calibrationVoltSteps = calibData.vcCalibStepsArrays;
        calibratonResistances = calibData.vcCalibResArray;
        areCalibResistOnBoard = calibData.areCalibResistOnBoard;
    } else if (deviceUnderCalibrationType == Device384PatchClamp
           #ifdef DEBUG
               || deviceUnderCalibrationType == Device384FakePatchClamp
           #endif
               ){
        /*! \todo add settings for PatchClamp in case we use this same class  */
        mDev->getCalibDataFeatures(calibData);
        calibrationVoltSteps = calibData.vcCalibStepsArrays;
        calibratonResistances = calibData.vcCalibResArray;
        areCalibResistOnBoard = calibData.areCalibResistOnBoard;
        ccCalibrationVoltSteps = calibData.ccCalibVoltStepsArrays;
        ccCalibrationCurrSteps = calibData.ccCalibCurrStepsArrays;
        ccCalibratonResistances = calibData.ccCalibResArray;
        ccCalibratonResisForCcAdcOffset = calibData.ccCalibResForCcAdcOffsetArray; // only for ccVoltageOffset (ADC)
    }

    gainADC.resize(vcCurrentRangesArray.size());
    offsetADC.resize(vcCurrentRangesArray.size());

    allGainADC.resize(vcCurrentRangesArray.size());
    allOffsetADC.resize(vcCurrentRangesArray.size());

    for(int i = 0; i< vcCurrentRangesArray.size(); i++){
        allGainADC[i].resize(currentChannelsNum);
        allOffsetADC[i].resize(currentChannelsNum);
    }

    offsetDAC.resize(currentChannelsNum);
    allOffsetDAC.resize(currentChannelsNum);


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
//    for(int i = 0; i < numOfBoards; i++){
//        boardSerialNums[i] = QString("serial_%1").arg(i);
//    }


    /*! EXAMPLE OF boardMapping.csv*/
//    0,EL_board_1
//    1,EL_board_2
//    2,EL_board_3
//    3,EL_board_4
//    4,EL_board_5
//    5,EL_board_6
//    6,EL_board_7
//    7,EL_board_8
//    8,EL_board_9
//    9,EL_board_10
//    10,EL_board_11
//    11,EL_board_12
//    12,EL_board_13
//    13,EL_board_14
//    14,EL_board_15
//    15,EL_board_16
//    16,EL_board_17
//    17,EL_board_18
//    18,EL_board_19
//    19,EL_board_20
//    20,EL_board_21
//    21,EL_board_22
//    22,EL_board_23
//    23,EL_board_24

}

CalibrationConsumer::~CalibrationConsumer(){

}

void CalibrationConsumer::run(){
    consumptionStopped = false;
    exitedDataConsumingLoop = false;


//    totalChannelsUnderCalibNum = 2*channelToCalibIdxs.size();

    currentSum.resize(channelToCalibIdxs.size());
    currentSum.fill(0.0);
    currentMeans.resize(calibrationVoltSteps[0].size());

    voltageSum.resize(channelToCalibIdxs.size());
    voltageSum.fill(0.0);
    voltageMeans.resize(ccCalibrationCurrSteps[0].size());


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
        vector <Measurement_t> samplingRates;
        mDev->getSamplingRatesFeatures(samplingRates);
        mDev->getMessageDispatcher()->setSamplingRate(0, true);
        mDev->setSamplingRate(samplingRates[0]);

        /*! \todo MPAC se qualcosa va male, chiama qui la vecchia procedure di calibrazione per nanopore*/
//        oldVcOnlyCalibration();


        uint16_t bbb; // buffer variable used sometimes.

        /*! MPAC imposto qui la condizione di VC se serve*/
        if(deviceUnderCalibrationType == Device384PatchClamp
            #ifdef DEBUG
                || deviceUnderCalibrationType == Device384FakePatchClamp
            #endif
           ){
            setVcConfiguration(channelToCalibIdxs, someTrue, someFalse);
        }

        /*! spegne lo stimolo e stacco il carico su tutti i canali per concentire all'utente di cambiare la model cell se c'è quella sbagliata*/
        turnAllStimulaOnOff(false);
        turnAllChannelsOnOff(false);
        if(areCalibResistOnBoard){
            turnAllCalSwOnOff(false);
        }

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
        qDebug() << "[CALIBRATIONCONSUMER] MI FERMO\n";
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
            vector <RangedMeasurement_t> rangeInfo;
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
                qDebug() << "[CALIBRATIONCONSUMER] MI FERMO\n";
                while(true){
                    QMutexLocker myLock(&popUpWindowMtx);
                    if(!waitForModelCellChanged){
                        break;
                    }
                    myLock.unlock();
                    QThread::msleep(10);
                }
            }
            qDebug() << "[CALIBRATIONCONSUMER] RIPARTO\n";

        /*! FOR: END ciclo sui range*/
        }

        /*! START CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
        calibrateDacOffset(vcCurrentRangesArray[rangeIdx]);
        /*! END CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/




        /*! Qui inizia la parte di calibrazione in CC*/
        /*! Open all IN_SW*/
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
            setSourceForVoltageChannel(0);
            setSourceForCurrentChannel(0);

            /*! \todo CCVgain ADC*/

            /*! FOR: START ciclo sui range di tensione in CC*/
            for(int jjj = 0; jjj <ccVoltageRangesArray.size(); jjj++){
                rangeIdx = jjj;

                /*! setto il range di tensione per Current Clamp*/
                vector <RangedMeasurement_t> rangeInfo;
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

            /*! FOR: START ciclo sui range di corrente in CC*/
            for(int jjj = 0; jjj <ccCurrentRangesArray.size(); jjj++){
                rangeIdx = jjj;

                /*! setto il range di tensione per Current Clamp*/
                vector <RangedMeasurement_t> rangeInfo;
                mDev->getCcCurrentRangesFeatures(rangeInfo);
                mDev->getMessageDispatcher()->setCCCurrentRange(rangeIdx, true);
                multiplierCurrent = rangeInfo[rangeIdx].multiplier();

                /*! START CALCOLO CC DAC GAIN (CCIgain)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
               calibrateCcDacGain(rangeIdx);
               /*! END CALCOLO CC DAC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            }

            /*! \note MPAC: faccio il giochino di staccare i CAL_SW e attaccare gli IN_SW, in modo da sfruttare per questo passaggio le resistenze piccole montate sulla model cell da 120 kOhm*/
            turnSomeCalSwOnOff(channelToCalibIdxs, someFalse);
            turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);

            /*! FOR: START ciclo sui range di tensione in CC*/
            /*! \todo MPAC: al 20230518 sono arrivato qui, ricontrolla e continua*/
            for(int jjj = 0; jjj <ccVoltageRangesArray.size(); jjj++){
                rangeIdx = jjj;

                /*! setto il range di tensione per Current Clamp*/
                vector <RangedMeasurement_t> rangeInfo;
                mDev->getCcVoltageRangesFeatures(rangeInfo);
                mDev->getMessageDispatcher()->setCCVoltageRange(rangeIdx, true);
                multiplierVoltage = rangeInfo[rangeIdx].multiplier();

                /*! START CALCOLO CC ADC OFFSET (CCVoffset)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//               calibrateCcAdcOffset(rangeIdx);
               /*! END CALCOLO CC ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            }

            /*! \note MPAC: risistemo gli switch, CAL_SW attaccati e IN_SW staccati*/
            turnSomeCalSwOnOff(channelToCalibIdxs, someTrue);
            turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);




            /*! \todo CCVoffset ADC, quello piu' problematico*/

            /*! \todo CCIoffset DAC*/

        }






        someFalse.clear();
        someTrue.clear();



        /*! AGGIORNO LE STRUTTURA IN CUI TENGO TUTTI I VALORI DI CALIBRAZIONE, allGainADC, allOffsetADc, allOffsetDAC*/
        for(int zzz = 0; zzz < vcCurrentRangesArray.size(); zzz++){
            for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
                allGainADC[zzz][channelToCalibIdxs[xxx]] = gainADC[zzz][xxx];
                allOffsetADC[zzz][channelToCalibIdxs[xxx]] = offsetADC[zzz][xxx];
                if(abs(allGainADC[zzz][xxx]) > gainThreshForSuspect){
                    suspectChannelIdxs[channelToCalibIdxs[xxx]] = true;
                }
            }
        }
        for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
            allOffsetDAC[channelToCalibIdxs[xxx]] = offsetDAC[xxx];
        }

        /*! salvo queste info su CSV la cui struttura deve essere ancora decisa. Forse un file per ciascuna scheda*/
        mainSaveOnCsv();

        /*! ALLA FINE DI TUTTO SETTO IL VC CURRENT RANGE DI DEFAULT*/
        mDev->getMessageDispatcher()->setVCCurrentRange(defaultVcCurrRangeIdx, true);

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

void CalibrationConsumer::oldVcOnlyCalibration(){
    uint16_t bbb; //useless

    /*! spegne lo stimolo e stacco il carico su tutti i canali per concentire all'utente di cambiare la model cell se c'è quella sbagliata*/
    turnAllStimulaOnOff(false);
    turnAllChannelsOnOff(false);
    if(areCalibResistOnBoard){
        turnAllCalSwOnOff(false);
    }

    QString msg = "Calibration will start in the range " + QString::fromStdString(vcCurrentRangesArray[0].niceLabel())+ ". Make sure you mounted the " + QString::fromStdString(calibratonResistances[0].niceLabel()) + " model cell.\nPress OK to continue.\n";
    waitForFirstModelCellChecked = true;
    emit sigNeedToCheckFirstModelCellMsg(msg);
    qDebug() << "[CALIBRATIONCONSUMER] MI FERMO\n";
    while(true){
        QMutexLocker myLock(&popUpWindowMtx);
        if(!waitForFirstModelCellChecked){
            break;
        }
        myLock.unlock();
        QThread::msleep(10);
    }

    /*! FOR: START ciclo sui range*/
    for(int jjj = 0; jjj <vcCurrentRangesArray.size(); jjj++){
        rangeIdx = jjj;

        /*! setto il range di corrente per Voltage Clamp*/
        vector <RangedMeasurement_t> rangeInfo;
        mDev->getVcCurrentRangesFeatures(rangeInfo, bbb);
        mDev->getMessageDispatcher()->setVCCurrentRange(rangeIdx, true);
        multiplierCurrent = rangeInfo[rangeIdx].multiplier();

        /*! spegne lo stimolo e stacco il carico su tutti i canali */
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
            QString msg = "Need to mount the model cell " + QString::fromStdString(calibratonResistances[rangeIdx+1].niceLabel()) + " for current range " + QString::fromStdString(vcCurrentRangesArray[rangeIdx+1].niceLabel())+"\nPress OK only once the model cell has been changed.\n";
            waitForModelCellChanged = true;
            emit sigNeedToChangeModelCellMsg(msg);
            qDebug() << "[CALIBRATIONCONSUMER] MI FERMO\n";
            while(true){
                QMutexLocker myLock(&popUpWindowMtx);
                if(!waitForModelCellChanged){
                    break;
                }
                myLock.unlock();
                QThread::msleep(10);
            }
        }
        qDebug() << "[CALIBRATIONCONSUMER] RIPARTO\n";

    /*! FOR: END ciclo sui range*/
    }

    /*! START CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    calibrateDacOffset(vcCurrentRangesArray[rangeIdx]);
    /*! END CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    someFalse.clear();
    someTrue.clear();



    /*! AGGIORNO LE STRUTTURA IN CUI TENGO TUTTI I VALORI DI CALIBRAZIONE, allGainADC, allOffsetADc, allOffsetDAC*/
    for(int zzz = 0; zzz < vcCurrentRangesArray.size(); zzz++){
        for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
            allGainADC[zzz][channelToCalibIdxs[xxx]] = gainADC[zzz][xxx];
            allOffsetADC[zzz][channelToCalibIdxs[xxx]] = offsetADC[zzz][xxx];
            if(abs(allGainADC[zzz][xxx]) > gainThreshForSuspect){
                suspectChannelIdxs[channelToCalibIdxs[xxx]] = true;
            }
        }
    }
    for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
        allOffsetDAC[channelToCalibIdxs[xxx]] = offsetDAC[xxx];
    }

    /*! salvo queste info su CSV la cui struttura deve essere ancora decisa. Forse un file per ciascuna scheda*/
    mainSaveOnCsv();

    /*! ALLA FINE DI TUTTO SETTO IL VC CURRENT RANGE DI DEFAULT*/
    mDev->getMessageDispatcher()->setVCCurrentRange(defaultVcCurrRangeIdx, true);

    /*! Invio dati a FPGA con massageDispatcher*/
    updateCalibParams();
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
    vector<double> x; /*! voltage steps*/
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
        currentMeans[voltStepIdx].resize(channelToCalibIdxs.size());

        /*! setta la Vhold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
        vector<Measurement_t> someVoltSteps;
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
        buffer.remove(0, samplesToremove);
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
    vector<double> y; /*! average currents*/
    y.resize(calibrationVoltSteps[thisActualRangeIdx].size());
    vector<double> usefulAdcGain;
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
    vector<Measurement_t> someVoltSteps;
    for(int i = 0; i < channelToCalibIdxs.size(); i++){
        someVoltSteps.push_back({0.0, UnitPfxMilli, "V"});
       mDev->getChannels()[channelToCalibIdxs[i]]->setVhold({0.0, UnitPfxMilli, "V"});
    }
    mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

    /*! accende lo stimolo su tutti i canali  o su quelli della scheda selezionata*/
    /*! gli switch di ingresso sono staccati dal passo precedente*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    currentMeans[0].resize(channelToCalibIdxs.size());

    /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
    la cui struttura è ancora da definire */
    sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
    minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
    samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
    hook->flush(); /*! Remove old buffered data */
    while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

    /*! butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
    buffer.remove(0, samplesToremove);
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

    vector<double> usefulAdcOffset;
    usefulAdcOffset.resize(channelToCalibIdxs.size());

    /*! moltiplico la corrente media per i GAIN calacolati al passo precedente e dovrei avere già l'offset di ADC*/
    for(int i = 0; i < currentSum.size(); i++){
//       usefulAdcOffset[i] = gainADC[rangeIdx][i] * currentSum[i]/((double)timeSamples); /*! \todo FCON la moltiplicazione per il gain non simula correttamente quello che accade in FPGA, meglio far fare il conto all'FPGA usando il comando del MessageDispatcher
        usefulAdcOffset[i] = -(gainADC[rangeIdx][i] * (currentSum[i]/((double)timeSamples) - thisActualRange.getMin().getNoPrefixValue()) + thisActualRange.getMin().getNoPrefixValue());

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

void CalibrationConsumer::calibrateDacOffset(RangedMeasurement_t thisActualRange){
    int numTries = 0;
    vector<bool> needsFurtherCalibration;
    needsFurtherCalibration.resize(channelToCalibIdxs.size());

    vector<double> adcCompensatedCurrent;
    adcCompensatedCurrent.resize(channelToCalibIdxs.size());

    /*! applico  0V ai canali selezionati*/
    vector<Measurement_t> someVoltSteps;
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

    currentMeans[0].resize(channelToCalibIdxs.size());

    vector<double> usefulDacOffset;
    usefulDacOffset.resize(channelToCalibIdxs.size());

    while(numTries <= CCS_DAC_OFFSET_MINIMIZATION_MAX_TRY){
        /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!  butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, samplesToremove);
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
//            (gainADC[rangeIdx][i] * (currentSum[i]/((double)timeSamples) - thisActualRange.getMin().getNoPrefixValue()) + thisActualRange.getMin().getNoPrefixValue());
//            adcCompensatedCurrent[i] = gainADC[rangeIdx][i] * currentSum[i]/((double)timeSamples) + offsetADC[rangeIdx][i]; /*! \todo FCON vedi commento nel calcolo dell'offset dell'ADC: far fare la calibrazione parziale in FPGA invece che in SW */
            adcCompensatedCurrent[i] = (gainADC[rangeIdx][i] * (currentSum[i]/((double)timeSamples) - thisActualRange.getMin().getNoPrefixValue()) + thisActualRange.getMin().getNoPrefixValue()) + offsetADC[rangeIdx][i];
            if (adcCompensatedCurrent[i] == 0.0){
               needsFurtherCalibration[i] = false;
           } else {
               /*! sottraggo allo step di tensione attualmente applicato*/
                double poffi = calibratonResistances[rangeIdx].getNoPrefixValue(); // Ohm
                double bubbi = someVoltSteps[i].getNoPrefixValue() - adcCompensatedCurrent[i]*poffi; // V
                someVoltSteps[i].value = bubbi/someVoltSteps[i].multiplier(); //mV perchè divido V per 1e-3
           }
           offsetDAC[i] = -(someVoltSteps[i].getNoPrefixValue()); //V
        }

        /*! mandi via messageDispatcher i valori aggiornati di voltage step per vedere se la lettura sui canali mi diventa finalmetne 0 */
        mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

        buffer.clear(); /*! is resized in getDataChunk()*/
        currentSum.clear();
        currentSum.resize(channelToCalibIdxs.size());
        currentSum.fill(0.0);

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
void CalibrationConsumer::onPerformCalibration(vector<uint16_t> channelsToCalibrateIdxs){
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
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    for (int i = 0; i < currentChannelsNum; i++){
        this->mDev->getChannels()[i]->setOn(onValue);
        channelIndexes[i] = i;
        onValues[i] = onValue;
        qDebug() << "[Channel " << i << "]: on/off status:" << onValue << "\n";
    }
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
}

void CalibrationConsumer::turnAllStimulaOnOff(bool onValue){
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    for (int i = 0; i < currentChannelsNum; i++){
        this->mDev->getChannels()[i]->setInStimActive(onValue);
        channelIndexes[i] = i;
        onValues[i] = onValue;
        qDebug() << "[Channel " << i << "]: on/off status:" << onValue << "\n";
    }
    this->mDev->getMessageDispatcher()->enableStimulus(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllCalSwOnOff(bool onValue){
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
//    for (int i = 0; i < currentChannelsNum; i++){
//        this->mDev->getChannels()[i]->setInStimActive(onValue);
//        channelIndexes[i] = i;
//        onValues[i] = onValue;
//        qDebug() << "[Channel " << i << "]: on/off status:" << onValue << "\n";
//    }
    this->mDev->getMessageDispatcher()->turnCalSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllVcSwOnOff(bool onValue){
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->turnVcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllCcSwOnOff(bool onValue){
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->turnCcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllVcCcSelOnOff(bool onValue){
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->turnVcCcSelOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnAllCcStimulaOnOff(bool onValue){
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(currentChannelsNum);
    onValues.resize(currentChannelsNum);
    this->mDev->getMessageDispatcher()->enableCcStimulus(channelIndexes, onValues, true);
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::selectSomeChannels(vector<uint16_t> channelIndexes, vector<bool> selectValues){
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setSelected(selectValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: selected status:" << selectValues[i] << "\n";
    }
}

/*! \todo MPAC: vogliamo mettere un Cal_SW anche nelmodelChannle con sua set e get???*/
void CalibrationConsumer::turnSomeChannelsOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setOn(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
    }
}

void CalibrationConsumer::turnSomeStimulaOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->enableStimulus(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setInStimActive(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
    }
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeCalSwOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnCalSwOn(channelIndexes, onValues, true);
//    for (int i = 0; i < channelIndexes.size(); i++){
//        this->mDev->getChannels()[channelIndexes[i]]->setInStimActive(onValues[i]);
//        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
//    }
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeVcSwOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnVcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeCcSwOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnCcSwOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeVcCcSelOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnVcCcSelOn(channelIndexes, onValues, true);
}

/*! \todo MPAC: anche qui ancora non aggiorniamo il modelChannel. Vogliamo farlo???*/
void CalibrationConsumer::turnSomeCcStimulaOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->enableCcStimulus(channelIndexes, onValues, true);
}

void CalibrationConsumer::setSourceForVoltageChannel(uint16_t source){
    this->mDev->getMessageDispatcher()->setSourceForVoltageChannel(source, true);
}

void CalibrationConsumer::setSourceForCurrentChannel(uint16_t source){
    this->mDev->getMessageDispatcher()->setSourceForCurrentChannel(source, true);
}



void CalibrationConsumer::setVcConfiguration(vector<uint16_t> channelIndexes, vector<bool> someTrue, vector<bool> someFalse){
    turnSomeCalSwOnOff(channelIndexes, someTrue);
    turnSomeVcSwOnOff(channelIndexes, someTrue);
    turnSomeCcSwOnOff(channelIndexes, someFalse);
    turnSomeCcStimulaOnOff(channelIndexes, someFalse);
    turnSomeVcCcSelOnOff(channelIndexes, someTrue);
    setSourceForVoltageChannel(0);
    setSourceForCurrentChannel(0);
}

void CalibrationConsumer::setCcConfiguration(vector<uint16_t> channelIndexes, vector<bool> someTrue, vector<bool> someFalse){
    turnSomeCalSwOnOff(channelIndexes, someTrue);
    turnSomeVcSwOnOff(channelIndexes, someFalse);
    turnSomeCcSwOnOff(channelIndexes, someTrue);
    turnSomeCcStimulaOnOff(channelIndexes, someTrue);
    turnSomeVcCcSelOnOff(channelIndexes, someFalse);
    setSourceForVoltageChannel(1);
    setSourceForCurrentChannel(1);
}

void CalibrationConsumer::leastSquareSimple(vector<double> x, vector<double> y, double &slope, double &offset){
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
    if(channelToCalibIdxs.size() == currentChannelsNum){
        for(int i = 0; i < numOfBoards; i++){
            /*! calibro tutte le board*/
            vector<uint16_t>::iterator first = channelToCalibIdxs.begin() + numOfChannelsOnBoard*i; // incluso
            vector<uint16_t>::iterator last = channelToCalibIdxs.begin() + numOfChannelsOnBoard*i + (numOfChannelsOnBoard); // escluso
            vector<uint16_t> chanSubsetToCalibIdxs(first, last);
            fileName = boardSerialNums[i] + QString(".csv");
            prepareStuffToSaveOnCsv(calibrationFilesFolder, fileName, chanSubsetToCalibIdxs);
        }
        QString msg = "All boards manual calibration successfull!\n";
        msg = msg + suspectChannelsMsg(channelToCalibIdxs);
        emit sigManualCalibDoneMsg(msg);
    } else {
        /*! calibro solo una board*/
        fileName = boardSerialNums[channelToCalibIdxs[0]/numOfChannelsOnBoard] + QString(".csv");
        prepareStuffToSaveOnCsv(calibrationFilesFolder, fileName, channelToCalibIdxs);
        QString msg = "Board " + QString("%1").arg(1+channelToCalibIdxs[0]/numOfChannelsOnBoard) +" manual calibration successfull!\n";
        msg = msg + suspectChannelsMsg(channelToCalibIdxs);
        emit sigManualCalibDoneMsg(msg);

    }

}

QString CalibrationConsumer::suspectChannelsMsg(vector<uint16_t> chanToCalibIdxs){
    QString msgSusp = "";
    vector<int> listOfSuspectIdxs;
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

void CalibrationConsumer::prepareStuffToSaveOnCsv(QString path, QString fileName, vector<uint16_t> chanSubset){
    QFile outFile(path + fileName);
    QTextStream stream;
        if (QDir().exists(path)) {
            outFile.open(QFile::WriteOnly);
            if (outFile.isOpen()) {
                stream.setDevice(&outFile);
                this->saveCsv(chanSubset, stream);
            }
            outFile.close();

        } else {
            if (QDir().mkpath(path)) {
                if (outFile.open(QFile::WriteOnly )) {
                    stream.setDevice(&outFile);
                    this->saveCsv(chanSubset, stream);
                    outFile.close();
                }
            }
        }
}

void CalibrationConsumer::saveCsv(vector<uint16_t> chanSubset, QTextStream &stream){
    stream << this->getCsvData(chanSubset);
}

QString CalibrationConsumer::getCsvData(vector<uint16_t> chanSubset){
    QString ret;
    QTextStream stream(&ret);

    stream << QString("%1").arg(boardSerialNums[chanSubset[0]/numOfChannelsOnBoard]) << "\n";
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
    /*! \todo bisogna adattare questa funzione e la calibrazione del DAC offset in caso ci siano più VC voltage ranges*/
//    for(int i = 0; i < vcVoltageRangesArray.size(); i++){
        stream << QString("%1").arg(vcVoltageRangesArray[0].max) << "\n";
        for(int j = 0; j < chanSubset.size(); j++){
            /*! \todo 30 decimali, solo per vedere qualcosa, questo numero sarà da ridurre*/
            if(offsetDAC.size()==currentChannelsNum){
                stream << QString("%1").arg(offsetDAC[chanSubset[j]], 0, 'e', 3) << myCsvSeparator;
            } else {
                stream << QString("%1").arg(offsetDAC[j], 0, 'e', 3) << myCsvSeparator;
            }
        }
        stream << "\n";
//    }
    return ret;
}

void CalibrationConsumer::loadDefaultCalibParams(int channelsNum){
    for(int i = 0; i < vcCurrentRangesArray.size(); i++){
        for(int j = 0; j < channelsNum; j++){
            gainADC[i].push_back(defaultAdcGainValue.getNoPrefixValue());
            offsetADC[i].push_back(defaultAdcOffsetValue.getNoPrefixValue());
        }
    }

    /*! \todo COME PER CALIBRATEDACOFFSET, ANCHE QUI BISOGNEREBBE CICLARE SUL NUMERO DI RANGE DI TENSIONE*/
    for(int j = 0; j < channelsNum; j++){
        offsetDAC.push_back(defaultDacOffsetValue.getNoPrefixValue());
    }
}

void CalibrationConsumer::loadInitialCalibParams(QString path, QString mappingFileName){
    QStringList mappingStringList;
    QStringList boardStringList;
    vector<bool> calibratedWithDefaultParams;

    /*! all'inizio devo caricare i valori di calibrazione per tutti i canali (o dai file se li trovo o dai valori di defalut)*/
    channelToCalibIdxs.resize(currentChannelsNum);
    for(int i = 0; i< currentChannelsNum; i++){
        channelToCalibIdxs[i] = i;
    }

    if (!QDir().exists(path)) {
        QString msg = "Calibration directory " + path + " not found.\nDefault calibration parameters were loaded.";
        emit sigCalibLoadingMsg(msg);

    } else{
        calibrationFilesFolder = path;
        QFile boardMappingFile(path + mappingFileName);
        if(!boardMappingFile.exists()){
            QString msg = "Calibration mapping file " + mappingFileName + " not found.\nDefault calibration parameters were loaded.";
            emit sigCalibLoadingMsg(msg);
        } else {
            boardMappingFile.open(QFile::ReadOnly);
            if (boardMappingFile.isOpen()) {
                QTextStream mappingStream(&boardMappingFile);
                while(!mappingStream.atEnd()){
                    QString line = mappingStream.readLine();
                    mappingStringList.append(line.split(myCsvSeparator));
                    if(mappingStringList[0].toInt()<0 || mappingStringList[0].toInt()>=numOfBoards){
                        QString msg = "Wrong mapping in " + mappingFileName + " for board " + mappingStringList[1] +".\nCalibration is in an unstable state.\nRecheck the mapping file, push disconnect, close and restart, EMCR and repeat the calibration procedure.";
                        emit sigCalibLoadingMsg(msg);
                        boardMappingFile.close();
                        return;
                    }
                    boardSerialNums[mappingStringList[0].toInt()] = mappingStringList[1];
                    mappingStringList.clear();
                }
                boardMappingFile.close();

                for(int boardCalibFileIdx = 0; boardCalibFileIdx < boardSerialNums.size(); boardCalibFileIdx++){
                    QString boardCalibFileName = path + boardSerialNums[boardCalibFileIdx] + ".csv";
                    QFile boardCalibFile(boardCalibFileName);

                    if(!boardCalibFile.exists()){
                        /*! QUI DEVO CARICARE I VALORI DI DEFAULT PER QUESTA SCHEDA PERCHè IL FILE NON ESISTE*/
                        loadDefaultCalibParams(numOfChannelsOnBoard);
                        calibratedWithDefaultParams.push_back(true);
//                        QString msg = "Calibration file " + boardCalibFileName + " not found.\nDefault calibration parameters were loaded for board: " + QString("%1").arg(boardCalibFileIdx+1);
//                        emit sigCalibLoadingMsg(msg);
                    } else {
                        boardCalibFile.open(QFile::ReadOnly);
                        if (boardCalibFile.isOpen()) {
                            QTextStream boardStream(&boardCalibFile);
                            extractBoardCalibDataFromCsv(boardStream);
                            boardCalibFile.close();
                            calibratedWithDefaultParams.push_back(false);
                        } else {
                            /*! QUI DEVO CARICARE I VALORI DI DEFAULT PER QUESTA SCHEDA PERCHè IL FILE NON SI APRE*/
                            loadDefaultCalibParams(numOfChannelsOnBoard);
                            calibratedWithDefaultParams.push_back(true);
                            /*! manda ancora messaggio di erore per nn esser riuscito ad aprire questo file */
//                            QString msg = "Cannot open calibration file " + boardCalibFileName + ".\nDefault calibration parameters were loaded for board: " + QString("%1").arg(boardCalibFileIdx+1);
                        }
                    }
                }
                QString msg = "Calibration parameters loaded successfully.\n";

                for(int k = 0; k < calibratedWithDefaultParams.size(); k++){
                    if(calibratedWithDefaultParams[k]){
                        msg = msg + "Board " + QString("%1").arg(k+1) + " calibrated with default parameters\n";
                    }
                }
//                msg = msg + "calibrated with default parameters";

                allGainADC = gainADC;
                allOffsetADC = offsetADC;
                allOffsetDAC = offsetDAC;


                /*! Invio dati a FPGA con massageDispatcher*/
                updateCalibParams();
                emit sigCalibLoadingMsg(msg);
            } else {
                QString msg = "Cannot open calibration mapping file " + mappingFileName + " not found.\nDefault calibration parameters were loaded.";
                emit sigCalibLoadingMsg(msg);
            }
        }
    }

    /*! tolto per problemi 12/04/2023 */
    // channelToCalibIdxs.clear();
}

void CalibrationConsumer::extractBoardCalibDataFromCsv(QTextStream &boardStream){
    QString dump;
    QString line;
    QStringList tempList;
    vector<double> tempVector;

    // seriale della scheda, da buttare
    dump = boardStream.readLine();

    // leggo sui VC current range
    for(int lineIdx = 0; lineIdx< vcCurrentRangesArray.size(); lineIdx++){
        // valore VC current range, da buttare
        dump = boardStream.readLine();

        // linea con valori utili di ADC gain
        line = boardStream.readLine();
        tempList.append(line.split(myCsvSeparator));
        tempList.removeLast(); // remove the \n at the end of the line
        for(int paramIdx = 0; paramIdx < tempList.size(); paramIdx++){
            gainADC[lineIdx].push_back(tempList[paramIdx].toDouble());
        }
        tempList.clear();

        // linea con valori utili di ADC offset
        line = boardStream.readLine();
        tempList.append(line.split(myCsvSeparator));
        tempList.removeLast(); // remove the \n at the end of the line
        for(int paramIdx = 0; paramIdx < tempList.size(); paramIdx++){
            offsetADC[lineIdx].push_back(tempList[paramIdx].toDouble());
        }
        tempVector.clear();
        tempList.clear();
    }

    //leggo su VC Voltage range
    // valore VC voltage range, da buttare
    dump = boardStream.readLine();

    // linea con valori utili

    line = boardStream.readLine();
    tempList.append(line.split(myCsvSeparator));
    tempList.removeLast(); // remove the \n at the end of the line
    for(int paramIdx = 0; paramIdx < tempList.size(); paramIdx++){
        offsetDAC.push_back(tempList[paramIdx].toDouble());
    }

    tempVector.clear();
    tempList.clear();
}

/*! This conversion is needed to send the calibration parameters contained in gainADC, offsetADC anf offsetDAC to the FPGA via MessageDispatcher
gainADC, offsetADC anf offsetDAC contain only the parameters corresponding to channelToCalibIdxs (i.e. all the 384 channels or the 16 channels
belonging to the board under calibration) */
void CalibrationConsumer::convertToMeasurement(vector<vector<Measurement_t>> &gainAdcMeas, vector<vector<Measurement_t>> &offsetAdcMeas, vector<Measurement_t> &offsetDacMeas){
    /*! loop over ranges */
    for(int iii = 0; iii < vcCurrentRangesArray.size(); iii++){
        for(int jjj = 0; jjj < currentChannelsNum; jjj++){
            gainAdcMeas[iii].push_back({allGainADC[iii][jjj], UnitPfxNone, ""});
            offsetAdcMeas[iii].push_back({allOffsetADC[iii][jjj], UnitPfxNone, "A"});
        }
    }
    for(int jjj = 0; jjj < currentChannelsNum; jjj++){
        offsetDacMeas.push_back({allOffsetDAC[jjj], UnitPfxNone, "V"});
    }
}

/*! \todo FCON recheck insieme a controllermain che updata calibration params quando si cambia range. Al momento funzion a perchè dopo la calibrazione di startup, non channelToCalibIdxs è mai vuoto
Ricontrollare se ci sono problemi alla prima chiamata controllerMain in onVcCurrentRangeSelected
*/
void CalibrationConsumer::updateCalibParams(){
    /*! \todo INVIARE NUOVI DATI DI CALIBRAZIONE A fpga DOPO AVERLI CONVERTITIT IN MEASUREMENT PER TUTTI I CANALI*/
    if(channelToCalibIdxs.size()==0){
        return;
    } else {
        vector<vector<Measurement_t>> gainAdcMeas;
        vector<vector<Measurement_t>> offsetAdcMeas;
        vector<Measurement_t> offsetDacMeas;
        vector<uint16_t> channelIndexes;

        gainAdcMeas.resize(vcCurrentRangesArray.size());
        offsetAdcMeas.resize(vcCurrentRangesArray.size());
        offsetDacMeas.resize(vcVoltageRangesArray.size());
        convertToMeasurement(gainAdcMeas, offsetAdcMeas, offsetDacMeas);

        for(int i = 0; i< currentChannelsNum; i++){
            channelIndexes.push_back(i);
        }

        RangedMeasurement_t thisVcCurrentRange = mDev->getVcCurrentRange();
        uint16_t thisVcCurrentRangeIdx;
        for (int j = 0; j < vcCurrentRangesArray.size(); j++){
            if(thisVcCurrentRange.max==vcCurrentRangesArray[j].max){
                thisVcCurrentRangeIdx = j;
            }
        }

            mDev->getMessageDispatcher()->setCalibVcCurrentGain(channelIndexes, gainAdcMeas[thisVcCurrentRangeIdx], true);
            mDev->getMessageDispatcher()->setCalibVcCurrentOffset(channelIndexes, offsetAdcMeas[thisVcCurrentRangeIdx], true);
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
    vector<double> x; /*! voltage steps*/
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
        vector<Measurement_t> someVoltSteps;
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
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, samplesToremove);
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
    vector<double> y; /*! average currents*/
    y.resize(ccCalibrationVoltSteps[thisActualRangeIdx].size());
    vector<double> usefulCcAdcGain;
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
    vector<double> x; /*! current steps*/
    x.resize(ccCalibrationCurrSteps[thisActualRangeIdx].size());
    for(int i = 0; i< ccCalibrationCurrSteps[thisActualRangeIdx].size(); i++){
        x[i] = ccCalibrationCurrSteps[thisActualRangeIdx][i].getNoPrefixValue();
    }

    /*! Ho settato la maggior parte degli switch prima di chiamare questafunzione, qui abilito
     *  solo lo stimolo in corrente e lo spengo alla fine della funzione*/
    turnSomeCcStimulaOnOff(channelToCalibIdxs, someTrue);

    /*! FOR: START ciclo sugli step di corrente*/
    for(int currStepIdx = 0; currStepIdx <ccCalibrationCurrSteps[thisActualRangeIdx].size(); currStepIdx++){
        voltageMeans[currStepIdx].resize(channelToCalibIdxs.size());

        /*! setta la Chold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
        vector<Measurement_t> someCurrSteps;
        /*! \todo MPAC: non abbiamo una setChold nel modello, al momento non aggiorniamo il modello per questa cosa*/
//        for(int i = 0; i < channelToCalibIdxs.size(); i++){
//            someCurrSteps.push_back(ccCalibrationCurrSteps[thisActualRangeIdx][currStepIdx]);
//           mDev->getChannels()[channelToCalibIdxs[i]]->setVhold(ccCalibrationCurrSteps[thisActualRangeIdx][currStepIdx]);
//        }
        mDev->getMessageDispatcher()->setCurrentHoldTuner(channelToCalibIdxs, someCurrSteps, true);

        /*! prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * totalChannelsNum;//channelToCalibIdxs.size();  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        hook->flush(); /*! Remove old buffered data */
        while (!hook->getDataChunk(buffer, 1, minDataBatchSize));

        /*!butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, samplesToremove);
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
    vector<double> y; /*! average currents*/
    y.resize(ccCalibrationCurrSteps[thisActualRangeIdx].size());
    vector<double> usefulCcDacGain;
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
        usefulCcDacGain[chIdx] = usefulSlope/ccCalibratonResistances[thisActualRangeIdx].getNoPrefixValue();
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

}
