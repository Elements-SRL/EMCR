#ifndef MULTIPLECHANNELCONTROLDOCKWIDGET_H
#define MULTIPLECHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>

#include "messagedispatcher.h"

class MultipleChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void setChannelsAuto(bool flag);
    void setStimulusAuto(bool flag);
    void setExpandAuto(bool flag);
    bool getExpertMode();
    void enableExpertMode(bool flag);

public slots:
    void onSetClampingModality(ClampingModality_t clampingModality);

private:
    MessageDispatcher * msgDisp = nullptr;

    QPushButton * switchChannelsOnBtn = nullptr;
    QPushButton * switchChannelsOffBtn = nullptr;
    QPushButton * switchChannelsAutoBtn = nullptr;
    QPushButton * calibrationResistorsOnBtn = nullptr;
    QPushButton * calibrationResistorsOffBtn = nullptr;
    QPushButton * turnStimulusOnBtn = nullptr;
    QPushButton * turnStimulusOffBtn = nullptr;
    QPushButton * turnStimulusAutoBtn = nullptr;
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
    QPushButton * expandTraceAutoBtn = nullptr;
    QPushButton * recordingStartBtn = nullptr;
    QPushButton * recordingStopBtn = nullptr;
    QLineEdit * fileNameLineEdit = nullptr;
    QLineEdit * recordPathLineEdit = nullptr;
    QGroupBox * zapGb = nullptr;

signals:
    void sigTurnChannelOn();
    void sigTurnChannelOff();
    void sigTurnChannelAuto(bool flag);
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
    void sigTurnStimulusAuto(bool flag);
    void sigZap(Measurement_t duration);
    void sigAddToBigPlot();
    void sigRemoveFromBigPlot();
    void sigAddToBigPlotAuto(bool flag);
    void sigAddPlotDetail();
    void sigRemovePlotDetail();
    void sigAddPlotDetailAuto(bool flag);
};

#endif // MULTIPLECHANNELCONTROLDOCKWIDGET_H
