#include "calibrationconsumer.h"

#include <QTime>
#include <QDebug>

CalibrationConsumer::CalibrationConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer){

    std::vector <Measurement_t> aaa;
    std::vector <RangedMeasurement_t> bbb;
    DeviceTypes_t ccc;

    mDev->getSamplingRatesFeatures(aaa);
    this->calibrationSamplingRate = aaa[0];

    mDev->getVcCurrentRangesFeatures(bbb);
    this->vcCurrentRangesArray = bbb;

    deviceUnderCalibrationType = mDev->getMessageDispatcher()->getDeviceType(mDev->getSerialNumber().toStdString(), ccc);
    if(ccc == Device384Nanopores || ccc == 2){ /*! \todo 2 means deviceFake in debug */
        /*! \todo should be put in nanopore-specific md */
        calibrationVoltStep.resize(5);
        this->calibrationVoltStep[0] = {-400.0, UnitPfxMilli, "V"};
        this->calibrationVoltStep[1] = {-200.0, UnitPfxMilli, "V"};
        this->calibrationVoltStep[2] = {0.0, UnitPfxMilli, "V"};
        this->calibrationVoltStep[3] = {200.0, UnitPfxMilli, "V"};
        this->calibrationVoltStep[4] = {400.0, UnitPfxMilli, "V"};

        calibratonResistances.resize(this->vcCurrentRangesArray.size());

        /*! \todo questo è brutale, dobbiamo prendere le info dal md device specific. */
        this->calibratonResistances[0] = {120.0, UnitPfxKilo, "Ohm"}; // 4uA
        this->calibratonResistances[1] = {2.49, UnitPfxMega, "Ohm"}; // 200nA
    } else {
        /*! \todo add settings for PatchClamp in case we use this same class  */
    }

    this->gainADC.resize(this->vcCurrentRangesArray.size());
    this->offsetADC.resize(this->vcCurrentRangesArray.size());

}

CalibrationConsumer::~CalibrationConsumer(){

}

