#ifndef BOARDCONTROLDOCKWIDGET_H
#define BOARDCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QHBoxLayout>

#include "modeldevice.h"
#include "myspinbox.h"

class BoardControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    BoardControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

private:
    QGridLayout * getLayoutWithScrollBar(QWidget * widget);

    ModelDevice * mDev;
    vector<double> previousGateSpinBoxValues;
    vector<double> previousSourceSpinBoxValues;
    vector<MySpinBox*>  gateSpinBoxes;
    vector<MySpinBox*>  sourceSpinBoxes;

    vector<uint16_t> getChangedChannelIndexes(vector<MySpinBox*> spinBoxVector, vector<double> previousChannelValues);
    void onApplyButtonClicked();

signals:
    void sigGateSourceVoltagesApplied(vector<uint16_t> gateVoltageBoardIndexes, vector<Measurement_t> gateVoltages, vector<uint16_t> sourceVoltageBoardIndexes, vector<Measurement_t> sourceVoltages);
};

#endif // BOARDCONTROLDOCKWIDGET_H
