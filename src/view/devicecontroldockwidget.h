#ifndef DEVICECONTROLDOCKWIDGET_H
#define DEVICECONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>

#include "activationbutton.h"
#include "messagedispatcher.h"
#include "collapsiblesection.h"

#define DCW_CURRENT_RANGE_TITLE "Current Ranges"
#define DCW_VOLTAGE_RANGE_TITLE "Voltage Ranges"

#define DCW_CC_CURRENT_RANGE_TITLE "Current Ranges"
#define DCW_CC_VOLTAGE_RANGE_TITLE "Voltage Ranges"

#define DCW_STIMULUS_FILTER_TITLE "Stimulus Filters"

#define DCW_SAMPLING_RATE_TITLE "Sampling Rates"
#define DCW_DOWNSAMPLING_RATIO_TITLE "Downsampling Ratio"
#define DCW_DIGITAL_FILTER_TITLE "Digital Filter"
#define DCW_CLMAPINGMODALITY_TITLE "Clamping Modality"

class DeviceControlDockWidget : public QDockWidget{
    Q_OBJECT

public:
    DeviceControlDockWidget(MessageDispatcher * msgDisp);

    void forceEmit();
    void updateParameters();
    void setVcVoltageRangesSectionEnabled(bool status);
    void setVcCurrentRangesSectionEnabled(bool status);
    void setCcVoltageRangesSectionEnabled(bool status);
    void setCcCurrentRangesSectionEnabled(bool status);
    void setSamplingRatesSectionEnabled(bool status);
    void setDownsamplingRatioSbxEnabled(bool status);

    QLabel * finalBandwidthLbl = nullptr;
    QSpinBox * downsamplingRatioSbx = nullptr;
    ActivationButton * digFiltBtn = nullptr;
    QComboBox * digFiltTypeCbx = nullptr;
    QDoubleSpinBox * digFiltCutoffFreqSbx = nullptr;
    QLabel * finalSamplingRateLbl = nullptr;

private:
    int voltageChannelsNum;
    int currentChannelsNum;

    MessageDispatcher * msgDisp = nullptr;
    std::vector <CollapsibleSection *> vcCurrentRangesSections;
    std::vector <std::vector <QRadioButton *>> vcCurrentRangesRadioButtons;
    CollapsibleSection * vcVoltageRangesSection = nullptr;
    std::vector <QRadioButton *> vcVoltageRangesRadioButtons;
    CollapsibleSection * ccCurrentRangesSection = nullptr;
    std::vector <QRadioButton *> ccCurrentRangesRadioButtons;
    std::vector <CollapsibleSection *> ccVoltageRangesSections;
    std::vector <std::vector <QRadioButton *>> ccVoltageRangesRadioButtons;
    CollapsibleSection * vcVoltageFiltersSection = nullptr;
    std::vector <QRadioButton *> vcVoltageFiltersRadioButtons;
    CollapsibleSection * ccCurrentFiltersSection = nullptr;
    std::vector <QRadioButton *> ccCurrentFiltersRadioButtons;
    CollapsibleSection * samplingRatesSection = nullptr;
    std::vector <QRadioButton *> samplingRatesRadioButtons;

    QGroupBox * downsamplingRatiosGroupBox = nullptr;
    QGroupBox * digitalFilterGroupBox = nullptr;

    ActivationButton * samplingRateToggle = nullptr;
    ActivationButton * downsamplingToggle = nullptr;

    CollapsibleSection * clampingModalitiesSection = nullptr;
    std::vector <QRadioButton *> clampingModalitiesRadioButtons;
    std::vector <ActivationButton *> customFlagsButtons;
    std::vector <CollapsibleSection *> customOptionsSections;
    std::vector <std::vector <QRadioButton *>> customOptionsRadioButtons;
    std::vector <QGroupBox *> customDoublesGroupBoxes;
    std::vector <QDoubleSpinBox *> customDoublesSpinBoxes;

    CollapsibleSection * setupSection(std::string title, std::vector <RangedMeasurement> rangedMeasurements, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx);
    CollapsibleSection * setupSection(std::string title, std::vector <RangedMeasurement> rangedMeasurements, QBoxLayout * parentLayout, std::vector <std::vector <QRadioButton *>> &radioButtons, int defaultIdx);
    CollapsibleSection * setupSection(std::string title, std::vector <Measurement> measurements, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx);
    CollapsibleSection * setupSection(std::string title, std::vector <std::string> strings, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx);
    CollapsibleSection * setupSection(std::string title, std::vector <QString> texts, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx);
    ActivationButton * setupActButton(std::string title, QBoxLayout * parentLayout, bool defaultFlag);
    QGroupBox * setupGroupBox(std::string title, QVBoxLayout * parentLayout, RangedMeasurement_t range, double valueDefault, QDoubleSpinBox * &spinbox);

    void setWidgetEnabled(QWidget * widget, bool status);
    void setWidgetVisible(QWidget * widget, bool status);
    void setDownsamplingVisualState(bool checked, int value, bool spinboxEnabled);

signals:
    void sigVcCurrentRangeSelected(int chIdx, int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int chIdx, int idx);
    void sigVcVoltageFilterSelected(int idx);
    void sigCcCurrentFilterSelected(int idx);
    void sigSamplingRateSelected(int idx);
    void sigCustomFlagSelected(unsigned int customFlagIdx, bool flag);
    void sigCustomOptionSelected(unsigned int customOptionIdx, int idx);
    void sigCustomDoubleChanged(unsigned int customDoubleIdx, double value);
    void sigClampingModalitySelected(ClampingModality_t mode);
    void sigDownsamplingToggleClicked(bool checked);
};

#endif // DEVICECONTROLDOCKWIDGET_H
