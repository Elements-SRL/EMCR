#ifndef MULTIPLECHANNELCONTROLDOCKWIDGET_H
#define MULTIPLECHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>

#include "messagedispatcher.h"
#include "globaldefines.h"

class MultipleChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void setRecording(bool flag);
    bool getExpertMode();
    void enableExpertMode(bool flag);

private:
    MessageDispatcher * msgDisp = nullptr;

    QPushButton * switchChannelsOnBtn = nullptr;
    QPushButton * switchChannelsOffBtn = nullptr;
    QPushButton * calibrationResistorsOnBtn = nullptr;
    QPushButton * calibrationResistorsOffBtn = nullptr;
    QPushButton * turnStimulusOnBtn = nullptr;
    QPushButton * turnStimulusOffBtn = nullptr;
    QPushButton * zapBtn = nullptr;
    QPushButton * offsetCorrectionStartBtn = nullptr;
    QPushButton * offsetCorrectionStopBtn = nullptr;
    QCheckBox * offsetCorrectionExpertChb = nullptr;
    QPushButton * offsetRecalibrationOnBtn = nullptr;
    QPushButton * offsetRecalibrationOffBtn = nullptr;
    QPushButton * offsetRecalibrationResetBtn = nullptr;
    QPushButton * liquidJunctionCompensationOnBtn = nullptr;
    QPushButton * liquidJunctionCompensationOffBtn = nullptr;
    QPushButton * liquidJunctionCompensationResetBtn = nullptr;
    QPushButton * expandTraceBtn = nullptr;
    QPushButton * reduceTraceBtn = nullptr;
    QPushButton * recordingStartBtn = nullptr;
    QPushButton * recordingStopBtn = nullptr;
    QLineEdit * fileNameLineEdit = nullptr;
    QLineEdit * recordPathLineEdit = nullptr;
    void emitFileName();
    void emitFilePath();

signals:
    void sigTurnChannelOn();
    void sigTurnChannelOff();
    void sigTurnCalibrationResistorsOn();
    void sigTurnCalibrationResistorsOff();
    void sigStartOffsetCorrection();
    void sigStopOffsetCorrection();
    void sigTurnOffsetRecalibrationOn();
    void sigTurnOffsetRecalibrationOff();
    void sigResetOffsetRecalibration();
    void sigTurnLjcOn();
    void sigTurnLjcOff();
    void sigResetLj();
    void sigTurnStimulsOn();
    void sigTurnStimulsOff();
    void sigZap(Measurement_t duration);
    void sigStartRecording();
    void sigStopRecording();
    void sigAddToBigPlot();
    void sigRemoveFromBigPlot();
    void sigFileNameChanged(QString);
    void sigRecordPathChanged(QString);
};

#endif // MULTIPLECHANNELCONTROLDOCKWIDGET_H
