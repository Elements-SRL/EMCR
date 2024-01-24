#include "singlechannelcontroldockwidget.h"

#include <QScrollBar>
#include <QScrollArea>

SingleChannelControlDockWidget::SingleChannelControlDockWidget(ApplicationStatus * appStatus, QWidget * parent) :
    QDockWidget(parent),
    appStatus(appStatus) {

    QWidget * mainWg = new QWidget(parent);
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setWindowTitle("Single channel controls");
    setObjectName("singleChannelControlsDw");
    setWidget(mainWg);

    QVBoxLayout * mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(1);
    mainWg->setLayout(mainVl);
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();

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

    auto msgDisp = appStatus->getMessageDispatcher();
    std::vector <RangedMeasurement_t> ranges;
    if (msgDisp->getVoltageHoldTunerFeatures(ranges) == Success) {
        buildOperation(mainVl, OperationHoldingStimulus, true);
        anyOperationActive = true;

    } else {
        QStandardItemModel * model = qobject_cast <QStandardItemModel *> (operationCbx->model());
        QStandardItem * item = model->item(OperationHoldingStimulus);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    if (msgDisp->getLiquidJunctionRangesFeatures(ranges) == Success) {
        buildOperation(mainVl, OperationLiquidJunction);
        anyOperationActive = true;

    } else {
        QStandardItemModel * model = qobject_cast <QStandardItemModel *> (operationCbx->model());
        QStandardItem * item = model->item(OperationLiquidJunction);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    if (msgDisp->hasStimulusHalf() == Success) {
        buildOperation(mainVl, OperationStimulusHalf);
        anyOperationActive = true;

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
    if (operationWidget == nullptr) {
        return;
    }
    auto operationButtonWidget = this->createOperationButtonWidget(operationType);
    if (operationButtonWidget == nullptr) {
        return;
    }
    operationWidget->setVisible(visibility);
    operationButtonWidget->setVisible(visibility);
    operationWidgets[operationType] = operationWidget;
    operationButtonWidgets[operationType] = operationButtonWidget;
    layout->addWidget(operationWidget);
    layout->addWidget(operationButtonWidget);
}

void SingleChannelControlDockWidget::onUpdate() {
    if (anyOperationActive) {
        std::vector <bool> selectedChannels = appStatus->getSelectedChannels();
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            operationEdits[operationCbx->currentIndex()][channelIdx]->setVisible(selectedChannels[channelIdx]);
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
        auto msgDisp = appStatus->getMessageDispatcher();
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
    auto msgDisp = appStatus->getMessageDispatcher();
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
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getSelectedChannels(selectedChannels);
    for (int i = 0; i < selectedChannels.size(); i++) {
        spinBox = static_cast <SpinBoxWithChannel *> (operationEdits[operationCbx->currentIndex()][i]);
        if (selectedChannels.at(i)) {
            spinBox->getSpinBox()->setValue(setAllWidgets[operationCbx->currentIndex()]->getSpinBox()->value());
        }
    }
}

void SingleChannelControlDockWidget::onVcVoltageRangeSelected(int idx) {
    std::vector <RangedMeasurement_t> ranges;
    auto msgDisp = appStatus->getMessageDispatcher();
    QString unit = "";
    if (msgDisp->getVoltageHoldTunerFeatures(ranges) == Success) {
        holdingTunerRange = ranges[idx];
        unit = QString().fromStdString(holdingTunerRange.getFullUnit());
        setAllChannelsSbxs[OperationHoldingStimulus]->setSuffix(QString(" ") + unit);
        setAllChannelsSbxs[OperationHoldingStimulus]->setRange(holdingTunerRange.min, holdingTunerRange.max);
        setAllChannelsSbxs[OperationHoldingStimulus]->setDecimals(holdingTunerRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
            sbx->setSuffix(QString(" ") + unit);
            sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
            sbx->setDecimals(holdingTunerRange.decimals());
        }
    }

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

void SingleChannelControlDockWidget::onCcCurrentRangeSelected(int idx) {
    std::vector <RangedMeasurement_t> ranges;
    auto msgDisp = appStatus->getMessageDispatcher();
    QString unit = "";
    if (msgDisp->getCurrentHoldTunerFeatures(ranges) == Success) {
        holdingTunerRange = ranges[idx];
        unit = QString().fromStdString(holdingTunerRange.getFullUnit());
        setAllChannelsSbxs[OperationHoldingStimulus]->setSuffix(QString(" ") + unit);
        setAllChannelsSbxs[OperationHoldingStimulus]->setRange(holdingTunerRange.min, holdingTunerRange.max);
        setAllChannelsSbxs[OperationHoldingStimulus]->setDecimals(holdingTunerRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            MySpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
            sbx->setSuffix(QString(" ") + unit);
            sbx->setRange(holdingTunerRange.min, holdingTunerRange.max);
            sbx->setDecimals(holdingTunerRange.decimals());
        }
    }

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

    auto selectedChannels = appStatus->getSelectedChannels();
    auto msgDisp = appStatus->getMessageDispatcher();

    operationEdits[idx].resize(currentChannelsNum);
    std::vector <RangedMeasurement_t> ranges;

    switch (idx) {
    case OperationHoldingStimulus:
        if (msgDisp->getVoltageHoldTunerFeatures(ranges) != Success) {
            return nullptr;
        }
        break;

    case OperationLiquidJunction:
        if (msgDisp->getLiquidJunctionRangesFeatures(ranges) != Success) {
            return nullptr;
        }
        break;

    case OperationStimulusHalf:
        if (msgDisp->getVoltageHalfFeatures(ranges) != Success) {
            return nullptr;
        }
        break;
    }

    QString unit = QString().fromStdString(ranges[0].getFullUnit());
    auto names = appStatus->getNames();
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        MySpinBox * sbx = new MySpinBox;
        sbx->setSuffix(QString(" ") + unit);
        sbx->setRange(ranges[0].min, ranges[0].max);
        sbx->setValue(0.0);
        sbx->setDecimals(ranges[0].decimals());
        SpinBoxWithChannel * widget = new SpinBoxWithChannel(names[channelIdx], sbx);
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
    auto msgDisp = appStatus->getMessageDispatcher();

    switch (idx) {
    case OperationHoldingStimulus:
        if (msgDisp->getVoltageHoldTunerFeatures(ranges) != Success) {
            return nullptr;
        }
        break;

    case OperationLiquidJunction:
        if (msgDisp->getLiquidJunctionRangesFeatures(ranges) != Success) {
            return nullptr;
        }
        break;

    case OperationStimulusHalf:
        if (msgDisp->getVoltageHalfFeatures(ranges) != Success) {
            return nullptr;
        }
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
    setAllWidgets[idx] = new SpinBoxWithChannel("", setAllChannelsSbxs[idx]);
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
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        MySpinBox * sbx2 = static_cast <SpinBoxWithChannel *> (operationEdits[OperationLiquidJunction][channelIdx])->getSpinBox();
        sbx2->setValue(voltages[channelIdx].value);
    }
}

void SingleChannelControlDockWidget::onBoardMappingsLoaded(){
    auto names = appStatus->getNames();
    for (int opIdx=0; opIdx<OperationsNum; opIdx++) {
         if (operationEdits[opIdx].size() > 0) {
            for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                auto sbwc = static_cast <SpinBoxWithChannel *> (operationEdits[opIdx][channelIdx]);
                sbwc->setName(names[channelIdx]);
                sbwc->setVisible(false);
            }
        }
    }
}

SpinBoxWithChannel::SpinBoxWithChannel(std::string title, MySpinBox * sbx) :
    valueSbx(sbx) {

    QHBoxLayout * hl = new QHBoxLayout;
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(1);
    this->setLayout(hl);

    channelLbl = new QLabel(QString::fromStdString(title));
    hl->addWidget(channelLbl);

    hl->addWidget(sbx);
}

MySpinBox * SpinBoxWithChannel::getSpinBox() {
    return valueSbx;
}

void SpinBoxWithChannel::setName(std::string name){
    channelLbl->setText(QString::fromStdString(name));
}

