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
    operationTitles[OperationRecordToFile] = "Record to file";
    operationTitles[OperationPlotToBigPlot] = "Expand selected traces";

    operationString.resize(OperationsNum);
    operationString[OperationTurnChannelsOnOff] = "Ch %1: On";
    operationString[OperationTurnStimulusOnOff] = "Ch %1: Stimulus on";
    operationString[OperationStartStopDigitalOffsetCompensation] = "Ch %1: Compensation active";
    operationString[OperationHoldingStimulus] = "NOT USED";
    operationString[OperationRecordToFile] = "Ch %1: Record this channel";
    operationString[OperationPlotToBigPlot] = "Ch %1: Expand trace";

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
    connect(applyBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onApplyButtonClicked);

    QGridLayout * applyBtnGridLayout = new QGridLayout;
    applyBtnGridLayout->addWidget(applyBtn, 0, 0, 1, 2);
    mainVl->addLayout(applyBtnGridLayout);

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

void ChannelControlDockWidget::onApplyButtonClicked() {
    int idx = operationCbx->currentIndex();
    switch (idx) {
    case OperationTurnChannelsOnOff: {
        QCheckBox * cb;
        vector<bool> values;
        vector<uint16_t> indexes;

        QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

        for (int i = 0; i<selectedIndexes.size(); i++) {
            cb = static_cast<QCheckBox *>(operationEdits[idx][i]);
            if (selectedIndexes.at(i)) {
                values.push_back(cb->isChecked());
                indexes.push_back(i);
            }
        }

        emit sigAppliedTurnChannelOnOff(indexes, values);
        break;
    }
    case OperationTurnStimulusOnOff:{
        QCheckBox * cb;
        vector<bool> values;
        vector<uint16_t> indexes;

        QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

        for (int i = 0; i<selectedIndexes.size(); i++) {
            cb = static_cast<QCheckBox *>(operationEdits[idx][i]);
            if (selectedIndexes.at(i)) {
                values.push_back(cb->isChecked());
                indexes.push_back(i);
            }
        }

        emit sigAppliedTurnStimulsOnOff(indexes, values);
        break;
    }
    case OperationStartStopDigitalOffsetCompensation:{
        QCheckBox * cb;
        vector<bool> values;
        vector<uint16_t> indexes;

        QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

        for (int i = 0; i<selectedIndexes.size(); i++) {
            cb = static_cast<QCheckBox *>(operationEdits[idx][i]);
            if (selectedIndexes.at(i)) {
                values.push_back(cb->isChecked());
                indexes.push_back(i);
            }
        }

        emit sigAppliedTurnDocOnOff(indexes, values);
        break;
    }
    case OperationHoldingStimulus:{
        SpinBoxWithChannel * vHoldSpinBox;
        vector<Measurement_t> values;
        vector<uint16_t> indexes;

        QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

        for (int i = 0; i<selectedIndexes.size(); i++) {
            vHoldSpinBox = static_cast<SpinBoxWithChannel *>(operationEdits[idx][i]);
            if (selectedIndexes.at(i)) {
                Measurement_t myMeasurementValue = {vHoldSpinBox->value(), UnitPfxMilli, "V"};
                values.push_back(myMeasurementValue);
                indexes.push_back(i);
            }
        }

        emit sigAppliedVoltageHoldValues(indexes, values);
        break;
    }
    case OperationRecordToFile:{
        /*! Nothing to be done, Apply replaced by start and stop recording */
        break;
    }
    case OperationPlotToBigPlot:{
        QCheckBox * cb;
        vector<bool> values;
        vector<uint16_t> indexes;

        QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

        for (int i = 0; i<selectedIndexes.size(); i++) {
            cb = static_cast<QCheckBox *>(operationEdits[idx][i]);
            if (selectedIndexes.at(i)) {
                values.push_back(cb->isChecked());
                indexes.push_back(i);
            }
        }

        emit sigAppliedPlotToBigPlot(indexes, values);
        break;
    }
    }

}


