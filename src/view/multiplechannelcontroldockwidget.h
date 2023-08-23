#ifndef MULTIPLECHANNELCONTROLDOCKWIDGET_H
#define MULTIPLECHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QPushButton>

#include "messagedispatcher.h"
#include "globaldefines.h"


class MultipleChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void setRecording(bool flag);

private:
    MessageDispatcher * msgDisp = nullptr;

    QPushButton * switchChannelsOnBtn = nullptr;
    QPushButton * switchChannelsOffBtn = nullptr;
    QPushButton * turnStimulusOnBtn = nullptr;
    QPushButton * turnStimulusOffBtn = nullptr;
    QPushButton * offsetCompensationOnBtn = nullptr;
    QPushButton * offsetCompensationOffBtn = nullptr;
    QPushButton * offsetCompensationResetBtn = nullptr;
    QPushButton * expandTraceBtn = nullptr;
    QPushButton * reduceTraceBtn = nullptr;
    QPushButton * recordingStartBtn = nullptr;
    QPushButton * recordingStopBtn = nullptr;

signals:
    void sigTurnChannelOn();
    void sigTurnChannelOff();
    void sigTurnDocOn();
    void sigTurnDocOff();
    void sigResetDoc();
    void sigTurnStimulsOn();
    void sigTurnStimulsOff();
    void sigStartRecording();
    void sigStopRecording();
    void sigAddToBigPlot();
    void sigRemoveFromBigPlot();
    void sigFileNameChanged(QString);
    void sigRecordPathChanged(QString);
};

#endif // MULTIPLECHANNELCONTROLDOCKWIDGET_H
