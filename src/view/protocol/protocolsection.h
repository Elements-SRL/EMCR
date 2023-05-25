#ifndef PROTOCOLSECTION_H
#define PROTOCOLSECTION_H

#include <QVector>

class ProtocolSections;

class ProtocolSection {
public:
    ProtocolSection();

    double duration();
    double minDuration();
    void setParent(ProtocolSections * protocolSections);
    ProtocolSection * getSectionByItem(int itemIdx, int repsIdx, int sweepIdx);

    int itemIdx;
    int repsIdx;
    int repsNum;
    int loopIdx;
    int sweepIdx;
    int sweepsNum;
    bool applySteps;
    double startingTime;
    double endingTime;

private:
    ProtocolSections * parent;
};

class ProtocolSections : public QVector <ProtocolSection *> {
public:
    ProtocolSections();
    ~ProtocolSections();

    void appendSection(ProtocolSection * protocolSection);
    ProtocolSection * getSectionByItem(int itemIdx, int repsIdx, int sweepIdx);
};

#endif // PROTOCOLSECTION_H
