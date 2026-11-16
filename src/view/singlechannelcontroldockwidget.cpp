#include "singlechannelcontroldockwidget.h"

#include <QScrollBar>
#include <QScrollArea>

SingleChannelControlDockWidget::SingleChannelControlDockWidget(ApplicationStatus * appStatus, QWidget * parent) :
    QDockWidget(parent),
    appStatus(appStatus) {

    this->setVisible(false);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setWindowTitle("Single channel controls");
    setObjectName("singleChannelControlsDw");

    QWidget * centralWidget = new QWidget();
    centralWidget->setObjectName("singleChannelCentralWidget");
    QVBoxLayout* externalLayout = new QVBoxLayout(centralWidget);
    externalLayout->setContentsMargins(0, 0, 0, 0);
    externalLayout->setSpacing(0);

    this->setWidget(centralWidget);

    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();

    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getStimulusBlockSize(stimulusBlockSize);

    operationTitles.resize(OperationsNum);
    operationTitles[OperationHoldingStimulus] = "Holding stimulus";
    operationTitles[OperationOffsetRecalibration] = "Offset recalibration";
    operationTitles[OperationLiquidJunction] = "Liquid junction compensation";
    operationTitles[OperationStimulusHalf] = "Stimulus half";
    operationTitles[OperationOffsetTracking] = "Offset tracking";
    operationTitles[OperationInitialStimulusRamp] = "Ramp initial stimulus";
    operationTitles[OperationFinalStimulusRamp] = "Ramp final stimulus";
    operationTitles[OperationDurationRamp] = "Ramp duration";

    QFrame* operationContainer = new QFrame();
    operationContainer->setObjectName("operationContainer");
    operationContainer->setProperty("floatingAware", true); // border depends on the dock floating state, see MainWindow::setDockWidget
    QHBoxLayout* operationContainerLayout = new QHBoxLayout(operationContainer);
    operationContainerLayout->setContentsMargins(8, 4, 8, 4);

    operationCbx = new QComboBox;
    operationCbx->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    operationCbx->setObjectName("operationCbx");
    operationContainerLayout->addWidget(operationCbx);
    externalLayout->addWidget(operationContainer);

    QFrame * operationHeader = new QFrame();
    operationHeader->setObjectName("operationHeader");
    operationHeader->setProperty("floatingAware", true); // border depends on the dock floating state, see MainWindow::setDockWidget
    QHBoxLayout* operationHeaderLayout = new QHBoxLayout(operationHeader);
    operationHeaderLayout->setContentsMargins(10, 10, 48, 10);

    auto channelHeader = new QLabel("Channel");
    auto operationHeaderL = new QLabel("Value");

    operationHeaderLayout->addWidget(channelHeader);
    operationHeaderLayout->addStretch();
    operationHeaderLayout->addWidget(operationHeaderL);
    externalLayout->addWidget(operationHeader);

    setAllChannelsSbxs.resize(OperationsNum);
    setAllWidgets.resize(OperationsNum);

    operationWidgets.resize(OperationsNum);
    operationButtonWidgets.resize(OperationsNum);
    operationEdits.resize(OperationsNum);

    for (int idx = 0; idx < OperationsNum; idx++) {
        operationCbx->addItem(operationTitles[idx]);
    }

    std::vector <RangedMeasurement_t> ranges;
    if (msgDisp->getVoltageHoldTunerFeatures(ranges) == Success || msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) == Success) {
        buildOperation(externalLayout, OperationHoldingStimulus, true);
    }
    else {
        QStandardItemModel * model = qobject_cast <QStandardItemModel *> (operationCbx->model());
        QStandardItem * item = model->item(OperationHoldingStimulus);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    buildOperation(externalLayout, OperationOffsetRecalibration);

    if (msgDisp->getLiquidJunctionRangesFeatures(ranges) == Success) {
        buildOperation(externalLayout, OperationLiquidJunction);
    }
    else {
        QStandardItemModel * model = qobject_cast <QStandardItemModel *> (operationCbx->model());
        QStandardItem * item = model->item(OperationLiquidJunction);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    if (msgDisp->hasStimulusHalf() == Success) {
        buildOperation(externalLayout, OperationStimulusHalf);
    }
    else {
        QStandardItemModel * model = qobject_cast <QStandardItemModel *> (operationCbx->model());
        QStandardItem * item = model->item(OperationStimulusHalf);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    buildOperation(externalLayout, OperationOffsetTracking);

    if (msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) == Success) {
        buildOperation(externalLayout, OperationInitialStimulusRamp);
        buildOperation(externalLayout, OperationFinalStimulusRamp);
        buildOperation(externalLayout, OperationDurationRamp);
    }
    else {
        QStandardItemModel * model = qobject_cast <QStandardItemModel *> (operationCbx->model());
        QStandardItem * item = model->item(OperationInitialStimulusRamp);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item = model->item(OperationFinalStimulusRamp);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item = model->item(OperationDurationRamp);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    applyBtn = new QPushButton("Apply");
    connect(applyBtn, &QPushButton::clicked, this, QOverload <> ::of(&SingleChannelControlDockWidget::onApplyButtonClicked));

    QFrame* footerFrame = new QFrame();
    footerFrame->setObjectName("customFooter");
    footerFrame->setProperty("floatingAware", true); // border depends on the dock floating state, see MainWindow::setDockWidget
    QHBoxLayout* applyBtnLayout = new QHBoxLayout(footerFrame);
    applyBtnLayout->setContentsMargins(8, 8, 8, 8);

    applyBtnLayout->addWidget(applyBtn);
    externalLayout->addWidget(footerFrame);

    connect(operationCbx, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &SingleChannelControlDockWidget::onOperationSelected);

    this->installEventFilter(this);
    widgetInitialized = true;
}

void SingleChannelControlDockWidget::buildOperation(QLayout * layout, Operations_t operationType, bool visibility){
    auto operationWidget = this->createOperationWidget(operationType);
    if (operationWidget == nullptr) {
        return;
    }
    auto operationButtonWidget = this->createOperationButtonWidget(operationType);
    if (operationButtonWidget == nullptr) {
        return;
    }
    operationWidget->setVisible(visibility);
    operationButtonWidget->setVisible(visibility && appStatus->getSelectedChannelsIndexes().size() > 1);
    operationWidgets[operationType] = operationWidget;
    operationButtonWidgets[operationType] = operationButtonWidget;
    layout->addWidget(operationWidget);
    layout->addWidget(operationButtonWidget);
}

void SingleChannelControlDockWidget::onUpdate() {
    if (widgetInitialized) {
        std::vector <bool> selectedChannels = appStatus->getSelectedChannels();
        if (operationCbx->currentIndex() == OperationHoldingStimulus && stimulusBlockSize > 1) {
            for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx += stimulusBlockSize) {
                bool visible = false;
                for (uint32_t offset = 0; offset < stimulusBlockSize; offset++) {
                    visible |= selectedChannels[channelIdx+offset];
                }
                operationEdits[operationCbx->currentIndex()][channelIdx]->setVisible(visible);
            }
        }
        else {
            for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                operationEdits[operationCbx->currentIndex()][channelIdx]->setVisible(selectedChannels[channelIdx]);
            }
        }
        operationButtonWidgets[operationCbx->currentIndex()]->setVisible(appStatus->getSelectedChannelsIndexes().size() > 1);
    }
}

