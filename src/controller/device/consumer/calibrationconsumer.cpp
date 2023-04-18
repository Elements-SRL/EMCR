#include "calibrationconsumer.h"
#include "messagedispatcher.h"
#include "errormanager.h"

#include <QTime>
#include <QDebug>

CalibrationConsumer::CalibrationConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer){

    std::vector <Measurement_t> aaa;
    DeviceTypes_t ccc;

    mDev->getSamplingRatesFeatures(aaa);
    calibrationSamplingRate = aaa[0];

    mDev->getVcCurrentRangesFeatures(vcCurrentRangesArray, defaultVcCurrRangeIdx);
    mDev->getVcVoltageRangesFeatures(vcVoltageRangesArray);

    deviceUnderCalibrationType = mDev->getMessageDispatcher()->getDeviceType(mDev->getSerialNumber().toStdString(), ccc);
    mDev->getBoardsNumberFeatures(numOfBoards);
    numOfChannelsOnBoard = currentChannelsNum/numOfBoards;

    mDev->getMessageDispatcher()->getCalibDefaultVcAdcGain(defaultAdcGainValue);    //1.57014;
    mDev->getMessageDispatcher()->getCalibDefaultVcAdcOffset(defaultAdcOffsetValue); // 0.0;
    mDev->getMessageDispatcher()->getCalibDefaultVcDacOffset(defaultDacOffsetValue); // 0.0;

    if(ccc == Device384Nanopores || ccc == Device384Fake){
        mDev->getCalibVcVoltStepFeatures(calibrationVoltStep);
        mDev->getCalibVcResFeatures(calibratonResistances);
    } else {
        /*! \todo add settings for PatchClamp in case we use this same class  */
    }

    gainADC.resize(vcCurrentRangesArray.size());
    offsetADC.resize(vcCurrentRangesArray.size());

    allGainADC.resize(vcCurrentRangesArray.size());
    allOffsetADC.resize(vcCurrentRangesArray.size());

    for(int i = 0; i< vcCurrentRangesArray.size(); i++){
        allGainADC[i].resize(currentChannelsNum);
        allOffsetADC[i].resize(currentChannelsNum);
    }
    allOffsetDAC.resize(currentChannelsNum);

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

    /*! devo resizare e aggiornare qui, una volta che so quanti sono i canali da calibrare*/
    offsetDAC.resize(channelToCalibIdxs.size());
    totalChannelsUnderCalibNum = 2*channelToCalibIdxs.size();

    currentSum.resize(channelToCalibIdxs.size());
    currentSum.fill(0.0);
    currentMeans.resize(calibrationVoltStep.size());


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

        uint16_t bbb; //useless

        /*! spegne lo stimolo e stacco il carico su tutti i canali per concentire all'utente di cambiare la model cell se c'è quella sbagliata*/
        turnAllStimulaOnOff(false);
        turnAllChannelsOnOff(false);

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

             /*! START CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            calibrateAdcGain();
            /*! END CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

            /*! a questo punto tutti i canali hanno carico staccato  e stimolo spento*/

            /*! START CALCOLO ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    qui il carico deve essere staccato, staccato al  punto precedente, lo si può fare anche in maniera esplicita qui */
            calibrateAdcOffset();
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
        calibrateDacOffset();
        /*! END CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

        someFalse.clear();
        someTrue.clear();

        /*! salvo queste info su CSV la cui struttura deve essere ancora decisa. Forse un file per ciascuna scheda*/
        mainSaveOnCsv();

        /*! AGGIORNO LE STRUTTURA IN CUI TENGO TUTTI I VALORI DI CALIBRAZIONE, allGainADC, allOffsetADc, allOffsetDAC*/
        for(int zzz = 0; zzz < vcCurrentRangesArray.size(); zzz++){
            for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
                allGainADC[zzz][channelToCalibIdxs[xxx]] = gainADC[zzz][xxx];
                allOffsetADC[zzz][channelToCalibIdxs[xxx]] = offsetADC[zzz][xxx];
            }
        }
        for(int xxx = 0; xxx < channelToCalibIdxs.size(); xxx++){
            allOffsetDAC[channelToCalibIdxs[xxx]] = offsetDAC[xxx];
        }

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

