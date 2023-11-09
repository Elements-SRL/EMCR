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
    operationTitles[OperationLiquidJunction] = "Offset compensation";
    operationTitles[OperationStimulusHalf] = "Stimulus half";

    operationCbx = new QComboBox;
    mainVl->addWidget(operationCbx);

    setAllChannelsSbxs.resize(OperationsNum);
    setAllWidgets.resize(OperationsNum);

    operationWidgets.resize(OperationsNum);
    operationButtonWidgets.resize(OperationsNum);
    operationEdits.resize(OperationsNum);

    for (int idx = 0; idx < OperationsNum; idx++) {
        operationCbx->addItem(operationTitles[idx]);
    }

    buildOperation(mainVl, OperationHoldingStimulus, true);
    buildOperation(mainVl, OperationLiquidJunction);

    //    change this to OperationStimulusHalf
    if (msgDisp->hasStimulusHalf() == Success){
        buildOperation(mainVl, OperationStimulusHalf);
    } else {
        QStandardItemModel * model = qobject_cast <QStandardItemModel *> (operationCbx->model());
        QStandardItem * item = model->item(OperationStimulusHalf);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    applyBtn = new QPushButton("Apply");
    connect(applyBtn, &QPushButton::clicked, this, QOverload <> ::of(&SingleChannelControlDockWidget::onApplyButtonClicked));

    QGridLayout * applyBtnGridLayout = new QGridLayout;
    applyBtnGridLayout->addWidget(applyBtn, 0, 0, 1, 2);
    mainVl->addLayout(applyBtnGridLayout);

    connect(operationCbx, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &SingleChannelControlDockWidget::onOperationSelected);

    this->installEventFilter(this);
}

void SingleChannelControlDockWidget::buildOperation(QLayout * layout, int operationType, bool visibility){
    auto operationWidget = this->createOperationWidget(operationType);
    auto operationButtonWidget = this->createOperationButtonWidget(operationType);
    operationWidget->setVisible(visibility);
    operationButtonWidget->setVisible(visibility);
    operationWidgets[operationType] = operationWidget;
    operationButtonWidgets[operationType] = operationButtonWidget;
    layout->addWidget(operationWidget);
    layout->addWidget(operationButtonWidget);
}

void SingleChannelControlDockWidget::onUpdate() {
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        operationEdits[operationCbx->currentIndex()][channelIdx]->setVisible(selectedChannels[channelIdx]);
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
    SpinBoxWithChannel * sbx;
    std::vector<Measurement_t> values;
    std::vector<uint16_t> indexes;
    auto range = operationIdx == OperationLiquidJunction ? liquidJunctionRange : holdingTunerRange;
    for (int i = 0; i < selectedChannels.size(); i++) {
        sbx = static_cast <SpinBoxWithChannel *> (operationEdits[operationIdx][i]);
        if (selectedChannels.at(i)) {
            Measurement_t m = {sbx->getSpinBox()->value(), range.prefix, range.unit};
            values.push_back(m);
            indexes.push_back(i);
        }
    }
    switch (operationIdx) {
    case OperationHoldingStimulus:{
        emit sigAppliedHoldValues(indexes, values);
        break;
    }
    case OperationLiquidJunction:{
        emit sigLiquidJunctionValues(indexes, values);
        break;
    }
    case OperationStimulusHalf:{
        emit sigAppliedStimHalfValues(indexes, values);
        break;
    }
    }
}

//TODO Questo non viene mai chiamato
void SingleChannelControlDockWidget::onAllButtonClicked(bool newState) {
    QCheckBox * cb;
    std::vector<bool> values;
    std::vector<uint16_t> indexes;
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int i = 0; i<selectedChannels.size(); i++) {
        cb = static_cast<QCheckBox *>(operationEdits[operationCbx->currentIndex()][i]);
        if (selectedChannels.at(i)) {
            cb->setChecked(newState);
        }
    }
}