void SingleChannelControlDockWidget::onApplyButtonClicked() {
    int idx = operationCbx->currentIndex();
    this->onApplyButtonClicked(idx, false);
}

void SingleChannelControlDockWidget::onApplyButtonClicked(int operationIdx, bool applyAll) {
    std::vector <bool> selectedChannels(currentChannelsNum, true);
    if (!applyAll) {
        selectedChannels = appStatus->getSelectedChannels();
    }
    SpinBoxWithChannel * sbx;
    std::vector <Measurement_t> values;
    std::vector <Measurement_t> values2;
    std::vector <Measurement_t> values3;
    std::vector <uint16_t> indexes;
    std::vector <RangedMeasurement_t> range;
    switch (operationIdx) {
    case OperationHoldingStimulus:
    case OperationStimulusHalf:
    case OperationInitialStimulusRamp:
    case OperationFinalStimulusRamp:
    case OperationDurationRamp:
        range = holdingTunerRange;
        break;

    case OperationOffsetRecalibration:
        range = offsetRecalibrationRange;
        break;

    case OperationLiquidJunction:
        range.resize(currentChannelsNum);
        std::fill(range.begin(), range.end(), liquidJunctionRange);
        break;

    case OperationOffsetTracking:
        range = offsetRecalibrationRange;
        break;
    }
    if (operationIdx < OperationInitialStimulusRamp) {
        if (operationIdx == OperationHoldingStimulus && stimulusBlockSize > 1) {
            for (int i = 0; i < selectedChannels.size(); i++) {
                if (selectedChannels.at(i)) {
                    int realIdx = (i/stimulusBlockSize)*stimulusBlockSize;
                    sbx = static_cast <SpinBoxWithChannel *> (operationEdits[operationIdx][realIdx]);
                    Measurement_t m = {sbx->getSpinBox()->value(), range[realIdx].prefix, range[realIdx].unit};
                    values.push_back(m);
                    indexes.push_back(i);
                }
            }
        }
        else {
            for (int i = 0; i < selectedChannels.size(); i++) {
                if (selectedChannels.at(i)) {
                    sbx = static_cast <SpinBoxWithChannel *> (operationEdits[operationIdx][i]);
                    Measurement_t m = {sbx->getSpinBox()->value(), range[i].prefix, range[i].unit};
                    values.push_back(m);
                    indexes.push_back(i);
                }
            }
        }
    }
    else {
        for (int i = 0; i < selectedChannels.size(); i++) {
            if (selectedChannels.at(i)) {
                sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationInitialStimulusRamp][i]);
                Measurement_t m = {sbx->getSpinBox()->value(), range[i].prefix, range[i].unit};
                values.push_back(m);

                sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationFinalStimulusRamp][i]);
                m = {sbx->getSpinBox()->value(), range[i].prefix, range[i].unit};
                values2.push_back(m);

                sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationDurationRamp][i]);
                m = {sbx->getSpinBox()->value(), stimulusDurationRange.prefix, stimulusDurationRange.unit};
                values3.push_back(m);

                indexes.push_back(i);
            }
        }
    }
    switch (operationIdx) {
    case OperationHoldingStimulus:
        emit sigAppliedHoldValues(indexes, values);
        break;

    case OperationOffsetRecalibration:
        emit sigAppliedOffsetRecalibration(indexes, values);
        break;

    case OperationLiquidJunction:
        emit sigLiquidJunctionValues(indexes, values);
        break;

    case OperationStimulusHalf:
        emit sigAppliedStimHalfValues(indexes, values);
        break;

    case OperationOffsetTracking:
        emit sigAppliedOffsetTracking(indexes, values);
        break;

    case OperationInitialStimulusRamp:
    case OperationFinalStimulusRamp:
    case OperationDurationRamp:
        emit sigAppliedRamp(indexes, values, values2, values3);
        break;
    }
}

