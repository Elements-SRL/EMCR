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
    void setVcVoltageRangesroupBoxEnabled(bool status);
    void setVcCurrentRangesGroupBoxEnabled(bool status);
    void setCcVoltageRangesGroupBoxEnabled(bool status);
    void setCcCurrentRangesGroupBoxEnabled(bool status);
    void setSamplingRatesGroupBoxEnabled(bool status);
    void setDownsamplingRatioSbxEnabled(bool status);

private:
    MessageDispatcher * msgDisp = nullptr;
    QGroupBox * vcCurrentRangesGroupBox = nullptr;
    std::vector <QRadioButton *> vcCurrentRangesRadioButtons;
    QGroupBox * vcVoltageRangesGroupBox = nullptr;
    std::vector <QRadioButton *> vcVoltageRangesRadioButtons;
    QGroupBox * ccCurrentRangesGroupBox = nullptr;
    std::vector <QRadioButton *> ccCurrentRangesRadioButtons;
    QGroupBox * ccVoltageRangesGroupBox = nullptr;
    std::vector <QRadioButton *> ccVoltageRangesRadioButtons;
    QGroupBox * vcVoltageFiltersGroupBox = nullptr;
    std::vector <QRadioButton *> vcVoltageFiltersRadioButtons;
    QGroupBox * ccCurrentFiltersGroupBox = nullptr;
    std::vector <QRadioButton *> ccCurrentFiltersRadioButtons;
    QGroupBox * samplingRatesGroupBox = nullptr;
    std::vector <QRadioButton *> samplingRatesRadioButtons;
    QGroupBox * downsamplingRatiosGroupBox = nullptr;
    QSpinBox * downsamplingRatioSbx;
    QLabel * finalSamplingRateLbl;
    QGroupBox * clampingModalitiesGroupBox = nullptr;
    std::vector <QRadioButton *> clampingModalitiesRadioButtons;
    std::vector <QGroupBox *> customOptionsGroupBoxes;
    std::vector <std::vector <QRadioButton *>> customOptionsRadioButtons;
    std::vector <QGroupBox *> customDoublesGroupBoxes;
    std::vector <QDoubleSpinBox *> customDoublesSpinBoxes;

    QGroupBox * setupGroupBox(std::string title, std::vector <RangedMeasurement> rangedMeasurements, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons);
    QGroupBox * setupGroupBox(std::string title, std::vector <Measurement> measurements, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons);
    QGroupBox * setupGroupBox(std::string title, std::vector <std::string> strings, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons);
    QGroupBox * setupGroupBox(std::string title, std::vector <QString> texts, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons);
    QGroupBox * setupGroupBox(std::string title, QVBoxLayout * parentLayout, RangedMeasurement_t range, double valueDefault, QDoubleSpinBox * &spinbox);

    void setWidgetEnabled(QWidget * widget, bool status);
    void setWidgetVisible(QWidget * widget, bool status);

signals:
    void sigVcCurrentRangeSelected(int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int idx);
    void sigVcVoltageFilterSelected(int idx);
    void sigCcCurrentFilterSelected(int idx);
    void sigSamplingRateSelected(int idx);
    void sigCustomOptionSelected(unsigned int customOptionIdx, int idx);
    void sigCustomDoubleChanged(unsigned int customDoubleIdx, double value);
    void sigDownsamplingRatioSelected(int ratio);
    void sigClampingModalitySelected(ClampingModality_t mode);
};

#endif // DEVICECONTROLDOCKWIDGET_H
