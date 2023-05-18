#ifndef CURSORSMANAGER_H
#define CURSORSMANAGER_H

#include "protocolcursor.h"
#include "protocolitemdroplist.h"
#include "plot.h"
#include "analysiscursor.h"

class ProtocolWidget;

class CursorsManager : public QObject {
    Q_OBJECT

public:
    CursorsManager(ProtocolPlot * plot, unsigned int maxTriggerEvents);

    void setProtocol(ProtocolWidget * protocol);
    void setAnalysisPidl(AnalysisProtocolItemDropList * analysisPidl);
    void enableAnalysis(bool enabled);

public slots:
    void onRequestCursors(ProtocolDropAnalysisItem * item);
    void onAddCursors();
    void onRemoveCursors(QVector <int> cursorsMap);
    void onUpdateCursors();

private:
    void interpretCursors();

    ProtocolPlot * plot;
    unsigned int maxTriggerEvents;
    ProtocolWidget * protocol;
    AnalysisProtocolItemDropList * analysisPidl;
    QVector <ProtocolCursor *> * cursors;
    QVector <AnalysisCursor *> analysisCursors;
    QVector <TriggerCursor *> triggerCursors;
};

#endif // CURSORSMANAGER_H
