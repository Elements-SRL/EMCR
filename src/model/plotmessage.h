#ifndef PLOTMESSAGE_H
#define PLOTMESSAGE_H

#include <variant>
#include <vector>
#include <tuple>
#include <map>

#include "eventpacket.h"
#include "baseline.h"

#include <QVector>

#define PMS_GAPFREE 0
#define PMS_EPISODIC 1
#define PMS_IV 2
#define PMS_EVENT_DETECTION 3
#define PMS_SPECTRUM 4

struct IvMessage{
    std::vector <double *> voltageValues;
    std::vector <double *> currentValues;
    std::vector <int> dataSize;
};

struct GapFreeMessage{
    double * timeValues;
    std::vector <double *> voltageValues;
    std::vector <double *> currentValues;
    int dataSize;
};

struct EpisodicMessage{
    std::vector <double> timeValues;
    std::vector <std::vector <double>> voltageValues;
    std::vector <std::vector <double>> currentValues;
    bool newProtocolFlag;
    bool newSweepFlag;
};

struct EventDetectionMessage {
    std::map <uint32_t, EventPacket> eventPackets;
};

struct SpectrumMessage {
    double * frequencyValues;
    std::vector <double *> psdValues;
    std::vector <double *> irmsValues;
    int dataSize;
};

// Forward declaration of the variant type
typedef std::variant<
    GapFreeMessage,
    EpisodicMessage,
    IvMessage,
    EventDetectionMessage,
    SpectrumMessage
> PlotMessage;

#endif // PLOTMESSAGE_H
