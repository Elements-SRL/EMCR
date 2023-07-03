#ifndef COMPENSATIONCONTROLDOCKWIDGET_H
#define COMPENSATIONCONTROLDOCKWIDGET_H
#include <QDockWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>

#include "messagedispatcher.h"
#include "myspinbox.h"

class CompensationControlDockWidget: public QDockWidget {
    Q_OBJECT

public:
    CompensationControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void onCompValuesDispatched(std::vector<std::vector<double>> compValueMatrix, std::vector<RangedMeasurement> cfastFeatures, std::vector<RangedMeasurement> cslowFeatures, std::vector<RangedMeasurement> rsFeatures, std::vector<RangedMeasurement> rsCpFeatures, std::vector<RangedMeasurement> rsPgFeatures , std::vector<RangedMeasurement> ccCfastFeatures);

private:
    QGridLayout * getLayoutWithScrollBar(QWidget * widget);

    MessageDispatcher * msgDisp = nullptr;

    std::vector<QCheckBox*>  cfastCheckBoxes;
    std::vector<MySpinBox*>  cfastSpinBoxes;
    std::vector<QCheckBox*>  cslowRsCheckBoxes;
    std::vector<MySpinBox*>  cslowSpinBoxes;
    std::vector<MySpinBox*>  rsSpinBoxes;
    std::vector<QCheckBox*>  rsCpCheckBoxes;
    std::vector<MySpinBox*>  rsCpSpinBoxes;
    std::vector<QCheckBox*>  rsPgCheckBoxes;
    std::vector<MySpinBox*>  rsPgSpinBoxes;
    std::vector<QComboBox*>  rsBwComboBoxes;
    std::vector<QCheckBox*>  ccCfastCheckBoxes;
    std::vector<MySpinBox*>  ccCfastSpinBoxes;

    std::vector<QHBoxLayout*> hbsCfast;
    std::vector<QHBoxLayout*> hbsCslowRs;
    std::vector<QHBoxLayout*> hbsRsCp;
    std::vector<QHBoxLayout*> hbsRsPg;
    std::vector<QHBoxLayout*> hbsCcCfast;

    void onApplyButtonClicked();

signals:
    void sigCompensationsApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues);
};

#endif // COMPENSATIONCONTROLDOCKWIDGET_H