void SingleChannelControlDockWidget::onSetAllButtonClicked() {
    SpinBoxWithChannel * spinBox;
    std::vector<bool> values;
    std::vector<uint16_t> indexes;
    std::vector <bool> selectedChannels = appStatus->getSelectedChannels();
    for (int i = 0; i < selectedChannels.size(); i++) {
        if (operationCbx->currentIndex() == OperationHoldingStimulus && stimulusBlockSize > 1) {
            spinBox = static_cast <SpinBoxWithChannel *> (operationEdits[operationCbx->currentIndex()][i]);
            if (selectedChannels.at(i) && ((i % stimulusBlockSize) == 0) && spinBox != nullptr) {
                spinBox->getSpinBox()->setValue(setAllWidgets[operationCbx->currentIndex()]->getSpinBox()->value());
            }
        }
        else {
            spinBox = static_cast <SpinBoxWithChannel *> (operationEdits[operationCbx->currentIndex()][i]);
            if (selectedChannels.at(i) && spinBox != nullptr) {
                spinBox->getSpinBox()->setValue(setAllWidgets[operationCbx->currentIndex()]->getSpinBox()->value());
            }
        }
    }
    this->onApplyButtonClicked();
}

void SingleChannelControlDockWidget::onVcVoltageRangeSelected() {
    std::vector <RangedMeasurement_t> ranges;
    auto msgDisp = appStatus->getMessageDispatcher();
    auto maxRange = this->getAppStatus()->getMaxVoltageRange();
    /*! \todo FCON possibili inconsistenza di unità tra range del tasto set all e tasti per i singoli canali */
    if (msgDisp->getVoltageHoldTunerFeatures(ranges) == Success) {
        holdingTunerRange = this->getAppStatus()->getVoltageRanges();
        setAllChannelsSbxs[OperationHoldingStimulus]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationHoldingStimulus]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx += stimulusBlockSize) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
            sbx->setRange(holdingTunerRange[channelIdx].min, holdingTunerRange[channelIdx].max);
            sbx->setDecimals(holdingTunerRange[channelIdx].decimals());
        }
    }

    if (setAllChannelsSbxs[OperationStimulusHalf] != nullptr) {
        setAllChannelsSbxs[OperationStimulusHalf]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationStimulusHalf]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationStimulusHalf][channelIdx])->getSpinBox();
            sbx->setRange(holdingTunerRange[channelIdx].min, holdingTunerRange[channelIdx].max);
            sbx->setDecimals(holdingTunerRange[channelIdx].decimals());
        }
    }

    if (msgDisp->getLiquidJunctionRangesFeatures(ranges) == Success) {
        /*! \todo FCON stiamo dando per scontato che il range per la liquid junction sia uguale al  range del DAC */
        liquidJunctionRange = maxRange;
        setAllChannelsSbxs[OperationLiquidJunction]->setRange(liquidJunctionRange.min, liquidJunctionRange.max);
        setAllChannelsSbxs[OperationLiquidJunction]->setDecimals(liquidJunctionRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationLiquidJunction][channelIdx])->getSpinBox();
            sbx->setRange(liquidJunctionRange.min, liquidJunctionRange.max);
            sbx->setDecimals(liquidJunctionRange.decimals());
        }
    }

    if (msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) == Success) {
        holdingTunerRange = this->getAppStatus()->getVoltageRanges();
        setAllChannelsSbxs[OperationInitialStimulusRamp]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationInitialStimulusRamp]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationInitialStimulusRamp][channelIdx])->getSpinBox();
            sbx->setRange(holdingTunerRange[channelIdx].min, holdingTunerRange[channelIdx].max);
            sbx->setDecimals(holdingTunerRange[channelIdx].decimals());
        }
        setAllChannelsSbxs[OperationFinalStimulusRamp]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationFinalStimulusRamp]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationFinalStimulusRamp][channelIdx])->getSpinBox();
            sbx->setRange(holdingTunerRange[channelIdx].min, holdingTunerRange[channelIdx].max);
            sbx->setDecimals(holdingTunerRange[channelIdx].decimals());
        }
    }
}