void SingleChannelControlDockWidget::onSetAllButtonClicked() {
    SpinBoxWithChannel * spinBox;
    std::vector<bool> values;
    std::vector<uint16_t> indexes;
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int i = 0; i < selectedChannels.size(); i++) {
        spinBox = static_cast <SpinBoxWithChannel *> (operationEdits[operationCbx->currentIndex()][i]);
        if (selectedChannels.at(i)) {
            spinBox->getSpinBox()->setValue(setAllWidgets[operationCbx->currentIndex()]->getSpinBox()->value());
        }
    }
}
/** \todo MPAC recheck se devi modificare anche questo per Stimulus Half */
void SingleChannelControlDockWidget::onVcVoltageRangeSelected(int idx) {
    std::vector <RangedMeasurement_t> ranges;
    msgDisp->getVoltageHoldTunerFeatures(ranges);
    holdingTunerRange = ranges[idx];
    QString unit = QString().fromStdString(holdingTunerRange.getFullUnit());
    setAllChannelsSbxs[OperationHoldingStimulus]->setSuffix(QString(" ") + unit);
    setAllChannelsSbxs[OperationHoldingStimulus]->setRange(holdingTunerRange.min, holdingTunerRange.max);
    setAllChannelsSbxs[OperationHoldingStimulus]->setDecimals(holdingTunerRange.decimals());
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
        sbx->setSuffix(QString(" ") + unit);
        sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
        sbx->setDecimals(holdingTunerRange.decimals());
    }

    /** \todo MPAC, recheck this section is new*/
    if (setAllChannelsSbxs[OperationStimulusHalf] != nullptr) {
        setAllChannelsSbxs[OperationStimulusHalf]->setSuffix(QString(" ") + unit);
        setAllChannelsSbxs[OperationStimulusHalf]->setRange(holdingTunerRange.min, holdingTunerRange.max);
        setAllChannelsSbxs[OperationStimulusHalf]->setDecimals(holdingTunerRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationStimulusHalf][channelIdx])->getSpinBox();
            sbx->setSuffix(QString(" ") + unit);
            sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
            sbx->setDecimals(holdingTunerRange.decimals());
        }
    }

    if (msgDisp->getLiquidJunctionRangesFeatures(ranges) == Success) {
        liquidJunctionRange = ranges[idx];
        unit = QString().fromStdString(liquidJunctionRange.getFullUnit());
        setAllChannelsSbxs[OperationLiquidJunction]->setSuffix(QString(" ") + unit);
        setAllChannelsSbxs[OperationLiquidJunction]->setRange(liquidJunctionRange.min, liquidJunctionRange.max);
        setAllChannelsSbxs[OperationLiquidJunction]->setDecimals(liquidJunctionRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationLiquidJunction][channelIdx])->getSpinBox();
            sbx->setSuffix(QString(" ") + unit);
            sbx->setRange(liquidJunctionRange.min, liquidJunctionRange.max);
            sbx->setDecimals(liquidJunctionRange.decimals());
        }
    }
}
/** \todo MPAC recheck se devi modificare anche questo per Stimulus Half */
void SingleChannelControlDockWidget::onCcCurrentRangeSelected(int idx) {
    std::vector <RangedMeasurement_t> ranges;
    msgDisp->getCurrentHoldTunerFeatures(ranges);
    holdingTunerRange = ranges[idx];
    QString unit = QString().fromStdString(holdingTunerRange.getFullUnit());
    setAllChannelsSbxs[OperationHoldingStimulus]->setSuffix(QString(" ") + unit);
    setAllChannelsSbxs[OperationHoldingStimulus]->setRange(holdingTunerRange.min, holdingTunerRange.max);
    setAllChannelsSbxs[OperationHoldingStimulus]->setDecimals(holdingTunerRange.decimals());
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
        sbx->setSuffix(QString(" ") + unit);
        sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
        sbx->setDecimals(holdingTunerRange.decimals());
    }

    /** \todo MPAC, recheck this section is new*/
    setAllChannelsSbxs[OperationStimulusHalf]->setSuffix(QString(" ") + unit);
    setAllChannelsSbxs[OperationStimulusHalf]->setRange(holdingTunerRange.min, holdingTunerRange.max);
    setAllChannelsSbxs[OperationStimulusHalf]->setDecimals(holdingTunerRange.decimals());
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationStimulusHalf][channelIdx])->getSpinBox();
        sbx->setSuffix(QString(" ") + unit);
        sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
        sbx->setDecimals(holdingTunerRange.decimals());
    }
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
    std::vector <RangedMeasurement_t> ranges;

    switch (idx) {
    case OperationHoldingStimulus:
        msgDisp->getVoltageHoldTunerFeatures(ranges);
        break;

    case OperationLiquidJunction:
        msgDisp->getLiquidJunctionRangesFeatures(ranges);
        break;

    case OperationStimulusHalf:
        /** \todo recheck, qui ci sono ancora gli hold, non gli half, sdal momento che non ne conosco le fatures*/
        msgDisp->getVoltageHoldTunerFeatures(ranges);
        break;
    }

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

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    scrollVl->addWidget(spacer);

    return operationWidgets[idx];
}

QWidget * SingleChannelControlDockWidget::createOperationButtonWidget(int idx) {
    operationButtonWidgets[idx] = new QWidget;
    std::vector <RangedMeasurement_t> ranges;

    switch (idx) {
    case OperationHoldingStimulus:
        msgDisp->getVoltageHoldTunerFeatures(ranges);
        break;

    case OperationLiquidJunction:
        msgDisp->getLiquidJunctionRangesFeatures(ranges);
        break;

    case OperationStimulusHalf:
        /** \todo recheck, qui ci sono ancora gli hold, non gli half, sdal momento che non ne conosco le fatures*/
        msgDisp->getVoltageHoldTunerFeatures(ranges);
        break;
    }

    QGridLayout * operationButtonGridLayout = new QGridLayout;
    operationButtonGridLayout->setContentsMargins(0, 0, 0, 2);
    operationButtonGridLayout->setSpacing(0);
    operationButtonWidgets[idx]->setLayout(operationButtonGridLayout);
    QString unit = QString().fromStdString(ranges[0].getFullUnit());
    setAllChannelsSbxs[idx] = new MySpinBox;
    setAllChannelsSbxs[idx]->setSuffix(QString(" ") + unit);
    setAllChannelsSbxs[idx]->setRange(ranges[0].min, ranges[0].max);
    setAllChannelsSbxs[idx]->setValue(0.0);
    setAllChannelsSbxs[idx]->setDecimals(ranges[0].decimals());
    setAllWidgets[idx] = new SpinBoxWithChannel(QString(""), setAllChannelsSbxs[idx]);
    QPushButton * setAllBtn = new QPushButton("Set all channels");
    connect(setAllBtn, &QPushButton::clicked, this, &SingleChannelControlDockWidget::onSetAllButtonClicked);
    operationButtonGridLayout->addWidget(setAllWidgets[idx], 0, 1);
    operationButtonGridLayout->addWidget(setAllBtn, 0, 0);

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
        if (operationWidgets[idx] != nullptr){
            operationWidgets[idx]->setVisible(false);
            operationButtonWidgets[idx]->setVisible(false);
        }
    }
    operationWidgets[operationIdx]->setVisible(true);
    operationButtonWidgets[operationIdx]->setVisible(true);

    this->onUpdate();
}

void SingleChannelControlDockWidget::setLiquidJunctionVoltages(std::vector <Measurement_t> voltages){
//    5 / 0;
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
