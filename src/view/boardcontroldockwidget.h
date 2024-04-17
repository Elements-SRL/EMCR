#ifndef BOARDCONTROLDOCKWIDGET_H
#define BOARDCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QHBoxLayout>

#include "messagedispatcher.h"
#include "nowheelspinbox.h"

class BoardControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    BoardControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

private:
    QGridLayout * getLayoutWithScrollBar(QWidget * widget);

    MessageDispatcher * msgDisp = nullptr;
    std::vector<double> previousGateSpinBoxValues;
    std::vector<double> previousSourceSpinBoxValues;
    std::vector<NoWheelSpinBox*>  gateSpinBoxes;
    std::vector<NoWheelSpinBox*>  sourceSpinBoxes;

    std::vector<uint16_t> getChangedChannelIndexes(std::vector<NoWheelSpinBox*> spinBoxVector, std::vector<double> previousChannelValues);
    void onApplyButtonClicked();

signals:
    void sigGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);
};

#endif // BOARDCONTROLDOCKWIDGET_H
