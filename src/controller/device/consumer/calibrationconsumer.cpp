#include "calibrationconsumer.h"
#include "messagedispatcher.h"

#include <QTime>
#include <QDebug>

CalibrationConsumer::CalibrationConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer){

    std::vector <Measurement_t> aaa;
    std::vector <RangedMeasurement_t> bbb;
    DeviceTypes_t ccc;

    mDev->getSamplingRatesFeatures(aaa);
    calibrationSamplingRate = aaa[0];

    mDev->getVcCurrentRangesFeatures(bbb);
    vcCurrentRangesArray = bbb;

    deviceUnderCalibrationType = mDev->getMessageDispatcher()->getDeviceType(mDev->getSerialNumber().toStdString(), ccc);

    if(ccc == Device384Nanopores || ccc == 2){ /*! \todo 2 means deviceFake in debug */
        mDev->getCalibVcVoltStepFeatures(calibrationVoltStep);
        mDev->getCalibVcResFeatures(calibratonResistances);
    } else {
        /*! \todo add settings for PatchClamp in case we use this same class  */
    }

    gainADC.resize(vcCurrentRangesArray.size());
    offsetADC.resize(vcCurrentRangesArray.size());

}

CalibrationConsumer::~CalibrationConsumer(){

}

void CalibrationConsumer::run(){
    consumptionStopped = false;
    exitedDataConsumingLoop = false;

    /*! \todo devo resizare e aggiornare qui, una volta che so quanti sono i canali da calibrare*/
    offsetDAC.resize(channelToCalibIdxs.size());
    totalChannelsUnderCalibNum = 2*channelToCalibIdxs.size();

    currentSum.resize(channelToCalibIdxs.size());
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

        /*! \todo  seleziona la più bassa sampling rate possibile*/
        vector <Measurement_t> samplingRates;
        mDev->getSamplingRatesFeatures(samplingRates);
        mDev->getMessageDispatcher()->setSamplingRate(0, true);
        mDev->setSamplingRate(samplingRates[0]);

        /*! \todo FOR: START ciclo sui range*/
        for(int jjj = 0; jjj <vcCurrentRangesArray.size(); jjj++){
            rangeIdx = jjj;

            /*! \todo setto il range di corrente per Voltage Clamp*/
            vector <RangedMeasurement_t> rangeInfo;
            mDev->getVcCurrentRangesFeatures(rangeInfo);
            mDev->getMessageDispatcher()->setVCCurrentRange(rangeIdx, true);
            multiplierCurrent = rangeInfo[rangeIdx].multiplier();

        /*! \todo  spegne lo stimolo e stacco il carico su tutti i canali */
            turnAllStimulaOnOff(false);
            turnAllChannelsOnOff(false);

             /*! \todo START CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            calibrateAdcGain();
            /*! \todo END CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

            /*! \todo a questo punto tutti i canali hanno carico staccato  e stimolo spento*/

            /*! \todo START CALCOLO ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    qui il carico deve essere staccato, staccato al  punto precedente, lo si può fare anche in maniera esplicita qui */
            calibrateAdcOffset();
            /*! \todo END CALCOLO ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

        /*! \todo FOR: END ciclo sui range*/
        }

        /*! \todo START CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
        calibrateDacOffset();
        /*! \todo END CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

        someFalse.clear();
        someTrue.clear();

        /*! \todo salvo queste info su CSV la cui struttura deve essere ancora decisa. Forse un file per ciascuna scheda*/
        prepareStuffToSaveOnCsv();

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
    /*! \todo Representation of voltage steps without any prefix, ...*/
    vector<double> x; /*! \todo  voltage steps*/
    x.resize(calibrationVoltStep.size());
    for(int i = 0; i< calibrationVoltStep.size(); i++){
        x[i] = calibrationVoltStep[i].getNoPrefixValue();
    }

    /*! \todo  seleziona tutti i canali  o quelli della scheda selezionata*/
//    selectSomeChannels(channelToCalibIdxs, someTrue);

    /*! \todo  attacca il carico  e accende lo stimolo su tutti i canali  o quelli della scheda selezionata*/
    turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    /*! \todo FOR: START ciclo sugli step di tensione*/
    for(int voltStepIdx = 0; voltStepIdx <calibrationVoltStep.size(); voltStepIdx++){
        currentMeans[voltStepIdx].resize(channelToCalibIdxs.size());

        /*! \todo  setta la Vhold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
        vector<Measurement_t> someVoltSteps;
        for(int i = 0; i < channelToCalibIdxs.size(); i++){
            someVoltSteps.push_back(calibrationVoltStep[voltStepIdx]);
           mDev->getChannels()[channelToCalibIdxs[i]]->setVhold(calibrationVoltStep[voltStepIdx]);
        }
        mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

        /*! \todo  prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * 2 * voltageChannelsNum;//channelToCalibIdxs.size();  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        hook->getDataChunk(buffer, 1, minDataBatchSize);

        /*! \todo  butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, samplesToremove);
        int actualBufferSize = buffer.size();
        buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
        actualBufferSize = buffer.size();

        int timeSamples = actualBufferSize/totalChannelsNum;

         /*! \todo  faccio media delle sole correnti */
        int channelIdx;
        for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
            for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
                if(channelToCalibIdxs.size()==voltageChannelsNum){
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

        buffer.clear(); /*! \todo is resized in getDataChunk()*/
        currentSum.clear();
        currentSum.resize(channelToCalibIdxs.size());
    }
    /*! \todo FOR: END ciclo sugli step di tensione*/

    /*! \todo UNA VOLTA CHE HO TUTTE CORRENTI MEDIE PER CIACUN Vstep PER CIASCN CANALE, FACCIO MINIMI QUADRATI */
    /*! \todo FOR: START ciclo sui canali*/

    vector<double> y; /*! \todo  average currents*/
    y.resize(calibrationVoltStep.size());
    vector<double> usefulAdcGain;
    usefulAdcGain.resize(channelToCalibIdxs.size());
    double usefulSlope;
    double uselessOffset;

    for(int chIdx = 0; chIdx < channelToCalibIdxs.size(); chIdx++){
        for(int i = 0; i< calibrationVoltStep.size(); i++){
            y[i] = currentMeans[i][chIdx];
        }
        /*! \todo calcolo slope con minimi quadrati che sarebbe 1/Rest*/
        leastSquareSimple(x, y, usefulSlope, uselessOffset);

        /*! \todo il gain sarebbe Rest/Rcalib, i.e. 1(Rcalib * slope)*/
        usefulAdcGain[chIdx] = 1/(usefulSlope * calibratonResistances[rangeIdx].getNoPrefixValue());
        y.clear();
        y.resize(calibrationVoltStep.size());
    }
    /*! \todo FOR: END ciclo sui canali*/
    gainADC[rangeIdx] = usefulAdcGain;

    for(int i = 0; i< calibrationVoltStep.size(); i++){
        currentMeans[i].clear();
    }

    /*! \todo  spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
//            selectSomeChannels(channelToCalibIdxs, someFalse);

}

void CalibrationConsumer::calibrateAdcOffset(){
    /*! \todo  applico  0V ai canali selezionati*/
    vector<Measurement_t> someVoltSteps;
    for(int i = 0; i < channelToCalibIdxs.size(); i++){
        someVoltSteps.push_back({0.0, UnitPfxMilli, "V"});
       mDev->getChannels()[channelToCalibIdxs[i]]->setVhold({0.0, UnitPfxMilli, "V"});
    }
    mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

    /*! \todo  seleziona tutti i canali  o quelli della scheda selezionata*/
//            selectSomeChannels(channelToCalibIdxs, someTrue);

    /*! \todo  accende lo stimolo su tutti i canali  o su quelli della scheda selezionata*/
    /*! \todo  gli switch di ingresso sono staccati dal passo precedente*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);

    currentMeans[0].resize(channelToCalibIdxs.size());

    /*! \todo  prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
    la cui struttura è ancora da definire */
    sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
    minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
    samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * 2 * voltageChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
    hook->getDataChunk(buffer, 1, minDataBatchSize);

    /*! \todo  butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
    buffer.remove(0, samplesToremove);
    int actualBufferSize = buffer.size();
    buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
    actualBufferSize = buffer.size();

    int timeSamples = actualBufferSize/totalChannelsNum;

     /*! \todo  faccio media delle sole correnti */
    int channelIdx;
    for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
        for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
            if(channelToCalibIdxs.size()==voltageChannelsNum){
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

    /*! \todo moltiplico la corrente media per i GAIN calacolati al passo precedente e dovrei avere già l'offset di ADC*/
    for(int i = 0; i < currentSum.size(); i++){
       usefulAdcOffset[i] = gainADC[rangeIdx][i] * currentSum[i]/((double)timeSamples);
    }

    buffer.clear(); /*! \todo is resized in getDataChunk()*/
    currentSum.clear();
    currentSum.resize(channelToCalibIdxs.size());

    offsetADC[rangeIdx] = usefulAdcOffset;

    /*! \todo  spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
//            selectSomeChannels(channelToCalibIdxs, someFalse);

}

void CalibrationConsumer::calibrateDacOffset(){
    int numTries = 0;
    vector<bool> needsFurtherCalibration;
    needsFurtherCalibration.resize(channelToCalibIdxs.size());

    vector<double> adcCompensatedCurrent;
    adcCompensatedCurrent.resize(channelToCalibIdxs.size());

    /*! \todo  applico  0V ai canali selezionati*/
    vector<Measurement_t> someVoltSteps;
    for(int i = 0; i < channelToCalibIdxs.size(); i++){
       someVoltSteps.push_back({0.0, UnitPfxMilli, "V"});
       mDev->getChannels()[channelToCalibIdxs[i]]->setVhold({0.0, UnitPfxMilli, "V"});
       needsFurtherCalibration[i] = true;
    }
    mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

    /*! \todo  seleziona tutti i canali  o quelli della scheda selezionata*/
//            selectSomeChannels(channelToCalibIdxs, someTrue);

    /*! \todo  accende lo stimolo e attacca gli switch di ingresso su tutti i canali  o su quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someTrue);
    turnSomeChannelsOnOff(channelToCalibIdxs, someTrue);

    currentMeans[0].resize(channelToCalibIdxs.size());

    vector<double> usefulDacOffset;
    usefulDacOffset.resize(channelToCalibIdxs.size());

    while(numTries <= CCS_DAC_OFFSET_MINIMIZATION_MAX_TRY){
        /*! \todo  prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
        la cui struttura è ancora da definire */
        sweepSamplingRateHz = mDev->getSamplingRate().getNoPrefixValue();
        minDataBatchSize = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_IN_S); /*! \todo proviamo  a mettere qui 1 intero secondo*/
        samplesToremove = qRound(sweepSamplingRateHz * CCS_CALIB_INTERVAL_TO_REMOVE_IN_S) * 2 * voltageChannelsNum;  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
        hook->getDataChunk(buffer, 1, minDataBatchSize);

        /*! \todo  butta via i primi e gli ultimi campioni corrispondenti  a 1/10 secondo*/
        buffer.remove(0, samplesToremove);
        int actualBufferSize = buffer.size();
        buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);
        actualBufferSize = buffer.size();

        int timeSamples = actualBufferSize/totalChannelsNum;

         /*! \todo  faccio media delle sole correnti */
        int channelIdx;
        for (int bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
            for (int currentIdx = 0; currentIdx < channelToCalibIdxs.size(); currentIdx++) {
                if(channelToCalibIdxs.size()==voltageChannelsNum){
                    channelIdx = bufferIdx+voltageChannelsNum+currentIdx;
                } else {
                    /*! necessario se calibro una sola scheda, mi serve come offset l'indice del primo canale che calibro*/
                    channelIdx = bufferIdx+voltageChannelsNum+currentIdx + channelToCalibIdxs[0];
                };
                currentSum[currentIdx] += buffer[channelIdx]*multiplierCurrent;
            }
        }

        /*! \todo moltiplico la corrente media per i GAIN ADC  e sottraggo offset ADC calacolati per tenere conto delle calibrazioni precedenti*/
        for(int i = 0; i < currentSum.size(); i++){
            adcCompensatedCurrent[i] = gainADC[rangeIdx][i] * currentSum[i]/((double)timeSamples) - offsetADC[rangeIdx][i];
            if (adcCompensatedCurrent[i] == 0.0){
               needsFurtherCalibration[i] = false;
           } else {
               /*! \todo sottraggo allo step di tensione attualmente applicato*/
                double poffi = calibratonResistances[rangeIdx].getNoPrefixValue(); // Ohm
                double bubbi = someVoltSteps[i].getNoPrefixValue() - adcCompensatedCurrent[i]/poffi; // A
                someVoltSteps[i].value = bubbi/someVoltSteps[i].multiplier(); //mV perchè divido V per 1e-3
           }
           offsetDAC[i] = someVoltSteps[i].value; //mV
        }

        /*! \todo mandi via messageDispatcher i valori aggiornati di voltage step per vedere se la lettura sui canali mi diventa finalmetne 0 */
        mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

        buffer.clear(); /*! \todo is resized in getDataChunk()*/
        currentSum.clear();
        currentSum.resize(channelToCalibIdxs.size());

        numTries++;
    }

    /*! \todo  spegne lo stimolo e stacca il carico su tutti i canali  o quelli della scheda selezionata*/
    turnSomeStimulaOnOff(channelToCalibIdxs, someFalse);
    turnSomeChannelsOnOff(channelToCalibIdxs, someFalse);
}


