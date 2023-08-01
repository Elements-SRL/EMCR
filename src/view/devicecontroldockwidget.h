#ifndef DEVICECONTROLDOCKWIDGET_H
#define DEVICECONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QLabel>

#include "messagedispatcher.h"

#define DCW_CURRENT_RANGE_TITLE "Current Ranges"
#define DCW_VOLTAGE_RANGE_TITLE "Voltage Ranges"

#define DCW_CC_CURRENT_RANGE_TITLE "Current Ranges"
#define DCW_CC_VOLTAGE_RANGE_TITLE "Voltage Ranges"

#define DCW_STIMULUS_FILTER_TITLE "Stimulus Filters"

#define DCW_SAMPLING_RATE_TITLE "Sampling Rates"
#define DCW_DOWNSAMPLING_RATIO_TITLE "Downsampling Ratio"
#define DCW_CLMAPINGMODALITY_TITLE "Clamping Modality"

class DeviceControlDockWidget : public QDockWidget{
    Q_OBJECT

public:
    DeviceControlDockWidget(MessageDispatcher * msgDisp);

    void forceEmit();
    void updateParameters();
    void onRecordingStarted();
    void onRecordingStopped();

private:
    MessageDispatcher * msgDisp = nullptr;
    QGroupBox * vcCurrentRangesGroupBox = nullptr;
    std::vector<QRadioButton *> vcCurrentRangesRadioButtons;
    bool vcCurrentRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * vcVoltageRangesGroupBox = nullptr;
    std::vector<QRadioButton *> vcVoltageRangesRadioButtons;
    bool vcVoltageRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * ccCurrentRangesGroupBox = nullptr;
    std::vector<QRadioButton *> ccCurrentRangesRadioButtons;
    bool ccCurrentRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * ccVoltageRangesGroupBox = nullptr;
    std::vector<QRadioButton *> ccVoltageRangesRadioButtons;
    bool ccVoltageRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * vcVoltageFiltersGroupBox = nullptr;
    std::vector<QRadioButton *> vcVoltageFiltersRadioButtons;
    bool vcVoltageFiltersPrevioueEnableStateBeforeRecording = false;
    QGroupBox * ccCurrentFiltersGroupBox = nullptr;
    std::vector<QRadioButton *> ccCurrentFiltersRadioButtons;
    bool ccCurrentFiltersPrevioueEnableStateBeforeRecording = false;
    QGroupBox * samplingRatesGroupBox = nullptr;
    std::vector<QRadioButton *> samplingRatesRadioButtons;
    bool samplingRatesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * downsamplingRatiosGroupBox = nullptr;
    QSpinBox * downsamplingRatioSbx;
    QLabel * finalSamplingRateLbl;
    bool downsamplingRatiosPrevioueEnableStateBeforeRecording = false;
    QGroupBox * clampingModalitiesGroupBox = nullptr;
    std::vector<QRadioButton *> clampingModalitiesRadioButtons;
    bool clampingModalitiesPrevioueEnableStateBeforeRecording = false;

    /*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//    void testFunction(QVBoxLayout* vLayout, QGroupBox* qGroupBox, std::vector <RangedMeasurement_t> myRanges, std::vector<QRadioButton *> &qRadioButtons);

signals:
    void sigVcCurrentRangeSelected(int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int idx);
    void sigVcVoltageFilterSelected(int idx);
    void sigCcCurrentFilterSelected(int idx);
    void sigSamplingRateSelected(int idx);
    void sigDownsamplingRatioSelected(int ratio);
    void sigClampingModalitySelected(int idx);
};

#endif // DEVICECONTROLDOCKWIDGET_H
