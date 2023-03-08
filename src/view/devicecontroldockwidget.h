#ifndef DEVICECONTROLDOCKWIDGET_H
#define DEVICECONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QGroupBox>
#include "model/modeldevice.h"
#include <QRadioButton>

class DeviceControlDockWidget : public QDockWidget{
    Q_OBJECT

public:
    DeviceControlDockWidget(ModelDevice * modelDevice);

private:
    ModelDevice * modelDevice;
    QGroupBox * vcCurrentRangesGroupBox;
    vector<QRadioButton *> vcCurrentRangesRadioButtons;

signals:
    void sigVcCurrentRangeSelected(int idx);
};

#endif // DEVICECONTROLDOCKWIDGET_H
