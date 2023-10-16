#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>

#include "protocolwidget.h"
#include "devicedataconsumer.h"
#include "protocoldefs.h"
#include "messagedispatcher.h"

class ProtocolManager : public QObject {
    Q_OBJECT

public:
    ProtocolManager(MessageDispatcher * msgDisp);

public slots:
    void onStartProtocolRequest(ProtocolWidget * protocol);
    void onIncreaseProtocolId();

private:
    ProtocolApplicationStatus_t startProtocol(ProtocolWidget * protocol, bool recordFlag = false);
    ProtocolApplicationStatus_t toProtocolApplicationStatus(ItemsProcStatus_t status);

    MessageDispatcher * msgDisp = nullptr;

    unsigned short protocolId = 0;
    unsigned short lastRunProtocolId = 0;

    ProtocolWidget * protocol;
    bool recordFlag;

    /*! \todo FCON questa cosa non mi piace per niente, se si cambia la struttura Measurement_t, anche solo l'ordine degli elementi qui si sballa tutto.
                   Cercare di gestire meglio tutta questa cosa (magari si riesce a fare qualcosa di più pulito anche per la conversione in Measurement_t) */
    /*! \todo FCON non mi piace molto nemmeno il fatto che si danno per scontate le unità di misura. Meglio pensare ad un modo per rendere i valor di
                   default del software disponibili ovunque */

    Measurement_t x0 = {
        0.0, // value
        UnitPfxNone, // prefix
        "V" // unit
    };

    Measurement_t xStep = {
        0.0, // value
        UnitPfxNone, // prefix
        "V" // unit
    };

    Measurement_t xFinal = {
        0.0, // value
        UnitPfxNone, // prefix
        "V" // unit
    };

    Measurement_t xFinalStep = {
        0.0, // value
        UnitPfxNone, // prefix
        "V" // unit
    };

    Measurement_t xAmp = {
        0.0, // value
        UnitPfxNone, // prefix
        "V" // unit
    };

    Measurement_t xAmpStep = {
        0.0, // value
        UnitPfxNone, // prefix
        "V" // unit
    };

    Measurement_t t0 = {
        0.0, // value
        UnitPfxMilli, // prefix
        "s" // unit
    };

    Measurement_t t0Step = {
        0.0, // value
        UnitPfxMilli, // prefix
        "s" // unit
    };

    Measurement_t f0 = {
        0.0, // value
        UnitPfxNone, // prefix
        "Hz" // unit
    };

    Measurement_t f0Step = {
        0.0, // value
        UnitPfxNone, // prefix
        "Hz" // unit
    };

    bool stimHalfFlag = false;

signals:
    void protocolStarted(unsigned int, ProtocolWidget *);
    void currentApplied();
    void protocolRequestOutcome(ProtocolApplicationStatus_t status);
};

#endif // PROTOCOLMANAGER_H
