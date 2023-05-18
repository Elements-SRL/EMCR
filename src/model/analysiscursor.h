#ifndef ANALYSISCURSOR_H
#define ANALYSISCURSOR_H

#include <QVector>

typedef enum {
    AnalysisCursorGapFree,
    AnalysisCursorGapFreeAllReps,
    AnalysisCursorGapFreeInfReps,
    AnalysisCursorGapFreeInfAllReps,
    AnalysisCursorEpisodic,
    AnalysisCursorEpisodicAllReps,
    AnalysisCursorEpisodicAllSweeps,
    AnalysisCursorEpisodicAllRepsAllSweeps
} AnalysisCursorType_t;

class AnalysisCursor {
public:
    AnalysisCursor(AnalysisCursorType_t type);
    AnalysisCursor(AnalysisCursor * cursor);
    virtual ~AnalysisCursor();

    void setDelays(QVector <double> delays);
    QVector <double> getDelays();
    void setSamplingRate(double samplingRate);
    virtual bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) = 0;
    virtual int getSampleDelay(int repsIdx, int sweepIdx) = 0;

    AnalysisCursorType_t type;
    int itemIdx;
    int repsIdx;
    int sweepIdx;

protected:
    QVector <double> delays;
    QVector <int> sampleDelays;
};

class AnalysisGapFreeCursor : public AnalysisCursor {
public:
    AnalysisGapFreeCursor(AnalysisCursorType_t type = AnalysisCursorGapFree);
    AnalysisGapFreeCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

class AnalysisGapFreeAllRepsCursor : public AnalysisCursor {
public:
    AnalysisGapFreeAllRepsCursor(AnalysisCursorType_t type = AnalysisCursorGapFreeAllReps);
    AnalysisGapFreeAllRepsCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

class AnalysisGapFreeInfRepsCursor : public AnalysisCursor {
public:
    AnalysisGapFreeInfRepsCursor(AnalysisCursorType_t type = AnalysisCursorGapFreeInfReps);
    AnalysisGapFreeInfRepsCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

class AnalysisGapFreeInfAllRepsCursor : public AnalysisCursor {
public:
    AnalysisGapFreeInfAllRepsCursor(AnalysisCursorType_t type = AnalysisCursorGapFreeInfAllReps);
    AnalysisGapFreeInfAllRepsCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

class AnalysisEpisodicCursor : public AnalysisCursor {
public:
    AnalysisEpisodicCursor(AnalysisCursorType_t type = AnalysisCursorEpisodic);
    AnalysisEpisodicCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

class AnalysisEpisodicAllRepsCursor : public AnalysisCursor {
public:
    AnalysisEpisodicAllRepsCursor(AnalysisCursorType_t type = AnalysisCursorEpisodicAllReps);
    AnalysisEpisodicAllRepsCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

class AnalysisEpisodicAllSweepsCursor : public AnalysisCursor {
public:
    AnalysisEpisodicAllSweepsCursor(AnalysisCursorType_t type = AnalysisCursorEpisodicAllSweeps);
    AnalysisEpisodicAllSweepsCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

class AnalysisEpisodicAllRepsAllSweepsCursor : public AnalysisCursor {
public:
    AnalysisEpisodicAllRepsAllSweepsCursor(AnalysisCursorType_t type = AnalysisCursorEpisodicAllRepsAllSweeps);
    AnalysisEpisodicAllRepsAllSweepsCursor(AnalysisCursor * cursor);

    bool checkTuple(int itemIdx, int repsIdx, int sweepIdx) override;
    int getSampleDelay(int repsIdx, int sweepIdx) override;
};

#endif // ANALYSISCURSOR_H
