#define TITLE "Controls"

#define CURRENT_RANGE "Current Ranges"
#define STIMULUS_FILTER "Readout Filters"

#include "devicecontroldockwidget.h"
#include <QVBoxLayout>

DeviceControlDockWidget::DeviceControlDockWidget(ModelDevice *modelDevice): QDockWidget()
{
    this->modelDevice = modelDevice;

    vector <RangedMeasurement_t> vcCurrentRanges;
    modelDevice->getVcCurrentRangesFeatures(vcCurrentRanges);

    QWidget *window = new QWidget;
    this->setWidget(window);
    QVBoxLayout * vLayout = new QVBoxLayout(window);

    this->setWindowTitle(TITLE);

    this->vcCurrentRangesGroupBox = new QGroupBox(CURRENT_RANGE);

    QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();

    vLayout->addWidget(this->vcCurrentRangesGroupBox);
    for (auto rm : vcCurrentRanges){
        QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
        radioButtonsBoxLayout->addWidget(qrb);
        this->vcCurrentRangesRadioButtons.push_back(qrb);
        vLayout->addWidget(qrb);
    }
    if (this->vcCurrentRangesRadioButtons.size() > 0) {
        this->vcCurrentRangesRadioButtons[0]->setChecked(true);
    }
    this->vcCurrentRangesGroupBox->setLayout(radioButtonsBoxLayout);

}
