#ifndef EVENTDETECTIONCHANNEL_H
#define EVENTDETECTIONCHANNEL_H

#include <vector>
#include <optional>
#include "event.h"
#include "firstorderiirfilter.h"

constexpr uint32_t EVENT_TH = 3; /*To be considered an event the signal must be EVENT_TH times the std dev*/
constexpr uint32_t MIN_LEN = 80;
constexpr uint32_t MAX_LEN = 4000;
constexpr uint32_t EVENT_PADDING = 4;

class EventDetector {
public:
    EventDetector(int sizeHint = -1);

    void pushEvent(Event event);
    std::vector<Event> getEvents();

    std::optional<std::pair<int, int>> analyze(double currentValue, uint32_t idx, uint32_t bufferSize);
    void clear();
private:
    double threshold = -1;
    double calculateThreshold(const std::vector<double>& data);
    bool eventAlreadyBegun = false;
    uint32_t eventLen = 0;
    uint32_t eventBeginIdx = 0;

    FirstOrderIirFilter* high = nullptr;
    FirstOrderIirFilter* low = nullptr;

    //INITIALIZE THOSE  vvvv
    std::vector<double> bandPassFilterData;
    std::vector<Event> events;
    double calcStdDev(const std::vector<double>& data);
};


#endif // EVENTDETECTIONCHANNEL_H