void ChannelControlDockWidget::onCheckAllButtonClicked() {
    QCheckBox * cb;
    vector<bool> values;
    vector<uint16_t> indexes;

    QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

    for (int i = 0; i<selectedIndexes.size(); i++) {
        cb = static_cast<QCheckBox *>(operationEdits[operationCbx->currentIndex()][i]);
        if (selectedIndexes.at(i)) {
            cb->setChecked(true);
        }
    }
}

void ChannelControlDockWidget::onUncheckAllButtonClicked() {
    QCheckBox * cb;
    vector<bool> values;
    vector<uint16_t> indexes;

    QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

    for (int i = 0; i<selectedIndexes.size(); i++) {
        cb = static_cast<QCheckBox *>(operationEdits[operationCbx->currentIndex()][i]);
        if (selectedIndexes.at(i)) {
            cb->setChecked(false);
        }
    }
}

void ChannelControlDockWidget::onSetAllButtonClicked() {
    SpinBoxWithChannel * spinBox;
    vector<bool> values;
    vector<uint16_t> indexes;

    QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

    for (int i = 0; i<selectedIndexes.size(); i++) {
        spinBox = static_cast<SpinBoxWithChannel *>(operationEdits[operationCbx->currentIndex()][i]);
        if (selectedIndexes.at(i)) {
            spinBox->setValue(this->setAllVholdSpinBox->value());
        }
    }
}

void ChannelControlDockWidget::onStartRecordingButtonClicked() {
    QCheckBox * cb;
    vector<bool> values;
    vector<uint16_t> indexes;
    bool isAtLeastOneChannelChecked = false;

    QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

    for (int i = 0; i<selectedIndexes.size(); i++) {
        cb = static_cast<QCheckBox *>(operationEdits[OperationRecordToFile][i]);
        if (selectedIndexes.at(i)) {
            values.push_back(cb->isChecked());
            indexes.push_back(i);
            if(cb->isChecked()){
                isAtLeastOneChannelChecked = true;
            }
        }
    }

    if(isAtLeastOneChannelChecked){
        operationWidgets[OperationRecordToFile]->setEnabled(false);
        QPixmap pixmapRecors("://imgs/recording protocol.png");
        QIcon recordIcon(pixmapRecors);
        startRecordingBtn->setIcon(recordIcon);
        emit sigStartRecording(indexes, values);
    } else{
        QString err = "Recording to file not possible";
        QString info = "No channel checked for recording";
        ErrorManager e(err, info);
    }
}

void ChannelControlDockWidget::onStopRecordingButtonClicked() {
    QPixmap pixmapRecors("://imgs/record protocol.png");
    QIcon recordIcon(pixmapRecors);
    startRecordingBtn->setIcon(recordIcon);

    QVector <bool> selectedIndexes = mDev->getSelectedChannelsIdxs();

    operationWidgets[OperationRecordToFile]->setEnabled(true);

    emit sigStopRecording();
}

