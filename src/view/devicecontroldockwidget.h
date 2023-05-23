#ifndef DEVICECONTROLDOCKWIDGET_H
#define DEVICECONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QGroupBox>
#include "model/modeldevice.h"
#include <QRadioButton>
#include <QVBoxLayout>

#define DCW_CURRENT_RANGE_TITLE "Current Ranges"
#define DCW_VOLTAGE_RANGE_TITLE "Voltage Ranges"
#define DCW_SAMPLING_RATE_TITLE "Sampling Rates"
#define DCW_STIMULUS_FILTER_TITLE "Readout Filters"

#define DCW_CC_CURRENT_RANGE_TITLE "CC Current Ranges"
#define DCW_CC_VOLTAGE_RANGE_TITLE "CC Voltage Ranges"

#define DCW_CLMAPINGMODALITY_TITLE "Clamping Modality"

class DeviceControlDockWidget : public QDockWidget{
    Q_OBJECT

public:
    DeviceControlDockWidget(ModelDevice * mDev);

    void forceEmit();
    void updateParameters();

public slots:
    void onStartRecording(vector<uint16_t> channelIndexes, vector<bool> onValues);
    void onStopRecording();

private:
    ModelDevice * mDev;
    QGroupBox * vcCurrentRangesGroupBox = nullptr;
    vector<QRadioButton *> vcCurrentRangesRadioButtons;
    bool vcCurrentRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * vcVoltageRangesGroupBox = nullptr;
    vector<QRadioButton *> vcVoltageRangesRadioButtons;
    bool vcVoltageRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * ccCurrentRangesGroupBox = nullptr;
    vector<QRadioButton *> ccCurrentRangesRadioButtons;
    bool ccCurrentRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * ccVoltageRangesGroupBox = nullptr;
    vector<QRadioButton *> ccVoltageRangesRadioButtons;
    bool ccVoltageRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * samplingRatesGroupBox = nullptr;
    vector<QRadioButton *> samplingRatesRadioButtons;
    bool samplingRatesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * clampingModalitiesGroupBox = nullptr;
    vector<QRadioButton *> clampingModalitiesRadioButtons;
    bool clampingModalitiesPrevioueEnableStateBeforeRecording = false;

    /*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//    void testFunction(QVBoxLayout* vLayout, QGroupBox* qGroupBox, vector <RangedMeasurement_t> myRanges, vector<QRadioButton *> &qRadioButtons);

signals:
    void sigVcCurrentRangeSelected(int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int idx);
    void sigSamplingRateSelected(int idx);
    void sigClampingModalitySelected(int idx);
};

#endif // DEVICECONTROLDOCKWIDGET_H
