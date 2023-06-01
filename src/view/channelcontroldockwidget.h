#ifndef CHANNELCONTROLDOCKWIDGET_H
#define CHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "modeldevice.h"
#include "myspinbox.h"
#include "errormanager.h"

class SpinBoxWithChannel;

class ChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ChannelControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

public slots:
    void onUpdate();
    void onSigRecording(bool state);
    void onVcVoltageRangeSelected(int idx);
    void onCcCurrentRangeSelected(int idx);

private:
    typedef enum Operations {
        OperationTurnChannelsOnOff,
        OperationTurnStimulusOnOff,
        OperationStartStopDigitalOffsetCompensation,
        OperationHoldingStimulus,
        OperationRecordToFile,
        OperationPlotToBigPlot,
        OperationsNum
    } Operations_t;

    QVector <QString> operationTitles;
    QVector <QString> operationString;

    QWidget * createOperationWidget(int idx);
    QWidget * createOperationButtonWidget(int idx);
    QVBoxLayout * getLayoutWithScrollBar(QWidget * widget);

    ModelDevice * mDev = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    QComboBox * operationCbx = nullptr;
    SpinBoxWithChannel * setAllVholdSpinBox = nullptr;

    QPushButton* startRecordingBtn = nullptr;
    QPushButton* stopRecordingBtn = nullptr;
    QPushButton* applyBtn = nullptr;

    QVector <QWidget *> operationWidgets;

    QVector <QVector <QWidget *>> operationEdits;

    QVector <QWidget *> operationButtonWidgets;
    RangedMeasurement_t holdingTunerRange;
    MySpinBox * setAllChannelsSbx;

private slots:
    void onOperationSelected(int operationIdx);
    void onApplyButtonClicked();
    void onApplyButtonClicked(int idx, bool applyAll);
    void onCheckAllButtonClicked();
    void onUncheckAllButtonClicked();
    void onSetAllButtonClicked();
    void onStartRecordingButtonClicked();
    void onStopRecordingButtonClicked();


signals:
    void sigAppliedTurnChannelOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onvalues);
    void sigAppliedTurnDocOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onvalues);
    void sigAppliedHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues);
    void sigAppliedTurnStimulsOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void sigStartRecording(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void sigStopRecording();
    void sigAppliedPlotToBigPlot(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
};

class SpinBoxWithChannel : public QWidget {
    Q_OBJECT

public:
    SpinBoxWithChannel(int idx, MySpinBox * sbx);
    SpinBoxWithChannel(QString title, MySpinBox * sbx);

    MySpinBox * getSpinBox();

private:
    QLabel * channelLbl;
    MySpinBox * valueSbx;
};

#endif // CHANNELCONTROLDOCKWIDGET_H