void SingleChannelControlDockWidget::onVcCurrentRangeSelected() {
    std::vector <RangedMeasurement_t> ranges;
    auto msgDisp = appStatus->getMessageDispatcher();
    uint16_t _;
    auto maxRange = this->getAppStatus()->getMaxCurrentRange();
    if (msgDisp->getVCCurrentRanges(ranges, _) == Success) {
        offsetRecalibrationRange = this->getAppStatus()->getCurrentRanges();
        setAllChannelsSbxs[OperationOffsetRecalibration]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationOffsetRecalibration]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationOffsetRecalibration][channelIdx])->getSpinBox();
            sbx->setRange(offsetRecalibrationRange[channelIdx].min, offsetRecalibrationRange[channelIdx].max);
            sbx->setDecimals(offsetRecalibrationRange[channelIdx].decimals());
        }
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationOffsetTracking][channelIdx])->getSpinBox();
            sbx->setRange(offsetRecalibrationRange[channelIdx].min, offsetRecalibrationRange[channelIdx].max);
            sbx->setDecimals(offsetRecalibrationRange[channelIdx].decimals());
        }
    }
}

void SingleChannelControlDockWidget::onCcCurrentRangeSelected() {
    std::vector <RangedMeasurement_t> ranges;
    auto msgDisp = appStatus->getMessageDispatcher();
    auto maxRange = this->getAppStatus()->getMaxCurrentRange();
    if (msgDisp->getCurrentHoldTunerFeatures(ranges) == Success) {
        holdingTunerRange = this->getAppStatus()->getCurrentRanges();
        setAllChannelsSbxs[OperationHoldingStimulus]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationHoldingStimulus]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx += stimulusBlockSize) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationHoldingStimulus][channelIdx])->getSpinBox();
            sbx->setRange(holdingTunerRange[channelIdx].min, holdingTunerRange[channelIdx].max);
            sbx->setDecimals(holdingTunerRange[channelIdx].decimals());
        }
    }

    if (setAllChannelsSbxs[OperationStimulusHalf] != nullptr) {
        setAllChannelsSbxs[OperationStimulusHalf]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationStimulusHalf]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationStimulusHalf][channelIdx])->getSpinBox();
            sbx->setRange(holdingTunerRange[channelIdx].min, holdingTunerRange[channelIdx].max);
            sbx->setDecimals(holdingTunerRange[channelIdx].decimals());
        }
    }
}

