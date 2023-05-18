#include "protocolsection.h"

ProtocolSection::ProtocolSection() {

}

double ProtocolSection::duration() {
    return endingTime-startingTime;
}

double ProtocolSection::minDuration() {
    ProtocolSection * firstSection;
    ProtocolSection * lastSection;
    if (sweepsNum > 1) {
        firstSection = this->getSectionByItem(itemIdx, repsIdx, 0);
        lastSection = this->getSectionByItem(itemIdx, repsIdx, sweepsNum-1);

    } else {
        firstSection = this->getSectionByItem(itemIdx, 0, 0);
        if (repsNum > 0) {
            lastSection = this->getSectionByItem(itemIdx, repsNum-1, 0);

        } else {
            lastSection = this->getSectionByItem(itemIdx, 0, 0);
        }
    }
    return qMin(firstSection->duration(), lastSection->duration());
}

void ProtocolSection::setParent(ProtocolSections * protocolSections) {
    parent = protocolSections;
}

ProtocolSection * ProtocolSection::getSectionByItem(int itemIdx, int repsIdx, int sweepIdx) {
    return parent->getSectionByItem(itemIdx, repsIdx, sweepIdx);
}

ProtocolSections::ProtocolSections() {

}

ProtocolSections::~ProtocolSections() {
    for (int idx = 0; idx < this->size(); idx++) {
        delete this->at(idx);
    }
    this->clear();
    this->squeeze();
}

void ProtocolSections::appendSection(ProtocolSection * protocolSection) {
    protocolSection->setParent(this);
    this->append(protocolSection);
}

ProtocolSection * ProtocolSections::getSectionByItem(int itemIdx, int repsIdx, int sweepIdx) {
    int sectionIdx = 0;
    if (itemIdx >= 0) {
        /*! If itemIdx is not negative just search for the required tuple */
        while (sectionIdx < this->size()) {
            if ((this->at(sectionIdx)->itemIdx == itemIdx) &&
                    (this->at(sectionIdx)->repsIdx == repsIdx) &&
                    (this->at(sectionIdx)->sweepIdx == sweepIdx)) {
                return this->at(sectionIdx);
            }
            sectionIdx++;
        }

    } else {
        /*! If itemIdx is negative look for the last section of the required sweep, so... */
        while (sectionIdx < this->size()) {
            /*! Either look for the first item of the following sweep, ... */
            if ((this->at(sectionIdx)->itemIdx == 0) &&
                    (this->at(sectionIdx)->repsIdx == 0) &&
                    (this->at(sectionIdx)->sweepIdx == sweepIdx)) {

                if (sectionIdx > 0) {
                    return this->at(sectionIdx-1);

                } else {
                    return nullptr;
                }
            }
            sectionIdx++;
        }

        sectionIdx--;
        /*! or if you reach the end check if the last section matches the sweep you're looking for */
        if (this->at(sectionIdx)->sweepIdx == sweepIdx-1) {
            return this->at(sectionIdx);
        }
    }
    return nullptr;
}
