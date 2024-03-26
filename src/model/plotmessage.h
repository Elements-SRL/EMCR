#ifndef PLOTMESSAGE_H
#define PLOTMESSAGE_H

#include <variant>
#include <vector>
#include <QVector>
#include <tuple>

struct IvMessage{
    std::vector<double> voltageValues;
    std::vector<double*> currentValues;
    int dataSize;
};

struct GapFreeMessage{
    double * timeValues;
    std::vector<double *> * voltageValues;
    std::vector<double *> * currentValues;
    int dataSize;
};

// Forward declaration of the variant type
typedef std::variant<
    IvMessage,
    GapFreeMessage
> PlotMessage;

#endif // PLOTMESSAGE_H