void SingleChannelControlDockWidget::onCcVoltageRangeSelected() {
    std::vector <RangedMeasurement_t> ranges;
    auto msgDisp = appStatus->getMessageDispatcher();
    uint16_t _;
    auto maxRange = this->getAppStatus()->getMaxVoltageRange();
    if (msgDisp->getCCVoltageRanges(ranges, _) == Success) {
        offsetRecalibrationRange = this->getAppStatus()->getVoltageRanges();
        setAllChannelsSbxs[OperationOffsetRecalibration]->setRange(maxRange.min, maxRange.max);
        setAllChannelsSbxs[OperationOffsetRecalibration]->setDecimals(maxRange.decimals());
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationOffsetRecalibration][channelIdx])->getSpinBox();
            sbx->setRange(offsetRecalibrationRange[channelIdx].min, offsetRecalibrationRange[channelIdx].max);
            sbx->setDecimals(offsetRecalibrationRange[channelIdx].decimals());
        }
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = static_cast <SpinBoxWithChannel *> (operationEdits[OperationOffsetTracking][channelIdx])->getSpinBox();
            sbx->setRange(offsetRecalibrationRange[channelIdx].min, offsetRecalibrationRange[channelIdx].max);
            sbx->setDecimals(offsetRecalibrationRange[channelIdx].decimals());
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

ApplicationStatus * SingleChannelControlDockWidget::getAppStatus() {
    return appStatus;
}

