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

class SpinBoxWithChannel;

class ChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ChannelControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

public slots:
    void onUpdate();

private:
    typedef enum Operations {
        OperationTurnChannelsOnOff,
        OperationTurnStimulusOnOff,
        OperationStartStopDigitalOffsetCompensation,
        OperationHoldingStimulus,
        OperationRecordToFile,
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

    QVector <QWidget *> operationWidgets;

    QVector <QVector <QWidget *>> operationEdits;

    QVector <QWidget *> operationButtonWidgets;

private slots:
    void onOperationSelected(int operationIdx);
    void onApplyButtonClicked();
    void onCheckAllButtonClicked();
    void onUncheckAllButtonClicked();
    void onSetAllButtonClicked();
    void onStartRecordingButtonClicked();
    void onStopRecordingButtonClicked();

signals:
    void sigAppliedTurnChannelOnOff(vector<uint16_t> channelIndexes, vector<bool> onvalues);
    void sigAppliedTurnDocOnOff(vector<uint16_t> channelIndexes, vector<bool> onvalues);
    void sigAppliedVoltageHoldValues(vector<uint16_t> channelIndexes, vector<Measurement_t> voltages);
    void sigAppliedTurnStimulsOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues);
    void sigStartRecording(vector<uint16_t> channelIndexes, vector<bool> onValues);
    void sigStopRecording();
};

class SpinBoxWithChannel : public QWidget {
    Q_OBJECT

public:
    SpinBoxWithChannel(int idx, MySpinBox * sbx);
    SpinBoxWithChannel(QString title, MySpinBox * sbx);

    double value();
    void setValue(double value);

private:
    QLabel * channelLbl;
    MySpinBox * valueSbx;

};

#endif // CHANNELCONTROLDOCKWIDGET_H