void CalibrationConsumer::calibrateAdcGain(){
    /*! Representation of voltage steps without any prefix, ...*/
    vector<double> x; /*! voltage steps*/
    x.resize(calibrationVoltStep.size());
    for(int i = 0; i< calibrationVoltStep.size(); i++){
        x[i] = calibrationVoltStep[i].getNoPrefixValue();
    }

    /*! attacca il carico  e accende lo stimolo su tutti i canali  o quelli della scheda selezionata*/
    turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    /*! FOR: START ciclo sugli step di tensione*/
    for(int voltStepIdx = 0; voltStepIdx <calibrationVoltStep.size(); voltStepIdx++){
        currentMeans[voltStepIdx].resize(channelToCalibIdxs.size());

        /*! setta la Vhold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
        vector<Measurement_t> someVoltSteps;
        for(int i = 0; i < channelToCalibIdxs.size(); i++){
            someVoltSteps.push_back(calibrationVoltStep[voltStepIdx]);
           mDev->getChannels()[channelToCalibIdxs[i]]->setVhold(calibrationVoltStep[voltStepIdx]);
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
    y.resize(calibrationVoltStep.size());
    vector<double> usefulAdcGain;
    usefulAdcGain.resize(channelToCalibIdxs.size());
    double usefulSlope;
    double uselessOffset;

    for(int chIdx = 0; chIdx < channelToCalibIdxs.size(); chIdx++){
        for(int i = 0; i< calibrationVoltStep.size(); i++){
            y[i] = currentMeans[i][chIdx];
        }
        /*! calcolo slope con minimi quadrati che sarebbe 1/Rest*/
        leastSquareSimple(x, y, usefulSlope, uselessOffset);

        /*! il gain sarebbe Rest/Rcalib, i.e. 1(Rcalib * slope)*/
        usefulAdcGain[chIdx] = 1/(usefulSlope * calibratonResistances[rangeIdx].getNoPrefixValue());
        y.clear();
        y.resize(calibrationVoltStep.size());
    }
    /*! FOR: END ciclo sui canali*/
    gainADC[rangeIdx] = usefulAdcGain;

    for(int i = 0; i< calibrationVoltStep.size(); i++){
        currentMeans[i].clear();
    }

    /*! spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
}

void CalibrationConsumer::calibrateAdcOffset(){
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
       usefulAdcOffset[i] = gainADC[rangeIdx][i] * currentSum[i]/((double)timeSamples); /*! \todo FCON la moltiplicazione per il gain non simula correttamente quello che accade in FPGA, meglio far fare il conto all'FPGA usando il comando del MessageDispatcher
                                                                                                       vale anche per il calcolo dell'offset del DAC */
    }

    buffer.clear(); /*! is resized in getDataChunk()*/
    currentSum.clear();
    currentSum.resize(channelToCalibIdxs.size());
    currentSum.fill(0.0);

    offsetADC[rangeIdx] = usefulAdcOffset;

    /*! spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
}

void CalibrationConsumer::calibrateDacOffset(){
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
    turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);

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
            adcCompensatedCurrent[i] = gainADC[rangeIdx][i] * currentSum[i]/((double)timeSamples) - offsetADC[rangeIdx][i]; /*! \todo FCON vedi commento nel calcolo dell'offset dell'ADC: far fare la calibrazione parziale in FPGA invece che in SW */
            if (adcCompensatedCurrent[i] == 0.0){
               needsFurtherCalibration[i] = false;
           } else {
               /*! sottraggo allo step di tensione attualmente applicato*/
                double poffi = calibratonResistances[rangeIdx].getNoPrefixValue(); // Ohm
                double bubbi = someVoltSteps[i].getNoPrefixValue() - adcCompensatedCurrent[i]/poffi; // A
                someVoltSteps[i].value = bubbi/someVoltSteps[i].multiplier(); //mV perchè divido V per 1e-3
           }
           offsetDAC[i] = someVoltSteps[i].getNoPrefixValue(); //V
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
    turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
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

void CalibrationConsumer::selectSomeChannels(vector<uint16_t> channelIndexes, vector<bool> selectValues){
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setSelected(selectValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: selected status:" << selectValues[i] << "\n";
    }
}

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
        QString msg = "All boards manual calibration successfull!";
        emit sigManualCalibDoneMsg(msg);
    } else {
        /*! calibro solo una board*/
        fileName = boardSerialNums[channelToCalibIdxs[0]/numOfChannelsOnBoard] + QString(".csv");
        prepareStuffToSaveOnCsv(calibrationFilesFolder, fileName, channelToCalibIdxs);
        QString msg = "Board " + QString("%1").arg(1+channelToCalibIdxs[0]/numOfChannelsOnBoard) +" manual calibration successfull!";
        emit sigManualCalibDoneMsg(msg);

    }

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
                stream << QString("%1").arg(gainADC[i][chanSubset[j]], 0, 'f', 10) << myCsvSeparator;
            } else {
                /*! One board channels calibration*/
                stream << QString("%1").arg(gainADC[i][j], 0, 'f', 10) << myCsvSeparator;
            }
        }
        stream << "\n";
        for(int j = 0; j < chanSubset.size(); j++){
            if(offsetADC[i].size()==currentChannelsNum){
                /*! All channels calibration*/
                stream << QString("%1").arg(offsetADC[i][chanSubset[j]], 0, 'f', 10) << myCsvSeparator;
            } else {
                /*! One board channels calibration*/
                stream << QString("%1").arg(offsetADC[i][j], 0, 'f', 10) << myCsvSeparator;
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
                stream << QString("%1").arg(offsetDAC[chanSubset[j]], 0, 'f', 30) << myCsvSeparator;
            } else {
                stream << QString("%1").arg(offsetDAC[j], 0, 'f', 30) << myCsvSeparator;
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
