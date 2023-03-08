#ifndef DEVICECONTROLDOCKWIDGET_H
#define DEVICECONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QGroupBox>
#include "model/modeldevice.h"
#include <QRadioButton>

class DeviceControlDockWidget : public QDockWidget{
public:
    DeviceControlDockWidget(ModelDevice * modelDevice);

public slots:
    void onUpdate();

private:
    ModelDevice * modelDevice;
    QGroupBox * vcCurrentRangesGroupBox;
    vector<QRadioButton *> vcCurrentRangesRadioButtons;
};

#endif // DEVICECONTROLDOCKWIDGET_H
