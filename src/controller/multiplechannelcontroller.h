#ifndef MULTIPLECHANNELCONTROLLER_H
#define MULTIPLECHANNELCONTROLLER_H

#include <QObject>

#include "multiplechannelcontroldockwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"

class MultipleChannelController : public QObject {
    Q_OBJECT

public:
    MultipleChannelController(MessageDispatcher * msgDisp, MainWindow * mainWindow);

public slots:
    void onRecordingRequest(bool flag);
    void onRecordingExecution(bool flag);

private:
    void turnSelectedChannelsOnOff(bool flag);
    void turnSelectedStimuliOnOff(bool flag);
    void turnSelectedDocOnOff(bool flag);

    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    MultipleChannelControlDockWidget * multipleChannelControlsDw = nullptr;

signals:
    void sigStartRecording();
    void sigStopRecording();
    void sigAddToBigPlot();
    void sigRemoveFromBigPlot();

    void sigChannelsTurnedOnOff(bool flag);
    void sigStimuliTurnedOnOff(bool flag);
    void sigDocTurnedOnOff(bool flag);
};

#endif // MULTIPLECHANNELCONTROLLER_H
