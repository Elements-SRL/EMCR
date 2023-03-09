#include "boardcontroldockwidget.h"

#include <vector>
#include <QScrollBar>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>

BoardControlDockWidget::BoardControlDockWidget(ModelDevice * mDev, QWidget * parent) :
    QDockWidget(parent),
    mDev(mDev) {

    int localNumOfBoards;
    mDev->getBoardsNumberFeatures(localNumOfBoards);

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Board controls");

    this->setWidget(mainWg);

    QGridLayout * mainGridLayout = this->getLayoutWithScrollBar(mainWg);
    mainWg->setLayout(mainGridLayout);

    // Column captions
    mainGridLayout->addWidget(new QLabel(" Channel"), 0, 0);
    mainGridLayout->addWidget(new QLabel("Gate"), 0, 1);
    mainGridLayout->addWidget(new QLabel("Source"), 0, 2);

    for(int i = 1; i <= localNumOfBoards; i++){
        QString channelLabel= QString("    %1").arg(i);
        mainGridLayout->addWidget(new QLabel(channelLabel),i, 0);

    }

    MySpinBox* gateSpinBox;
    RangedMeasurement_t gateRange;
    mDev->getGateVoltagesTunerFeatures(gateRange);
    QString gateUnit = QString().fromStdString(gateRange.getFullUnit());
    for(int i = 1; i <= localNumOfBoards; i++){
        gateSpinBox = new MySpinBox();
//        gateSpinBox->setFocusPolicy( Qt::StrongFocus );
        gateSpinBox->setSuffix(QString(" ") + gateUnit);
        gateSpinBox->setRange(gateRange.min, gateRange.max);
        gateSpinBox->setValue(0.0);
        this->previousGateSpinBoxValues.push_back(gateSpinBox->value());
        gateSpinBox->setDecimals(gateRange.decimals());
        this->gateSpinBoxes.push_back(gateSpinBox);
        mainGridLayout->addWidget(gateSpinBox,i, 1);

    }


    MySpinBox* sourceSpinBox;
    RangedMeasurement_t sourceRange;
    mDev->getSourceVoltagesTunerFeatures(sourceRange);
    QString sourceUnit = QString().fromStdString(sourceRange.getFullUnit());
    for(int i = 1; i <= localNumOfBoards; i++){
        sourceSpinBox = new MySpinBox();
//        sourceSpinBox->setFocusPolicy( Qt::StrongFocus );
        sourceSpinBox->setSuffix(QString(" ") + sourceUnit);
        sourceSpinBox->setRange(sourceRange.min, sourceRange.max);
        sourceSpinBox->setValue(0.0);
//        sourceSpinBox->setGroupSeparatorShown(true);
        this->previousSourceSpinBoxValues.push_back(sourceSpinBox->value());
        sourceSpinBox->setDecimals(sourceRange.decimals());
        this->sourceSpinBoxes.push_back(sourceSpinBox);
        mainGridLayout->addWidget(sourceSpinBox,i, 2);

    }

    QPushButton* applyButton = new QPushButton("Apply");
    mainGridLayout->addWidget(applyButton, localNumOfBoards+1, 0, 1, 3);
    connect(applyButton, &QPushButton::clicked, this, &BoardControlDockWidget::onApplyButtonClicked);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    mainGridLayout->addWidget(spacer, localNumOfBoards+2,0);
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
    scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    vl->addWidget(scrollArea);

    QWidget * scrollWg = new QWidget;
    scrollArea->setWidget(scrollWg);

    QGridLayout * scrollHl = new QGridLayout;
    scrollHl->setContentsMargins(0, 0, 20, 0);
    scrollHl->setSpacing(1);
    scrollWg->setLayout(scrollHl);

    return scrollHl;
}

vector<uint16_t> BoardControlDockWidget::getChangedChannelIndexes(vector<MySpinBox*> spinBoxVector, vector<double> previousChannelValues){
    int j=0;
    vector<uint16_t> changedChannelIndexes;
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
    this->mDev->getGateVoltagesTunerFeatures(range);
    vector<Measurement_t> gateChangedVoltages;
    vector<uint16_t> gateChangedChannelIndexes = this->getChangedChannelIndexes(this->gateSpinBoxes, this->previousGateSpinBoxValues);
    gateChangedVoltages.resize(gateChangedChannelIndexes.size());
    for(int i = 0; i < gateChangedChannelIndexes.size(); i++){
        double changedValue;
        changedValue = this->gateSpinBoxes[gateChangedChannelIndexes[i]]->value();
        gateChangedVoltages[i] = {changedValue, range.prefix, range.unit};
    }

    this->mDev->getSourceVoltagesTunerFeatures(range);
    vector<Measurement_t> sourceChangedVoltages;
    vector<uint16_t> sourceChangedChannelIndexes = this->getChangedChannelIndexes(this->sourceSpinBoxes, this->previousSourceSpinBoxValues);
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

