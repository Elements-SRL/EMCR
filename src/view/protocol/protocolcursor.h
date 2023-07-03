#ifndef PROTOCOLCURSOR_H
#define PROTOCOLCURSOR_H

#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QStandardItemModel>

#include "qwt_plot_marker.h"

#include "protocolsection.h"
#include "protocoldefs.h"
#include "cursor.h"
#include "messagedispatcher.h"

class ProtocolWidget;

class ProtocolCursor : public QObject {
    Q_OBJECT

public:
    typedef enum {
        LocationFromStart,
        LocationToEnd
    } LocationType_t;

    typedef enum {
        RepetitionAll,
        RepetitionLast,
        RepetitionOne
    } RepetitionType_t;

    typedef enum {
        SweepAll,
        SweepLast,
        SweepOne
    } SweepType_t;

    typedef enum {
        TriggerNone,
        TriggerRising,
        TriggerFalling
    } TriggerType_t;

    ProtocolCursor(MessageDispatcher * msgDisp, QwtPlot * plot, double x, int cursorIdx);
    ~ProtocolCursor();

    void openPropertyDialog();
    void setVisible(bool visible);
    void setSection(ProtocolSection * section, double offset, ProtocolType_t protocolType);
    bool updateSection(ProtocolSection * section, ProtocolType_t protocolType);
    void setCursorIdx(int idx);

    int getItemIdx();
    ProtocolType_t getProtocolType();

    int getLocationType();
    double getLocationDelay();

    int getRepetitionType();
    int getRepetitionIdx();
    int getRepetitionsNum();

    int getLoopIdx();

    int getSweepType();
    int getSweepIdx();
    int getSweepsNum();

    int getTriggerType();
    int getTriggerId();

    void setXValue(double x);
    double getXValue();
    double getOffset();
    double getOffset(int sectionRepsIdx, int sectionSweepIdx);
    double getAbsoluteOffset();
    double getAbsoluteOffset(int sectionRepsIdx, int sectionSweepIdx);

    bool precedes(ProtocolCursor * cursor);
    bool sameRepetitions(ProtocolCursor * cursor);

    YAML::Cursor getYamlCursor();

    void setCursorFromYaml(const YAML::Cursor &yamlCursor);

private:
    void initializePropertyDialog();
    void getLastRepComboItem();
    void setMaxDuration();

    bool precedesLoopLevel(ProtocolCursor * cursor, int sweepIter);
    bool precedesItemLevel(ProtocolCursor * cursor, int sweepIter, int repetitionIter);
    bool sameRepetitionsLoopLevel(ProtocolCursor * cursor);

    MessageDispatcher * msgDisp = nullptr;
    int cursorIdx;
    ProtocolType_t protocolType;

    QwtPlotMarker * marker;
    QDialog * propertyDialog;
    ProtocolSection * protocolSection = nullptr;
    ProtocolSection * protocolSectionOrig = nullptr;

    QComboBox * locationCb;
    QDoubleSpinBox * locationSb;
    int locationType = LocationFromStart;
    double locationDelay = 0.0;

    QComboBox * repetitionCb;
    QSpinBox * repetitionSb;
    int repetitionType = RepetitionAll;
    int repetitionIdx = 1;
    QStandardItem * lastRepItem;

    QGroupBox * sweepGb;
    QComboBox * sweepCb;
    QSpinBox * sweepSb;
    int sweepType = SweepAll;
    int sweepIdx = 1;

    QComboBox * triggerCb;
    QSpinBox * triggerSb;
    int triggerType = TriggerNone;
    int triggerId = 1;

    int itemIdx = 0;

private slots:
    void onRepetitionIdxChanged(int repsIdx);
    void onSweepIdxChanged(int sweepIdx);

    void onAcceptPropertyDialog();
    void onRejectPropertyDialog();

signals:
    void cursorOpenPropertiesRequest(int);
    void cursorDeleteRequest(int);
    void propertiesAccepted();
};

class TriggerCursor {
public:
    TriggerCursor(int id, bool high, double delay, bool terminator = false);
    TriggerCursor(const TriggerCursor * cursor);

    bool operator < (const TriggerCursor &a) const;

    int id;
    bool high;
    double delay;
    bool terminator;
};

struct TriggerCursorsCompare {
    bool operator () (const TriggerCursor * a, const TriggerCursor * b) {
        return (*a < *b);
    }
};

class TriggerTerminator : public TriggerCursor {
public:
    TriggerTerminator();
};

#endif // PROTOCOLCURSOR_H
