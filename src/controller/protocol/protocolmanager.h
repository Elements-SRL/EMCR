#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>

#include "protocolwidget.h"
#include "devicedataconsumer.h"

class ProtocolManager : public QObject {
    Q_OBJECT

public:
    typedef enum {
        Success,
        ErrorNotEnoughItemsForSequence,
        ErrorOverlappingSequences,
        ErrorMidInfiniteSequence,
        ErrorItemsOverflow,
        ErrorItemsNotFound,
        ErrorItemsOverStimulus,
        ErrorItemsUnderStimulus,
        ErrorItemsUnderDuration,
        ErrorItemsNotProcessed,
        ErrorProtocolInhibited
    } ProtocolApplicationStatus_t;

    ProtocolManager(ModelDevice *  mDev);

    ProtocolApplicationStatus_t startProtocol(ProtocolWidget * protocol, bool recordFlag = false);
    void saveLast(ProtocolWidget * protocol);
    ProtocolType_t getLastStartedType();
    void setSecondaryDevice(bool flag);

public slots:
    void onIncreaseProtocolId();
    void onProtocolEnded();

private:
    ProtocolApplicationStatus_t toProtocolApplicationStatus(ItemsProcStatus_t status);

    ModelDevice *  mDev;

    unsigned short protocolId = 0;
    unsigned short lastRunProtocolId = 0;
    ProtocolType_t lastStartedType = ProtocolTypeGapfree;

    ProtocolWidget * protocol;
    bool recordFlag;
    bool secondaryDeviceFlag = false;

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

    Measurement_t xAmp = {
        0.0, // value
        UnitPfxNone, // prefix
        "V" // unit
    };

    Measurement_t t0 = {
        0.0, // value
        UnitPfxMilli, // prefix
        "s" // unit
    };

    Measurement_t tStep = {
        0.0, // value
        UnitPfxMilli, // prefix
        "s" // unit
    };

    Measurement_t freq = {
        0.0, // value
        UnitPfxKilo, // prefix
        "Hz" // unit
    };

signals:
    void protocolStarted(unsigned int, ProtocolWidget *);
    void protocolSaveRequest(unsigned int, ProtocolWidget *);
    void currentApplied();
};

#endif // PROTOCOLMANAGER_H
