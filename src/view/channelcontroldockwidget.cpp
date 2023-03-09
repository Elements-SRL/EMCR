#include "channelcontroldockwidget.h"

#include <QScrollBar>
#include <QScrollArea>

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

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    mainVl->addWidget(spacer);

    connect(operationCbx, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &ChannelControlDockWidget::onOperationSelected);
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
            QCheckBox * btn = new QCheckBox(QString("Ch %1 On").arg(channelIdx+1));
            btn->setChecked(true);
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

            SpinBoxWithChannel * widget = new SpinBoxWithChannel(channelIdx, sbx);

            scrollVl->addWidget(widget);
            operationEdits[idx][channelIdx] = widget;
        }
        break;
    }
    }
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
        operationWidgets[idx]->setVisible(idx == operationIdx);
    }
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
