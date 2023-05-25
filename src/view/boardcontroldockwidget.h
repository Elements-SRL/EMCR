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
    std::vector<double> previousGateSpinBoxValues;
    std::vector<double> previousSourceSpinBoxValues;
    std::vector<MySpinBox*>  gateSpinBoxes;
    std::vector<MySpinBox*>  sourceSpinBoxes;

    std::vector<uint16_t> getChangedChannelIndexes(std::vector<MySpinBox*> spinBoxVector, std::vector<double> previousChannelValues);
    void onApplyButtonClicked();

signals:
    void sigGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);
};

#endif // BOARDCONTROLDOCKWIDGET_H