void CalibrationConsumer::run(){
    consumptionStopped = false;
    exitedDataConsumingLoop = false;

    int bufferIdx;
    int bufferLen = 0;
    int channelIdx;
    int counter;
    currentSum.resize(channelToCalibIdxs.size());
    currentMean.resize(channelToCalibIdxs.size());

    RangedMeasurement_t actualRange;
    Measurement_t actualCalibResist;

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();



    while(true){
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        /*! \todo FOR: START ciclo sui range*/
        for(int rangeIdx = 0; rangeIdx < this->vcCurrentRangesArray.size(); rangeIdx++){

        /*! \todo  seleziona tutti i canali */
            this->selectSelectAllChannels(true);

        /*! \todo  spegne tutti i canali*/
            this->turnAllChannelsOnOff(false);
            this->selectSelectAllChannels(false); // prima deseleziono tutto e poi seleziono i canali che mi interessa calibrare

        /*! \todo seleziono range più basso e fisso la resistenza di calibrazione nota in una variabile*/
            actualRange = vcCurrentRangesArray[rangeIdx];
            actualCalibResist = calibratonResistances[rangeIdx];

            /*! \todo START CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

                /*! \todo  seleziona la più bassa sampling rate possibile*/
            vector <Measurement_t> samplingRates;
            mDev->getSamplingRatesFeatures(samplingRates);
            this->mDev->getMessageDispatcher()->setSamplingRate(0, true);
            this->mDev->setSamplingRate(samplingRates[0]);


                /*! \todo  seleziona tutti i canali  o quelli della scheda selezionata*/
            vector<bool> someTrue;
            for(int i = 0; i < channelToCalibIdxs.size(); i++){
                someTrue.push_back(true);
            }
            this->selectSelectChannels(channelToCalibIdxs, someTrue);

                /*! \todo  accende tutti i canali  o quelli della scheda selezionata*/
            this->turnChannelsOnOff(channelToCalibIdxs, someTrue);

                /*! \todo FOR: START ciclo sugli step di tensione*/
            for(int voltStepIdx = 0; voltStepIdx < this->calibrationVoltStep.size(); voltStepIdx++){

                    /*! \todo  setta la Vhold per i canali selezionati e applica lo stimolo a tutti i canali selezionati*/
                vector<Measurement_t> someVoltSteps;
                for(int i = 0; i < channelToCalibIdxs.size(); i++){
                    someVoltSteps.push_back(this->calibrationVoltStep[voltStepIdx]);
                    this->mDev->getChannels()[channelToCalibIdxs[i]]->setVhold(this->calibrationVoltStep[voltStepIdx]);
                }
                this->mDev->getMessageDispatcher()->setVoltageHoldTuner(channelToCalibIdxs, someVoltSteps, true);

                    /*! \todo  prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
                    la cui struttura è ancora da definire, e.g. ogni riga contiene un istante temporale e ogni colonna rappresenta un canale */

                this->sweepSamplingRateHz = this->mDev->getSamplingRate().getNoPrefixValue();
                minDataBatchSize = qRound(sweepSamplingRateHz*1); /*! \todo proviamo  a mettere qui 1 intero secondo*/
                samplesToremove = qRound(sweepSamplingRateHz*0.1) * 2 * channelToCalibIdxs.size();  /*! \todo proviamo  a mettere qui 1/10 di secondo*/
                hook->getDataChunk(buffer, 1, minDataBatchSize);

                /*! \todo  butta via i primi e gli ultimi 1000 campioni, i.e. righe*/
                buffer.remove(0, samplesToremove);
                int actualBufferSize = buffer.size();
                buffer.remove(actualBufferSize-1-samplesToremove, samplesToremove);


                timeSamples = buffer.size()/totalChannelsNum;

                    /*! \todo  faccio media per colonne delle sole correnti */
                for (bufferIdx = 0; bufferIdx < buffer.size(); bufferIdx += totalChannelsNum) {
                    for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
                        channelIdx = bufferIdx+voltageChannelsNum+currentIdx;
                        currentSum[currentIdx] += buffer[channelIdx];
                    }
                }

                for(int i = 0; i < currentSum.size(); i++){
                    currentMean[i] = currentSum[i]/((double)timeSamples);
                }



                    /*! \todo FOR: START ciclo sui canali*/

                        /*! \todo calcolo slope con minimi quadrati che sarebbe la resistenza stimata*/

                        /*! \todo salvo il divido la resistenza stimata per la mia di test e ottengo il gain per lo specifico canale e per lo specifico Vtest da qualche parte, e.g. altra matriciona o struttura dati
                            oppure manda i 384 gain a FPGA*/

                    /*! \todo FOR: END ciclo sui canali*/

                /*! \todo FOR: END ciclo sugli step di tensione*/

                /*! \todo spengo e deselezionio i canali e settola Vhold a 0V */


            /*! \todo END CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            }

            /*! \todo START CALCOLO ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
                /*! \todo  seleziona tutti i canali  o quelli della scheda selezionata*/

                /*! \todo  accende tutti i canali  o quelli della scheda selezionata*/

                /*! \todo  applico gli 0V precedentemente selezionati  ai canali selezionati*/

                /*! \todo   apro tutti gli switch per i canali selezionati (input_SW per EL06  e CAL_SW + VC_SWper EL07)*/

                /*! \todo  prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
                la cui struttura è ancora da definire, e.g. ogni riga contiene un istante temporale e ogni colonna rappresenta un canale */

                /*! \todo  butta via i primi e gli ultimi 1000 campioni, i.e. righe*/

                /*! \todo  faccio media per colonne delle sole correnti e le moltiplico per i) il gain calcolato al punto precedente SE NON HAI MANDATO I GAIN ALLE FPGA*/

                /*! \todo FOR: START ciclo sui canali*/

                    /*! \todo calcolo offset con minimi quadrati e la divido per la resistenza nota*/

                    /*! \todo cambio segno  e salvo l'offset per lo specifico canale e per lo specifico Vtest da qualche parte, e.g. altra matriciona o struttura dati
                        E EVENTUALMETNE MANDA A FPGA*/

                /*! \todo END: START ciclo sui canali*/


            /*! \todo END CALCOLO ADC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

        /*! \todo FOR: END ciclo sui range*/
        }

            /*! \todo START CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

                /*! \todo chiudo tutti gli switch per i canali selezionati*/

                /*! \todo QUI LE CORREZIONI DI ADC GAIN E OFFSET DEVONO ESSERE APPLICATE O DA SW O DA FPGA, MA CI DEVONO ESSERE!!!!!!!!!!!!!!!!*/

                /*! \todo applico 0V ai canali selezionati*/

                /*! \todo FOR: START ciclo sui canali*/

                    /*! \todo controllo se in media leggo 0A come sopra*/

                    /*! \todo IF YES leggo 0A*/
                        /*! \todo FINE*/
                    /*! \todo ELSE*/
                        /*! in while corrente letta IS NOT in un certo range intorno allo 0V, continuo ad abbassre lo stimolo di tensione*/
                    /*! \todo END IF*/

                    /*! \todo salvo il valore di tensione di test (i.e. offset DAC) per lo specifico canale da qualche parte, e.g. altra matriciona o struttura dati*/

                /*! \todo END: START ciclo sui canali*/


            /*! \todo END CALCOLO DAC OFFSET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

        /*! \todo salvo queste info su CSV la cui struttura deve essere ancora decisa. Forse un file per ciascuna scheda*/






    }

    consumptionLock.relock();

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

void CalibrationConsumer::selectSelectAllChannels(bool selectValue){
    uint16_t numOfChannelsToUpadate = this->mDev->getChannels().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        this->mDev->getChannels()[i]->setSelected(selectValue);
    }
}

void CalibrationConsumer::turnAllChannelsOnOff(bool onValue){
    uint16_t numOfChannelsToUpadate = this->mDev->getChannels().size();
    vector<uint16_t> channelIndexes;
    vector<bool> onValues;
    channelIndexes.resize(numOfChannelsToUpadate);
    onValues.resize(numOfChannelsToUpadate);
    for (int i = 0; i < numOfChannelsToUpadate; i++){
        this->mDev->getChannels()[i]->setOn(onValue);
        channelIndexes[i] = i;
        onValues[i] = onValue;
        qDebug() << "[Channel " << i << "]: on/off status:" << onValue << "\n";
    }
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
}

void CalibrationConsumer::selectSelectChannels(vector<uint16_t> channelIndexes, vector<bool> selectValues){
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setSelected(selectValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: selected status:" << selectValues[i] << "\n";
    }
}

void CalibrationConsumer::turnChannelsOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setOn(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
    }
}
