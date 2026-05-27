#ifndef MULTIPLECHANNELCONTROLDOCKWIDGET_H
#define MULTIPLECHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QMap>
#include <QString>

#include "messagedispatcher.h"
#include "autotoggle.h"


enum ChannelProperty {
    EXPAND,
    PLOT_DETAIL,
    CH_INPUT,
    STIMULUS,
    RECALIBRATION,
    LIQUID_JUNCTION,
    CALIB_RESISTORS

};

static QMap<ChannelProperty, QString> channelPropertyId = {
    {EXPAND, "EXP"},
    {PLOT_DETAIL, "PLT"},
    {CH_INPUT, "CHI"},
    {STIMULUS, "STI"},
    {RECALIBRATION, "REC"},
    {LIQUID_JUNCTION, "LQJ"},
    {CALIB_RESISTORS, "CLR"}
};

static QMap<ChannelProperty, QString> channelPropertyBadge = {
    {EXPAND, "E"},
    {PLOT_DETAIL, "P"},
    {CH_INPUT, "O"},
    {STIMULUS, "X"},
    {RECALIBRATION, "C"},
    {LIQUID_JUNCTION, "J"},
    {CALIB_RESISTORS, "R"}
};

static QMap<ChannelProperty, QString> channelPropertyName = {
    {EXPAND, "Expand Trace"},
    {PLOT_DETAIL, "Plot Detail"},
    {CH_INPUT, "Channel Input"},
    {STIMULUS, "Stimulus"},
    {RECALIBRATION, "Recalibration"},
    {LIQUID_JUNCTION, "Liquid Junction"},
    {CALIB_RESISTORS, "Calibration Resistors"}

};

class MultipleChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    MultipleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void setChannelsAuto(bool flag);
    void setStimulusAuto(bool flag);
    void setExpandAuto(bool flag);
    void setPlotDetailAuto(bool flag);
    bool getExpertMode();
    void enableExpertMode(bool flag);
    void setSelectionCount(int count, int totalChannels);
    void updateSummary(const ChannelProperty &propertyType, const QString &text, const QString &status);
    void updateFeatureDetail(const ChannelProperty &propertyType, int onCount, int offCount, bool isAuto, bool isEmpty);
    void enableDisableControls(ChannelProperty propertyType, bool flag);

public slots:
    void onSetClampingModality(ClampingModality_t clampingModality);

private:
    MessageDispatcher * msgDisp = nullptr;

    QPushButton * switchChannelsOnBtn = nullptr;
    QPushButton * switchChannelsOffBtn = nullptr;
    AutoToggle * switchChannelsAutoBtn = nullptr;
    QPushButton * calibrationResistorsOnBtn = nullptr;
    QPushButton * calibrationResistorsOffBtn = nullptr;
    QPushButton * turnStimulusOnBtn = nullptr;
    QPushButton * turnStimulusOffBtn = nullptr;
    AutoToggle * turnStimulusAutoBtn = nullptr;
    QPushButton * zapBtn = nullptr;
    QPushButton * offsetCorrectionStartBtn = nullptr;
    QPushButton * offsetCorrectionStopBtn = nullptr;
    QPushButton * offsetCorrectionMode = nullptr;
    QPushButton * offsetRecalibrationOnBtn = nullptr;
    QPushButton * offsetRecalibrationOffBtn = nullptr;
    QPushButton * offsetRecalibrationResetBtn = nullptr;
    QPushButton * liquidJunctionCompensationOnBtn = nullptr;
    QPushButton * liquidJunctionCompensationOffBtn = nullptr;
    QPushButton * liquidJunctionCompensationResetBtn = nullptr;
    QPushButton * expandTraceBtn = nullptr;
    QPushButton * reduceTraceBtn = nullptr;
    AutoToggle * expandTraceAutoBtn = nullptr;
    AutoToggle * plotDetailAutoBtn = nullptr;
    QPushButton * expandChannelDetailBtn = nullptr;
    QPushButton * reduceChannelDetailBtn = nullptr;
    QPushButton * recordingStartBtn = nullptr;
    QPushButton * recordingStopBtn = nullptr;
    QLineEdit * fileNameLineEdit = nullptr;
    QLineEdit * recordPathLineEdit = nullptr;
    QGroupBox * zapGb = nullptr;
    QLabel * m_selectionCounterLabel = nullptr;
    std::map<QString, QLabel*> m_summaryLabels;
    std::map<QString, QLabel*> m_featureOnLabels;
    std::map<QString, QLabel*> m_featureOffLabels;

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
    // Plot detail
    void sigAddRemovePlotDetail(bool flag);
    void sigAddPlotDetailAuto(bool flag);
};

#endif // MULTIPLECHANNELCONTROLDOCKWIDGET_H
