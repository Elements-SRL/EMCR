#include "analysiscursor.h"

AnalysisCursor::AnalysisCursor(AnalysisCursorType_t type) :
    type(type) {

}

AnalysisCursor::AnalysisCursor(AnalysisCursor * cursor) :
    type(cursor->type),
    itemIdx(cursor->itemIdx),
    repsIdx(cursor->repsIdx),
    sweepIdx(cursor->sweepIdx) {

    this->setDelays(cursor->getDelays());
}

AnalysisCursor::~AnalysisCursor() {
    sampleDelays.clear();
    delays.clear();
    sampleDelays.squeeze();
    delays.squeeze();
}

void AnalysisCursor::setDelays(QVector <double> delays) {
    this->delays = delays;
    sampleDelays.resize(delays.size());
}

QVector <double> AnalysisCursor::getDelays() {
    return delays;
}

void AnalysisCursor::setSamplingRate(double samplingRate) {
    for (int delayIdx = 0; delayIdx < delays.size(); delayIdx++) {
        sampleDelays[delayIdx] = qRound(samplingRate*delays[delayIdx]);
    }
}

AnalysisGapFreeCursor::AnalysisGapFreeCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisGapFreeCursor::AnalysisGapFreeCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisGapFreeCursor::checkTuple(int itemIdx, int repsIdx, int) {
    return ((this->itemIdx == itemIdx) &&
            (this->repsIdx == repsIdx));
}

int AnalysisGapFreeCursor::getSampleDelay(int, int) {
    return sampleDelays[0];
}

AnalysisGapFreeAllRepsCursor::AnalysisGapFreeAllRepsCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisGapFreeAllRepsCursor::AnalysisGapFreeAllRepsCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisGapFreeAllRepsCursor::checkTuple(int itemIdx, int, int) {
    return (this->itemIdx == itemIdx);
}

int AnalysisGapFreeAllRepsCursor::getSampleDelay(int repsIdx, int) {
    return sampleDelays[repsIdx];
}

AnalysisGapFreeInfRepsCursor::AnalysisGapFreeInfRepsCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisGapFreeInfRepsCursor::AnalysisGapFreeInfRepsCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisGapFreeInfRepsCursor::checkTuple(int itemIdx, int repsIdx, int) {
    return ((this->itemIdx == itemIdx) &&
            (this->repsIdx == repsIdx));
}

int AnalysisGapFreeInfRepsCursor::getSampleDelay(int, int) {
    return sampleDelays[0];
}

AnalysisGapFreeInfAllRepsCursor::AnalysisGapFreeInfAllRepsCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisGapFreeInfAllRepsCursor::AnalysisGapFreeInfAllRepsCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisGapFreeInfAllRepsCursor::checkTuple(int itemIdx, int, int) {
    return (this->itemIdx == itemIdx);
}

int AnalysisGapFreeInfAllRepsCursor::getSampleDelay(int, int) {
    return sampleDelays[0];
}

AnalysisEpisodicCursor::AnalysisEpisodicCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisEpisodicCursor::AnalysisEpisodicCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisEpisodicCursor::checkTuple(int itemIdx, int repsIdx, int sweepIdx) {
    return ((this->itemIdx == itemIdx) &&
            (this->repsIdx == repsIdx) &&
            (this->sweepIdx == sweepIdx));
}

int AnalysisEpisodicCursor::getSampleDelay(int, int) {
    return sampleDelays[0];
}

AnalysisEpisodicAllRepsCursor::AnalysisEpisodicAllRepsCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisEpisodicAllRepsCursor::AnalysisEpisodicAllRepsCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisEpisodicAllRepsCursor::checkTuple(int itemIdx, int, int sweepIdx) {
    return ((this->itemIdx == itemIdx) &&
            (this->sweepIdx == sweepIdx));
}

int AnalysisEpisodicAllRepsCursor::getSampleDelay(int, int) {
    return sampleDelays[0];
}

AnalysisEpisodicAllSweepsCursor::AnalysisEpisodicAllSweepsCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisEpisodicAllSweepsCursor::AnalysisEpisodicAllSweepsCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisEpisodicAllSweepsCursor::checkTuple(int itemIdx, int repsIdx, int) {
    return ((this->itemIdx == itemIdx) &&
            (this->repsIdx == repsIdx));
}

int AnalysisEpisodicAllSweepsCursor::getSampleDelay(int, int sweepIdx) {
    return sampleDelays[sweepIdx];
}

AnalysisEpisodicAllRepsAllSweepsCursor::AnalysisEpisodicAllRepsAllSweepsCursor(AnalysisCursorType_t type) :
    AnalysisCursor(type) {

}

AnalysisEpisodicAllRepsAllSweepsCursor::AnalysisEpisodicAllRepsAllSweepsCursor(AnalysisCursor * cursor) :
    AnalysisCursor(cursor) {

}

bool AnalysisEpisodicAllRepsAllSweepsCursor::checkTuple(int itemIdx, int, int) {
    return (this->itemIdx == itemIdx);
}

int AnalysisEpisodicAllRepsAllSweepsCursor::getSampleDelay(int, int sweepIdx) {
    return sampleDelays[sweepIdx];
}
