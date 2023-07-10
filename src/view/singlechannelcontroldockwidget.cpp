#include "singlechannelcontroldockwidget.h"

#include <QScrollBar>
#include <QScrollArea>

SingleChannelControlDockWidget::SingleChannelControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setWindowTitle("Single channel controls");
    setObjectName("singleChannelControlsDw");
    setWidget(mainWg);

    QVBoxLayout * mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(1);
    mainWg->setLayout(mainVl);

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);

    operationTitles.resize(OperationsNum);
    operationTitles[OperationHoldingStimulus] = "Holding stimulus";

    operationString.resize(OperationsNum);
    operationString[OperationHoldingStimulus] = "NOT USED";

    operationCbx = new QComboBox;
    mainVl->addWidget(operationCbx);

    operationWidgets.resize(OperationsNum);
    operationButtonWidgets.resize(OperationsNum);
    operationEdits.resize(OperationsNum);
    for (int idx = 0; idx < OperationsNum; idx++) {
        operationCbx->addItem(operationTitles[idx]);
        operationWidgets[idx] = this->createOperationWidget(idx);
        operationWidgets[idx]->setVisible(idx == 0);
        mainVl->addWidget(operationWidgets[idx]);
        operationButtonWidgets[idx] = this->createOperationButtonWidget(idx);
        operationButtonWidgets[idx]->setVisible(idx == 0);
        mainVl->addWidget(operationButtonWidgets[idx]);
    }

//    QPushButton * applyBtn = new QPushButton("Apply");
    this->applyBtn = new QPushButton("Apply");
    connect(applyBtn, &QPushButton::clicked, this, QOverload <> ::of(&SingleChannelControlDockWidget::onApplyButtonClicked));

    QGridLayout * applyBtnGridLayout = new QGridLayout;
    applyBtnGridLayout->addWidget(applyBtn, 0, 0, 1, 2);
    mainVl->addLayout(applyBtnGridLayout);

    connect(operationCbx, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &SingleChannelControlDockWidget::onOperationSelected);

    this->installEventFilter(this);
}

void SingleChannelControlDockWidget::onUpdate() {
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        for (int idx = 0; idx < OperationsNum; idx++) {
            operationEdits[idx][channelIdx]->setVisible(selectedChannels[channelIdx]);
        }
    }
}

void SingleChannelControlDockWidget::onApplyButtonClicked() {
    int idx = operationCbx->currentIndex();
    this->onApplyButtonClicked(idx, false);
}

void SingleChannelControlDockWidget::onApplyButtonClicked(int operationIdx, bool applyAll) {
    std::vector <bool> selectedChannels(currentChannelsNum, true);
    if (!applyAll) {
        msgDisp->getSelectedChannels(selectedChannels);
    }

    switch (operationIdx) {
    case OperationHoldingStimulus:{
        SpinBoxWithChannel * sbx;
        std::vector<Measurement_t> values;
        std::vector<uint16_t> indexes;

        for (int i = 0; i<selectedChannels.size(); i++) {
            sbx = static_cast<SpinBoxWithChannel *>(operationEdits[operationIdx][i]);
            if (selectedChannels.at(i)) {
                Measurement_t myMeasurementValue = {sbx->getSpinBox()->value(), holdingTunerRange.prefix, holdingTunerRange.unit};
                values.push_back(myMeasurementValue);
                indexes.push_back(i);
            }
        }

        emit sigAppliedHoldValues(indexes, values);
        break;
    }
    }
}

void SingleChannelControlDockWidget::onCheckAllButtonClicked() {
    QCheckBox * cb;
    std::vector<bool> values;
    std::vector<uint16_t> indexes;
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int i = 0; i<selectedChannels.size(); i++) {
        cb = static_cast<QCheckBox *>(operationEdits[operationCbx->currentIndex()][i]);
        if (selectedChannels.at(i)) {
            cb->setChecked(true);
        }
    }
}

void SingleChannelControlDockWidget::onUncheckAllButtonClicked() {
    QCheckBox * cb;
    std::vector<bool> values;
    std::vector<uint16_t> indexes;
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int i = 0; i<selectedChannels.size(); i++) {
        cb = static_cast<QCheckBox *>(operationEdits[operationCbx->currentIndex()][i]);
        if (selectedChannels.at(i)) {
            cb->setChecked(false);
        }
    }
}

void SingleChannelControlDockWidget::onSetAllButtonClicked() {
    SpinBoxWithChannel * spinBox;
    std::vector<bool> values;
    std::vector<uint16_t> indexes;
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int i = 0; i<selectedChannels.size(); i++) {
        spinBox = static_cast<SpinBoxWithChannel *>(operationEdits[operationCbx->currentIndex()][i]);
        if (selectedChannels.at(i)) {
            spinBox->getSpinBox()->setValue(this->setAllVholdSpinBox->getSpinBox()->value());
        }
    }
}

void SingleChannelControlDockWidget::onVcVoltageRangeSelected(int idx) {
    std::vector <RangedMeasurement_t> ranges;
    msgDisp->getVoltageHoldTunerFeatures(ranges);
    holdingTunerRange = ranges[idx];
    QString unit = QString().fromStdString(holdingTunerRange.getFullUnit());
    setAllChannelsSbx->setSuffix(QString(" ") + unit);
    setAllChannelsSbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
    setAllChannelsSbx->setDecimals(holdingTunerRange.decimals());
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
        sbx->setSuffix(QString(" ") + unit);
        sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
        sbx->setDecimals(holdingTunerRange.decimals());
    }

    this->onApplyButtonClicked(OperationHoldingStimulus, true);
}

