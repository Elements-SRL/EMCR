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
    ~MultipleChannelController();
    void addRemoveFromBigPlot(bool flag);

public slots:
    void onRecordingRequest(bool flag);
    void onRecordingExecution(bool flag);

private:
    void turnSelectedChannelsOnOff(bool flag);
    void turnSelectedCalibrationResistorsOnOff(bool flag);
    void turnSelectedStimuliOnOff(bool flag);
    void turnSelectedOffsetRecalibrationOnOff(bool flag);
    void resetOffsetRecalibration();
    void turnSelectedLjcOnOff(bool flag);
    void resetLj();

    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    MultipleChannelControlDockWidget * multipleChannelControlsDw = nullptr;

signals:
    void sigStartRecording();
    void sigStopRecording();

    void sigChannelsTurnedOnOff(bool flag);
    void sigCalibrationResistorsTurnedOnOff(bool flag);
    void sigStimuliTurnedOnOff(bool flag);
    void sigOffsetRecalibrationTurnedOnOff(bool flag);
    void sigOffsetRecalibrationResetted();
    void sigLjcTurnedOnOff(bool flag);
    void sigLjResetted();
    void sigAddRemoveFromBigPlot(bool flag);
};

#endif // MULTIPLECHANNELCONTROLLER_H
