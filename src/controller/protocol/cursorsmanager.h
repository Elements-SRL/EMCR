#ifndef CURSORSMANAGER_H
#define CURSORSMANAGER_H

#include "protocolcursor.h"
#include "protocolitemdroplist.h"
#include "protocolplot.h"

class ProtocolWidget;

class CursorsManager : public QObject {
    Q_OBJECT

public:
    CursorsManager(ProtocolPlot * plot, unsigned int maxTriggerEvents);

    void setProtocol(ProtocolWidget * protocol);

public slots:
    void onAddCursors();
    void onRemoveCursors(QVector <int> cursorsMap);
    void onUpdateCursors();

private:
    void interpretCursors();

    ProtocolPlot * plot;
    unsigned int maxTriggerEvents;
    ProtocolWidget * protocol;
    QVector <ProtocolCursor *> * cursors;
    QVector <TriggerCursor *> triggerCursors;
};

#endif // CURSORSMANAGER_H