void SingleChannelControlDockWidget::onCcCurrentRangeSelected(int idx) {
    std::vector <RangedMeasurement_t> ranges;
    msgDisp->getCurrentHoldTunerFeatures(ranges);
    holdingTunerRange = ranges[idx];
    QString unit = QString().fromStdString(holdingTunerRange.getFullUnit());
    setAllChannelsSbx->setSuffix(QString(" ") + unit);
    setAllChannelsSbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
    setAllChannelsSbx->setDecimals(holdingTunerRange.decimals());
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
        sbx->setSuffix(QString(" ") + unit);
        sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
        sbx->setDecimals(holdingTunerRange.decimals());
    }

    this->onApplyButtonClicked(OperationHoldingStimulus, true);
}

bool SingleChannelControlDockWidget::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast <QKeyEvent *> (event);
        if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
            this->onApplyButtonClicked();
        }
    }
    return QObject::eventFilter(obj, event);
}

QWidget * SingleChannelControlDockWidget::createOperationWidget(int idx) {
    operationWidgets[idx] = new QWidget;
    QVBoxLayout * scrollVl = getLayoutWithScrollBar(operationWidgets[idx]);

    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);

    operationEdits[idx].resize(currentChannelsNum);
    switch (idx) {
    case OperationHoldingStimulus: {
        std::vector <RangedMeasurement_t> ranges;
        /*! \todo FCON magari non è necessariamente disponibile il DAC di tensione, bensì quello di corrente */
        msgDisp->getVoltageHoldTunerFeatures(ranges);
        QString unit = QString().fromStdString(ranges[0].getFullUnit());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            MySpinBox * sbx = new MySpinBox;
            sbx->setSuffix(QString(" ") + unit);
            sbx->setRange(ranges[0].min, ranges[0].max);
            sbx->setValue(0.0);
            sbx->setDecimals(ranges[0].decimals());
            SpinBoxWithChannel * widget = new SpinBoxWithChannel(channelIdx, sbx);
            widget->setVisible(selectedChannels[channelIdx]);

            scrollVl->addWidget(widget);
            operationEdits[idx][channelIdx] = widget;
        }
        break;
    }
    }

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    scrollVl->addWidget(spacer);

    return operationWidgets[idx];
}

QWidget * SingleChannelControlDockWidget::createOperationButtonWidget(int idx) {
    operationButtonWidgets[idx] = new QWidget;

    switch (idx) {
    case OperationHoldingStimulus:{
        QGridLayout* operationButtonGridLayout = new QGridLayout;
        operationButtonGridLayout->setContentsMargins(0, 0, 0, 2);
        operationButtonGridLayout->setSpacing(0);
        operationButtonWidgets[idx]->setLayout(operationButtonGridLayout);
        std::vector <RangedMeasurement_t> ranges;
        /*! \todo FCON magari non è necessariamente disponibile il DAC di tensione, bensì quello di corrente */
        msgDisp->getVoltageHoldTunerFeatures(ranges);
        QString unit = QString().fromStdString(ranges[0].getFullUnit());
        setAllChannelsSbx = new MySpinBox;
        setAllChannelsSbx->setSuffix(QString(" ") + unit);
        setAllChannelsSbx->setRange(ranges[0].min, ranges[0].max); /*! \todo questo range dovrebbe cambiare quando cambia il range del DAC */
        setAllChannelsSbx->setValue(0.0);
        setAllChannelsSbx->setDecimals(ranges[0].decimals());
        setAllVholdSpinBox = new SpinBoxWithChannel(QString(""), setAllChannelsSbx);
        QPushButton* setAllBtn = new QPushButton("Set all channels");
        connect(setAllBtn, &QPushButton::clicked, this, &SingleChannelControlDockWidget::onSetAllButtonClicked);
        operationButtonGridLayout->addWidget(setAllVholdSpinBox, 0, 1);
        operationButtonGridLayout->addWidget(setAllBtn, 0, 0);
        break;
    }
    }

    return operationButtonWidgets[idx];
}

QVBoxLayout * SingleChannelControlDockWidget::getLayoutWithScrollBar(QWidget * widget) {
    QVBoxLayout * vl = new QVBoxLayout;
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(1);
    widget->setLayout(vl);

    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    vl->addWidget(scrollArea);

    QWidget * scrollWg = new QWidget;
    scrollArea->setWidget(scrollWg);

    QVBoxLayout * scrollVl = new QVBoxLayout;
    scrollVl->setContentsMargins(0, 0, 0, 0);
    scrollVl->setSpacing(1);
    scrollWg->setLayout(scrollVl);

    return scrollVl;
}

void SingleChannelControlDockWidget::onOperationSelected(int operationIdx) {
    for (int idx = 0; idx < OperationsNum; idx++) {
        operationWidgets[idx]->setVisible(false);
        operationButtonWidgets[idx]->setVisible(false);
    }
    operationWidgets[operationIdx]->setVisible(true);
    operationButtonWidgets[operationIdx]->setVisible(true);
}

SpinBoxWithChannel::SpinBoxWithChannel(int idx, MySpinBox * sbx) :
    SpinBoxWithChannel(QString().fromStdString("Ch %1").arg(idx+1), sbx) {

}

SpinBoxWithChannel::SpinBoxWithChannel(QString title, MySpinBox * sbx) :
    valueSbx(sbx) {

    QHBoxLayout * hl = new QHBoxLayout;
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(1);
    this->setLayout(hl);

    channelLbl = new QLabel(title);
    hl->addWidget(channelLbl);

    hl->addWidget(sbx);
}

MySpinBox * SpinBoxWithChannel::getSpinBox() {
    return valueSbx;
}
