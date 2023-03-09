#include "channelcontroldockwidget.h"

#include <QScrollBar>
#include <QScrollArea>
#include <QPushButton>
#include <iostream>

ChannelControlDockWidget::ChannelControlDockWidget(ModelDevice * mDev, QWidget * parent) :
    QDockWidget(parent),
    mDev(mDev) {

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Channels controls");

    this->setWidget(mainWg);

    QVBoxLayout * mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(1);
    mainWg->setLayout(mainVl);

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);

    operationTitles.resize(OperationsNum);
    operationTitles[OperationTurnChannelsOnOff] = "Turn channels on/off";
    operationTitles[OperationTurnStimulusOnOff] = "Turn stimulus on/off";
    operationTitles[OperationStartStopDigitalOffsetCompensation] = "Start/stop digital offset compensation";
    operationTitles[OperationHoldingStimulus] = "Holding stimulus";

    operationString.resize(OperationsNum);
    operationString[OperationTurnChannelsOnOff] = "Ch %1: On";
    operationString[OperationTurnStimulusOnOff] = "Ch %1: Stimulus on";
    operationString[OperationStartStopDigitalOffsetCompensation] = "Ch %1: Compensation active";
    operationString[OperationHoldingStimulus] = "NOT USED";

    operationCbx = new QComboBox;
    mainVl->addWidget(operationCbx);

    operationWidgets.resize(OperationsNum);
    operationEdits.resize(OperationsNum);
    for (int idx = 0; idx < OperationsNum; idx++) {
        operationCbx->addItem(operationTitles[idx]);
        operationWidgets[idx] = this->createOperationWidget(idx);
        operationWidgets[idx]->setVisible(idx == 0);
        mainVl->addWidget(operationWidgets[idx]);
    }

    QPushButton * applyBtn = new QPushButton("APPLY");
    connect(applyBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onApplyButtonClicked);
    mainVl->addWidget(applyBtn);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    mainVl->addWidget(spacer);

    connect(operationCbx, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &ChannelControlDockWidget::onOperationSelected);
}

void ChannelControlDockWidget::onApplyButtonClicked() {
    int idx = operationCbx->currentIndex();
    switch (idx) {
    case OperationTurnChannelsOnOff: {
        cout<< "turnChannelsOnOff" << endl;
        QCheckBox * cb;
        vector<bool> values;
        vector<uint16_t> indexes;

        QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

//        for (auto oe: operationEdits[idx]){
//            cb = static_cast<QCheckBox *>(oe);
//            if(cb->isChecked()){
//                values.push_back(cb->isChecked());
//                indexes.push_back(operationEdits[idx].indexOf(oe));
//            }
//        }
//        cout <<indexes[0] << endl;

        for (int i = 0; i<selectedIndexes.size(); i++) {
            cb = static_cast<QCheckBox *>(operationEdits[idx][i]);
            if (selectedIndexes.at(i)) {
                values.push_back(cb->isChecked());
                indexes.push_back(i);
            }
        }
        cout << indexes.size() << "  " <<values.size() <<endl;
        break;
    }
    case OperationTurnStimulusOnOff:
        cout<< "OperationTurnStimulusOnOff" << endl;
//        TODO
        break;
    case OperationStartStopDigitalOffsetCompensation:
        cout<< "OperationStartStopDigitalOffsetCompensation" << endl;
//        TODO
        break;
    case OperationHoldingStimulus:
        cout<< "OperationHoldingStimulus" << endl;
//        TODO
        break;

    }
}

void ChannelControlDockWidget::onUpdate() {
    QVector <bool> selectedChannels = mDev->getSelectedChannelsIdxs();
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        for (int idx = 0; idx < OperationsNum; idx++) {
            operationEdits[idx][channelIdx]->setVisible(selectedChannels[channelIdx]);
        }
    }
}

QWidget * ChannelControlDockWidget::createOperationWidget(int idx) {
    operationWidgets[idx] = new QWidget;
    QVBoxLayout * scrollVl = getLayoutWithScrollBar(operationWidgets[idx]);

    operationEdits[idx].resize(currentChannelsNum);
    switch (idx) {
    case OperationTurnChannelsOnOff:
    case OperationTurnStimulusOnOff:
    case OperationStartStopDigitalOffsetCompensation:
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            QCheckBox * btn = new QCheckBox(QString(operationString[idx]).arg(channelIdx+1));
            btn->setChecked(true);
            btn->setVisible(mDev->getSelectedChannelsIdxs()[channelIdx]);
            scrollVl->addWidget(btn);
            operationEdits[idx][channelIdx] = btn;
        }
        break;

    case OperationHoldingStimulus: {
        RangedMeasurement_t range;
        mDev->getVoltageHoldTunerFeatures(range);
        QString unit = QString().fromStdString(range.getFullUnit());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            QDoubleSpinBox * sbx = new QDoubleSpinBox;
            sbx->setSuffix(unit);
            sbx->setRange(range.min, range.max);
            sbx->setValue(0.0);
            sbx->setDecimals(range.decimals());
            SpinBoxWithChannel * widget = new SpinBoxWithChannel(channelIdx, sbx);
            widget->setVisible(mDev->getSelectedChannelsIdxs()[channelIdx]);

            scrollVl->addWidget(widget);
            operationEdits[idx][channelIdx] = widget;
        }
        break;
    }
    }

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    scrollVl->addWidget(spacer);

    return operationWidgets[idx];
}

QVBoxLayout * ChannelControlDockWidget::getLayoutWithScrollBar(QWidget * widget) {
    QVBoxLayout * vl = new QVBoxLayout;
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(1);
    widget->setLayout(vl);

    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    vl->addWidget(scrollArea);

    QWidget * scrollWg = new QWidget;
    scrollArea->setWidget(scrollWg);

    QVBoxLayout * scrollVl = new QVBoxLayout;
    scrollVl->setContentsMargins(0, 0, 0, 0);
    scrollVl->setSpacing(1);
    scrollWg->setLayout(scrollVl);

    return scrollVl;
}

void ChannelControlDockWidget::onOperationSelected(int operationIdx) {
    for (int idx = 0; idx < OperationsNum; idx++) {
        operationWidgets[idx]->setVisible(false);
    }
    operationWidgets[operationIdx]->setVisible(true);
}

SpinBoxWithChannel::SpinBoxWithChannel(int idx, QDoubleSpinBox * sbx) :
    valueSbx(sbx) {

    QHBoxLayout * hl = new QHBoxLayout;
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(1);
    this->setLayout(hl);

    channelLbl = new QLabel(QString().fromStdString("Ch %1").arg(idx+1));
    hl->addWidget(channelLbl);

    hl->addWidget(sbx);
}

double SpinBoxWithChannel::value() {
    return valueSbx->value();
}
