#ifndef PLOTMESSAGE_H
#define PLOTMESSAGE_H

#include <variant>
#include <vector>
#include <map>
#include "eventpacket.h"
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

struct EpisodicMessage{
    std::vector <double> timeValues;
    std::vector <std::vector <double>> voltageValues;
    std::vector <std::vector <double>> currentValues;
    double durationS;
    bool newProtocolFlag;
    bool newSweepFlag;
};

struct SpectrumMessage {
    double * frequencyValues;
    std::vector <double *> psdValues;
    std::vector <double *> irmsValues;
    int dataSize;
};

struct EventDetectionMessage {
    std::map <uint32_t, EventPacket> eventPackets;
};

// Forward declaration of the variant type
typedef std::variant<
    GapFreeMessage,
    EpisodicMessage,
    IvMessage,
    SpectrumMessage,
    EventDetectionMessage
> PlotMessage;

#endif // PLOTMESSAGE_H