QWidget * SingleChannelControlDockWidget::createOperationWidget(int idx) {
    operationWidgets[idx] = new QWidget;
    QVBoxLayout * scrollVl = getLayoutWithScrollBar(operationWidgets[idx]);

    auto selectedChannels = appStatus->getSelectedChannels();
    auto msgDisp = appStatus->getMessageDispatcher();

    operationEdits[idx].resize(currentChannelsNum);
    std::vector <RangedMeasurement_t> ranges;
    uint16_t _;

    switch (idx) {
    case OperationHoldingStimulus:
        if (msgDisp->getVoltageHoldTunerFeatures(ranges) != Success && msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) != Success) {
            return nullptr;
        }
        break;

    case OperationOffsetRecalibration:
        if (msgDisp->getVCCurrentRanges(ranges, _) != Success) {
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

    case OperationOffsetTracking:
        if (msgDisp->getVCCurrentRanges(ranges, _) != Success) {
            return nullptr;
        }
        break;

    case OperationInitialStimulusRamp:
    case OperationFinalStimulusRamp:
        if (msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) != Success) {
            return nullptr;
        }
        break;

    case OperationDurationRamp:
        if (msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) != Success) {
            return nullptr;
        }
        std::fill(ranges.begin(), ranges.end(), stimulusDurationRange);
        break;
    }

    QString unit = QString().fromStdString(ranges[0].getFullUnit());
    auto names = appStatus->getNames();
    if (idx == OperationHoldingStimulus && stimulusBlockSize > 1) {
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx += stimulusBlockSize) {
            NoWheelSpinBox * sbx = new NoWheelSpinBox;
            sbx->setRange(ranges[0].min, ranges[0].max);
            sbx->setValue(0.0);
            sbx->setDecimals(ranges[0].decimals());
            SpinBoxWithChannel * widget = new SpinBoxWithChannel(names[channelIdx] + " - " + names[channelIdx+stimulusBlockSize-1], sbx, &unit);
            widget->setVisible(selectedChannels[channelIdx]);

            scrollVl->addWidget(widget);
            operationEdits[idx][channelIdx] = widget;
        }
    }
    else {
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            NoWheelSpinBox * sbx = new NoWheelSpinBox;
            sbx->setRange(ranges[0].min, ranges[0].max);
            sbx->setValue(0.0);
            sbx->setDecimals(ranges[0].decimals());
            SpinBoxWithChannel * widget = new SpinBoxWithChannel(names[channelIdx], sbx, &unit);
            widget->setVisible(selectedChannels[channelIdx]);

            scrollVl->addWidget(widget);
            operationEdits[idx][channelIdx] = widget;
        }
    }

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    scrollVl->addWidget(spacer);

    return operationWidgets[idx];
}

QWidget * SingleChannelControlDockWidget::createOperationButtonWidget(int idx) {
    operationButtonWidgets[idx] = new QWidget;
    operationButtonWidgets[idx]->setObjectName("operationButtonWidget");
    operationButtonWidgets[idx]->setProperty("floatingAware", true); // border depends on the dock floating state, see MainWindow::setDockWidget
    std::vector <RangedMeasurement_t> ranges;
    uint16_t _;
    auto msgDisp = appStatus->getMessageDispatcher();

    switch (idx) {
    case OperationHoldingStimulus:
        if (msgDisp->getVoltageHoldTunerFeatures(ranges) != Success && msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) != Success) {
            return nullptr;
        }
        break;

    case OperationOffsetRecalibration:
        if (msgDisp->getVCCurrentRanges(ranges, _) != Success) {
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

    case OperationOffsetTracking:
        if (msgDisp->getVCCurrentRanges(ranges, _) != Success) {
            return nullptr;
        }
        break;

    case OperationInitialStimulusRamp:
    case OperationFinalStimulusRamp:
        if (msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) != Success) {
            return nullptr;
        }
        break;

    case OperationDurationRamp:
        if (msgDisp->getVoltageRampTunerFeatures(ranges, stimulusDurationRange) != Success) {
            return nullptr;
        }
        std::fill(ranges.begin(), ranges.end(), stimulusDurationRange);
        break;
    }

    QGridLayout * operationButtonGridLayout = new QGridLayout;
    operationButtonGridLayout->setContentsMargins(8, 8, 8, 8);
    operationButtonGridLayout->setSpacing(2);
    operationButtonWidgets[idx]->setLayout(operationButtonGridLayout);
    QString unit = QString().fromStdString(ranges[0].getFullUnit());
    setAllChannelsSbxs[idx] = new NoWheelSpinBox;
    setAllChannelsSbxs[idx]->setRange(ranges[0].min, ranges[0].max);
    setAllChannelsSbxs[idx]->setValue(0.0);
    setAllChannelsSbxs[idx]->setDecimals(ranges[0].decimals());
    setAllWidgets[idx] = new SpinBoxWithChannel("", setAllChannelsSbxs[idx], &unit);
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
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    vl->addWidget(scrollArea);

    QFrame * scrollWg = new QFrame;
    scrollArea->setWidget(scrollWg);
    scrollWg->setObjectName("singleChannelControlsScrollContainer");
    scrollWg->setProperty("floatingAware", true); // border depends on the dock floating state, see MainWindow::setDockWidget

    QVBoxLayout * scrollVl = new QVBoxLayout;
    scrollVl->setContentsMargins(2, 8, 2, 0);
    scrollVl->setSpacing(8);
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
    operationButtonWidgets[operationIdx]->setVisible(appStatus->getSelectedChannelsIndexes().size() > 1);

    this->onUpdate();
}

