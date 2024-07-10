#ifndef PLOTMESSAGE_H
#define PLOTMESSAGE_H

#include <variant>
#include <vector>
#include <tuple>
#include <map>

#include "eventpacket.h"
#include "baseline.h"

#include <QVector>


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

struct EventDetectionMessage {
    std::map <uint32_t, EventPacket> eventPackets;
};

struct SpectrumMessage {
    double * frequencyValues;
    std::vector <double *> currentValues;
    int dataSize;
};

// Forward declaration of the variant type
typedef std::variant<
    GapFreeMessage,
    IvMessage,
    EventDetectionMessage,
    SpectrumMessage
> PlotMessage;

#endif // PLOTMESSAGE_H
