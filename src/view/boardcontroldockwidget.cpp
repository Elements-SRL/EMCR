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
    mainGridLayout->addWidget(applyButton, localNumOfBoards+1, 0, 1, 3);
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
    scrollHl->setContentsMargins(0, 0, 0, 0);
    scrollHl->setSpacing(1);
    scrollWg->setLayout(scrollHl);

    return scrollHl;
}