void SingleChannelControlDockWidget::setOffsetRecalibrationValues(std::vector <Measurement_t> values) {
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        NoWheelSpinBox * sbx2 = static_cast <SpinBoxWithChannel *> (operationEdits[OperationOffsetRecalibration][channelIdx])->getSpinBox();
        sbx2->setValue(values[channelIdx].value);
    }
}

void SingleChannelControlDockWidget::setLiquidJunctionVoltages(std::vector <Measurement_t> voltages) {
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        NoWheelSpinBox * sbx2 = static_cast <SpinBoxWithChannel *> (operationEdits[OperationLiquidJunction][channelIdx])->getSpinBox();
        sbx2->setValue(voltages[channelIdx].value);
    }
}

void SingleChannelControlDockWidget::setOffsetTrackingValues(std::vector <Measurement_t> values) {
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        NoWheelSpinBox * sbx2 = static_cast <SpinBoxWithChannel *> (operationEdits[OperationOffsetTracking][channelIdx])->getSpinBox();
        sbx2->setValue(values[channelIdx].value);
    }
}

void SingleChannelControlDockWidget::onBoardMappingsLoaded() {
    auto names = appStatus->getNames();
    for (int opIdx = 0; opIdx < OperationsNum; opIdx++) {
         if (operationEdits[opIdx].size() > 0) {
            for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                auto sbwc = static_cast <SpinBoxWithChannel *> (operationEdits[opIdx][channelIdx]);
                sbwc->setName(names[channelIdx]);
                sbwc->setVisible(false);
            }
        }
    }
}

SpinBoxWithChannel::SpinBoxWithChannel(std::string title, NoWheelSpinBox * sbx, const QString * sbxUnit) :
    valueSbx(sbx) {

    QGridLayout* grid = new QGridLayout();
    grid->setContentsMargins(8, 4, 8, 4);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(3);
    this->setLayout(grid);

    channelLbl = new QLabel(QString::fromStdString(title));
    unitMeasure = new QLabel(*sbxUnit);

    grid->addWidget(channelLbl, 0, 0);
    grid->addWidget(sbx, 0, 1);
    grid->addWidget(unitMeasure, 0, 2);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);
    grid->setColumnStretch(2, 0);

    unitMeasure->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    sbx->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sbx->setMinimumWidth(sbx->sizeHint().width());

}

NoWheelSpinBox * SpinBoxWithChannel::getSpinBox() {
    return valueSbx;
}

void SpinBoxWithChannel::setName(std::string name){
    channelLbl->setText(QString::fromStdString(name));
}