void ChannelControlDockWidget::onSigRecording(bool state){
    if(state == false){
        this->stopRecordingBtn->click();
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
    case OperationRecordToFile:
    case OperationPlotToBigPlot:
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            QCheckBox * btn = new QCheckBox(QString(operationString[idx]).arg(channelIdx+1));
            btn->setChecked(false);
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
            MySpinBox * sbx = new MySpinBox;
            sbx->setSuffix(QString(" ") + unit);
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

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    scrollVl->addWidget(spacer);

    return operationWidgets[idx];
}

QWidget * ChannelControlDockWidget::createOperationButtonWidget(int idx) {
    operationButtonWidgets[idx] = new QWidget;

    switch (idx) {
    case OperationTurnChannelsOnOff:
    case OperationTurnStimulusOnOff:
    case OperationStartStopDigitalOffsetCompensation:
    case OperationPlotToBigPlot:{
        // mettere bottoni check/uncheck all
        QGridLayout* operationButtonGridLayout = new QGridLayout;
        operationButtonGridLayout->setContentsMargins(0, 0, 0, 2);
        operationButtonGridLayout->setSpacing(0);
        operationButtonWidgets[idx]->setLayout(operationButtonGridLayout);
        QPushButton* checkAllBtn = new QPushButton("Check all");
        QPushButton* uncheckAllBtn = new QPushButton("Uncheck all");
        connect(checkAllBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onCheckAllButtonClicked);
        connect(uncheckAllBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onUncheckAllButtonClicked);

        operationButtonGridLayout->addWidget(checkAllBtn, 0, 0);
        operationButtonGridLayout->addWidget(uncheckAllBtn, 0, 1);
        break;
    }
    case OperationHoldingStimulus:{
        QGridLayout* operationButtonGridLayout = new QGridLayout;
        operationButtonGridLayout->setContentsMargins(0, 0, 0, 2);
        operationButtonGridLayout->setSpacing(0);
        operationButtonWidgets[idx]->setLayout(operationButtonGridLayout);
        RangedMeasurement_t range;
        mDev->getVoltageHoldTunerFeatures(range);
        QString unit = QString().fromStdString(range.getFullUnit());
        MySpinBox * sbx = new MySpinBox;
        sbx->setSuffix(QString(" ") + unit);
        sbx->setRange(range.min, range.max);
        sbx->setValue(0.0);
        sbx->setDecimals(range.decimals());
        setAllVholdSpinBox = new SpinBoxWithChannel(QString(""), sbx);
        QPushButton* setAllBtn = new QPushButton("Set all channels");
        connect(setAllBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onSetAllButtonClicked);
        operationButtonGridLayout->addWidget(setAllVholdSpinBox, 0, 1);
        operationButtonGridLayout->addWidget(setAllBtn, 0, 0);
        break;
    }
    case OperationRecordToFile:{
        QGridLayout* operationButtonGridLayout = new QGridLayout;
        operationButtonGridLayout->setContentsMargins(0, 0, 0, 2);
        operationButtonGridLayout->setSpacing(0);
        operationButtonWidgets[idx]->setLayout(operationButtonGridLayout);
        QPushButton* checkAllBtn = new QPushButton("Check all");
        QPushButton* uncheckAllBtn = new QPushButton("Uncheck all");
        startRecordingBtn = new QPushButton("Start");
        stopRecordingBtn = new QPushButton("Stop");
        QPixmap pixmapRecors("://imgs/record protocol.png");
        QIcon recordIcon(pixmapRecors);
        startRecordingBtn->setIcon(recordIcon);
        QPixmap pixmapStop("://imgs/stop protocol.png");
        QIcon stopRecordIcon(pixmapStop);
        stopRecordingBtn->setIcon(stopRecordIcon);
        connect(checkAllBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onCheckAllButtonClicked);
        connect(uncheckAllBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onUncheckAllButtonClicked);
        connect(startRecordingBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onStartRecordingButtonClicked);
        connect(stopRecordingBtn, &QPushButton::clicked, this, &ChannelControlDockWidget::onStopRecordingButtonClicked);

        operationButtonGridLayout->addWidget(checkAllBtn, 0, 0);
        operationButtonGridLayout->addWidget(uncheckAllBtn, 0, 1);
        operationButtonGridLayout->addWidget(startRecordingBtn, 1, 0);
        operationButtonGridLayout->addWidget(stopRecordingBtn, 1, 1);
        break;
    }
    }

    return operationButtonWidgets[idx];
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

void ChannelControlDockWidget::onOperationSelected(int operationIdx) {
    for (int idx = 0; idx < OperationsNum; idx++) {
        operationWidgets[idx]->setVisible(false);
        operationButtonWidgets[idx]->setVisible(false);
    }
    operationWidgets[operationIdx]->setVisible(true);
    operationButtonWidgets[operationIdx]->setVisible(true);

    if (operationIdx == OperationRecordToFile){
        this->applyBtn->setEnabled(false);
    } else {
        this->applyBtn->setEnabled(true);
    }
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

double SpinBoxWithChannel::value() {
    return valueSbx->value();
}

void SpinBoxWithChannel::setValue(double value) {
    return valueSbx->setValue(value);
}
