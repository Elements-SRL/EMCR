#include "calibrationconsumer.h"

#include <QTime>

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

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    while(true){
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        /*! \todo FOR: START ciclo sui range*/

        /*! \todo  seleziona tutti i canali */

        /*! \todo  spegne tutti i canali*/

        /*! \todo seleziono range più basso e fisso la resistenza di calibrazione nota in una variabile*/

            /*! \todo START CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

                /*! \todo  seleziona la più bassa sampling rate possibile*/

                /*! \todo  seleziona tutti i canali  o quelli della scheda selezionata*/

                /*! \todo  accende tutti i canali  o quelli della scheda selezionata*/

                /*! \todo FOR: START ciclo sugli step di tensione*/

                    /*! \todo  setta la Vhold per i canali selezionati*/

                    /*! \todo  applica lo stimolo a tutti i canali selezionati*/

                    /*! \todo  prende dati per 1s, basandosi sulla sampling rate di calibrazione, i.e. la più bassa. E.g. almeno 7500 o 5000 campioni, verrà fuori una matrice dove
                    la cui struttura è ancora da definire, e.g. ogni riga contiene un istante temporale e ogni colonna rappresenta un canale */

                    /*! \todo  butta via i primi e gli ultimi 1000 campioni, i.e. righe*/

                    /*! \todo  faccio media per colonne delle sole correnti */

                    /*! \todo FOR: START ciclo sui canali*/

                        /*! \todo calcolo slope con minimi quadrati che sarebbe la resistenza stimata*/

                        /*! \todo salvo il divido la resistenza stimata per la mia di test e ottengo il gain per lo specifico canale e per lo specifico Vtest da qualche parte, e.g. altra matriciona o struttura dati
                            oppure manda i 384 gain a FPGA*/

                    /*! \todo FOR: END ciclo sui canali*/

                /*! \todo FOR: END ciclo sugli step di tensione*/

                /*! \todo spengo i canali e settola Vhold a 0V */


            /*! \todo END CALCOLO ADC GAIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


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