/*! \todo RECHECK: this can be used to pass specific params from the calibration GUI to the calibration thread, e.g. calibrate only one board
More functions will be needed, e.g. to load calibration from  a csv file
*/
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

void CalibrationConsumer::prepareStuffToSaveOnCsv(){


    QString path = "C:/schifo/";
    QTextStream stream;
    QFile outFile(path + "bah.csv");

    if (QDir().exists(path)) {
        outFile.open(QFile::WriteOnly);
        if (outFile.isOpen()) {
                stream.setDevice(&outFile);
                this->saveCsv(stream);
        }
        outFile.close();

    } else {
        if (QDir().mkpath(path)) {
            if (outFile.open(QFile::WriteOnly )) {
                stream.setDevice(&outFile);
                this->saveCsv(stream);
                outFile.close();
            }

        }
    }
}

void CalibrationConsumer::saveCsv(QTextStream &stream){
    stream << this->getCsvData();
}

QString CalibrationConsumer::getCsvData(){
    QString ret;
    QTextStream stream(&ret);
    QString myCsvSeparator = ",";
    stream << "Range" << myCsvSeparator << QString("%1").arg(vcCurrentRangesArray[0].max) << "\n";
    for(int i = 0; i<gainADC[0].size(); i++){
        stream << QString("%1").arg(gainADC[0][i], 0, 'f', 10) << myCsvSeparator;
    }
    stream << "\n";
//    stream << "Ciao:" << csvSeparator << QString("%1").arg(currentChannelIdx+1) << "\n";
    return ret;


}
