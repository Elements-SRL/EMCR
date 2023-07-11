#include "boardcontroldockwidget.h"

#include <vector>

#include <QScrollBar>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>

BoardControlDockWidget::BoardControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    int localNumOfBoards;
    msgDisp->getBoardsNumberFeatures(localNumOfBoards);

    QWidget * bigMainWg = new QWidget();
    bigMainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Board controls");
    setObjectName("boardControlsDw");

    QWidget * mainWg = new QWidget();

    this->setWidget(bigMainWg);
    QVBoxLayout * mainVl = new QVBoxLayout(bigMainWg);
    mainVl->setContentsMargins(0, 0, 0, 2);
    mainVl->setSpacing(1);
    mainVl->addWidget(mainWg);

    QGridLayout * mainGl = this->getLayoutWithScrollBar(mainWg);

    // Column captions
    mainGl->addWidget(new QLabel(" Board"), 0, 0);
    mainGl->addWidget(new QLabel("Gate"), 0, 1);
    mainGl->addWidget(new QLabel("Source"), 0, 2);
    int row = 2;
    mainGl->addWidget(new QLabel("ALL   "), row++, 0, Qt::AlignRight);
    for(row = 3; row < localNumOfBoards+3; row++){
        QString channelLabel= QString("%1   ").arg(row-2);
        mainGl->addWidget(new QLabel(channelLabel), row, 0, Qt::AlignRight);
    }

    RangedMeasurement_t gateRange;
    if (msgDisp->getGateVoltagesTunerFeatures(gateRange) == Success) {
        MySpinBox* gateSpinBox;
        QString gateUnit = QString().fromStdString(gateRange.getFullUnit());
        int row = 1;
        QPushButton * setAllBtn = new QPushButton("Set all boards");
        mainGl->addWidget(setAllBtn, row++, 1);
        gateSpinBox = new MySpinBox();
        gateSpinBox->setSuffix(QString(" ") + gateUnit);
        gateSpinBox->setRange(gateRange.min, gateRange.max);
        gateSpinBox->setValue(0.0);
        gateSpinBox->setDecimals(gateRange.decimals());
        mainGl->addWidget(gateSpinBox, row++, 1);
        connect(setAllBtn, &QPushButton::clicked, this, [=] () {
            for (auto sbx : gateSpinBoxes) {
                sbx->setValue(gateSpinBox->value());
            }
        });
        for(row = 3; row < localNumOfBoards+3; row++){
            gateSpinBox = new MySpinBox();
            gateSpinBox->setSuffix(QString(" ") + gateUnit);
            gateSpinBox->setRange(gateRange.min, gateRange.max);
            gateSpinBox->setValue(0.0);
            previousGateSpinBoxValues.push_back(gateSpinBox->value());
            gateSpinBox->setDecimals(gateRange.decimals());
            gateSpinBoxes.push_back(gateSpinBox);
            mainGl->addWidget(gateSpinBox, row, 1);
        }
    }

    RangedMeasurement_t sourceRange;
    if (msgDisp->getSourceVoltagesTunerFeatures(sourceRange) == Success) {
        MySpinBox* sourceSpinBox;
        QString sourceUnit = QString().fromStdString(sourceRange.getFullUnit());
        int row = 1;
        QPushButton * setAllBtn = new QPushButton("Set all boards");
        mainGl->addWidget(setAllBtn, row++, 2);
        sourceSpinBox = new MySpinBox();
        sourceSpinBox->setSuffix(QString(" ") + sourceUnit);
        sourceSpinBox->setRange(sourceRange.min, sourceRange.max);
        sourceSpinBox->setValue(0.0);
        sourceSpinBox->setDecimals(sourceRange.decimals());
        mainGl->addWidget(sourceSpinBox, row++, 2);
        connect(setAllBtn, &QPushButton::clicked, this, [=] () {
            for (auto sbx : sourceSpinBoxes) {
                sbx->setValue(sourceSpinBox->value());
            }
        });
        for(row = 3; row < localNumOfBoards+3; row++){
            sourceSpinBox = new MySpinBox();
            sourceSpinBox->setSuffix(QString(" ") + sourceUnit);
            sourceSpinBox->setRange(sourceRange.min, sourceRange.max);
            sourceSpinBox->setValue(0.0);
            previousSourceSpinBoxValues.push_back(sourceSpinBox->value());
            sourceSpinBox->setDecimals(sourceRange.decimals());
            sourceSpinBoxes.push_back(sourceSpinBox);
            mainGl->addWidget(sourceSpinBox, row, 2);

        }
    }

    QPushButton* applyButton = new QPushButton("Apply");
    mainVl->addWidget(applyButton);
    connect(applyButton, &QPushButton::clicked, this, &BoardControlDockWidget::onApplyButtonClicked);
}

QGridLayout * BoardControlDockWidget::getLayoutWithScrollBar(QWidget * widget) {
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

    QGridLayout * scrollHl = new QGridLayout;
    scrollHl->setContentsMargins(0, 0, 20, 0);
    scrollHl->setSpacing(1);
    scrollWg->setLayout(scrollHl);

    return scrollHl;
}

std::vector<uint16_t> BoardControlDockWidget::getChangedChannelIndexes(std::vector<MySpinBox*> spinBoxVector, std::vector<double> previousChannelValues){
    int j=0;
    std::vector<uint16_t> changedChannelIndexes;
    for(int i = 0; i < spinBoxVector.size(); i++){
        if(spinBoxVector[i]->value() != previousChannelValues[i]){
            changedChannelIndexes.push_back((uint16_t)i);
            j++;
        }
    }
    return changedChannelIndexes;
}

void BoardControlDockWidget::onApplyButtonClicked(){
    RangedMeasurement_t range;
    msgDisp->getGateVoltagesTunerFeatures(range);
    std::vector<Measurement_t> gateChangedVoltages;
    std::vector<uint16_t> gateChangedChannelIndexes = this->getChangedChannelIndexes(this->gateSpinBoxes, this->previousGateSpinBoxValues);
    gateChangedVoltages.resize(gateChangedChannelIndexes.size());
    for(int i = 0; i < gateChangedChannelIndexes.size(); i++){
        double changedValue;
        changedValue = this->gateSpinBoxes[gateChangedChannelIndexes[i]]->value();
        gateChangedVoltages[i] = {changedValue, range.prefix, range.unit};
    }

    msgDisp->getSourceVoltagesTunerFeatures(range);
    std::vector<Measurement_t> sourceChangedVoltages;
    std::vector<uint16_t> sourceChangedChannelIndexes = this->getChangedChannelIndexes(this->sourceSpinBoxes, this->previousSourceSpinBoxValues);
    sourceChangedVoltages.resize(sourceChangedChannelIndexes.size());
    for(int i = 0; i < sourceChangedChannelIndexes.size(); i++){
        double changedValue;
        changedValue = this->sourceSpinBoxes[sourceChangedChannelIndexes[i]]->value();
        sourceChangedVoltages[i] = {changedValue, range.prefix, range.unit};
    }

    emit sigGateSourceVoltagesApplied(gateChangedChannelIndexes, gateChangedVoltages, sourceChangedChannelIndexes, sourceChangedVoltages);

    /*!update prevoius gate e source values */
    for(int i = 0; i < this->previousGateSpinBoxValues.size(); i++){
        previousGateSpinBoxValues[i] = this->gateSpinBoxes[i]->value();
    }

    for(int i = 0; i < this->previousSourceSpinBoxValues.size(); i++){
        previousSourceSpinBoxValues[i] = this->sourceSpinBoxes[i]->value();
    }
}

