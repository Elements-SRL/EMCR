#ifndef EVENTDETECTIONCHANNEL_H
#define EVENTDETECTIONCHANNEL_H

#include <vector>
#include <optional>
#include "event.h"
#include "firstorderiirfilter.h"

#define EVENT_TH 3 /*To be considered an event the signal must be EVENT_TH times the std dev*/
#define MIN_LEN 1
#define MAX_LEN 150
#define EVENT_PADDING 4

class EventDetector {
public:
    EventDetector(int sizeHint = -1);

    void pushEvent(Event event);
    std::vector<Event> getEvents();

    std::optional<std::pair<int, int>> analyze(double currentValue, uint32_t idx, uint32_t bufferSize);

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
