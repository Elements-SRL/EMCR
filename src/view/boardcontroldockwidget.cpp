#include "boardcontroldockwidget.h"

#include <vector>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>

BoardControlDockWidget::BoardControlDockWidget(ModelDevice * mDev, QWidget * parent) :
    QDockWidget(parent),
    mDev(mDev) {

    int localNumOfBoards = 24;

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Board controls");

    this->setWidget(mainWg);

    QGridLayout * mainGridLayout = new QGridLayout();
    mainGridLayout->setContentsMargins(0, 0, 0, 0);
    mainGridLayout->setSpacing(1);
    mainWg->setLayout(mainGridLayout);

    // Column captions
    mainGridLayout->addWidget(new QLabel(" Channel"), 0, 0);
    mainGridLayout->addWidget(new QLabel("Gate"), 0, 1);
    mainGridLayout->addWidget(new QLabel("Source"), 0, 2);

    for(int i = 1; i <= localNumOfBoards; i++){
        QString channelLabel= QString("    %1").arg(i);
        mainGridLayout->addWidget(new QLabel(channelLabel),i, 0);

    }

    vector<QDoubleSpinBox*>  gateSpinBoxes;
    QDoubleSpinBox* gateSpinBox;
    for(int i = 1; i <= localNumOfBoards; i++){
        gateSpinBox = new QDoubleSpinBox();
        gateSpinBox->setSuffix(" mV");
        gateSpinBoxes.push_back(gateSpinBox);
        mainGridLayout->addWidget(gateSpinBox,i, 1);

    }

    vector<QDoubleSpinBox*>  sourceSpinBoxes;
    QDoubleSpinBox* sourceSpinBox;
    for(int i = 1; i <= localNumOfBoards; i++){
        sourceSpinBox = new QDoubleSpinBox();
        sourceSpinBox->setSuffix(" mV");
        sourceSpinBoxes.push_back(sourceSpinBox);
        mainGridLayout->addWidget(sourceSpinBox,i, 2);

    }
    QPushButton* applyButton = new QPushButton("Apply");
    mainGridLayout->addWidget(applyButton, 25, 0, 25, 3);


}
