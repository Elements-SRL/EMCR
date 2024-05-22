#ifndef PLOTMESSAGE_H
#define PLOTMESSAGE_H

#include <variant>
#include <vector>
#include <QVector>
#include <tuple>
#include "eventpacket.h"
#include <map>
#include "baseline.h"

struct IvMessage{
    std::vector<double*> voltageValues;
    std::vector<double*> currentValues;
    std::vector<int> dataSize;
};

struct GapFreeMessage{
    double * timeValues;
    std::vector<double *> voltageValues;
    std::vector<double *> currentValues;
    int dataSize;
};

struct EventDetectionMessage {
    std::map<uint32_t, EventPacket> eventPackets;
};

// Forward declaration of the variant type
typedef std::variant<
    GapFreeMessage,
    IvMessage,
    EventDetectionMessage
> PlotMessage;

#endif // PLOTMESSAGE_H
