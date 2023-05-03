#ifndef COMPENSATIONCONTROLDOCKWIDGET_H
#define COMPENSATIONCONTROLDOCKWIDGET_H
#include <QDockWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>

#include "modeldevice.h"
#include "myspinbox.h"

class CompensationControlDockWidget: public QDockWidget {
    Q_OBJECT

public:
    CompensationControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

private:
    QGridLayout * getLayoutWithScrollBar(QWidget * widget);

    ModelDevice * mDev;

    vector<QCheckBox*>  cfastCheckBoxes;
    vector<MySpinBox*>  cfastSpinBoxes;
    vector<QCheckBox*>  cslowRsCheckBoxes;
    vector<MySpinBox*>  cslowSpinBoxes;
    vector<MySpinBox*>  rsSpinBoxes;
    vector<QCheckBox*>  rsCpCheckBoxes;
    vector<MySpinBox*>  rsCpSpinBoxes;
    vector<QCheckBox*>  rsPgCheckBoxes;
    vector<MySpinBox*>  rsPgSpinBoxes;
    vector<QComboBox*>  rsBwComboBoxes;

    vector<QHBoxLayout*> hbsCfast;
    vector<QHBoxLayout*> hbsCslowRs;
    vector<QHBoxLayout*> hbsRsCp;
    vector<QHBoxLayout*> hbsRsPg;

    void onApplyButtonClicked();

signals:
    void sigCompensationsApplied(vector<uint16_t> channelIndexes, vector<bool> cfastEn, vector<bool> cslowRsEn, vector<bool> rsCpEn, vector<bool> rsPgEn);


};

#endif // COMPENSATIONCONTROLDOCKWIDGET_H
