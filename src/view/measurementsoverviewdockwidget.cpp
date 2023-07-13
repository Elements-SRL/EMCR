#include "measurementsoverviewdockwidget.h"
#include <QScrollBar>
#include <QScrollArea>
#include <iostream>

MeasurementsOverviewDockWidget::MeasurementsOverviewDockWidget(QWidget * parent) :
    QDockWidget(parent) {

    mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setWindowTitle("Measurements Overview");
    setObjectName("measurementsOverviewDW");
    setWidget(mainWg);

    mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(1);
    mainWg->setLayout(mainVl);

    QHBoxLayout * buttonsLayout = new QHBoxLayout();

    b1 = new QPushButton("b1");
    b2 = new QPushButton("b2");
    b3 = new QPushButton("b3");

    b1->setCheckable(true);
    b2->setCheckable(true);
    b3->setCheckable(true);

    b1->setChecked(true);
    b2->setChecked(true);
    b3->setChecked(true);

    buttonsLayout->addWidget(b1);
    buttonsLayout->addWidget(b2);
    buttonsLayout->addWidget(b3);


    for (int i=0; i< getTotalChannelsChannels(); i++) {
        col1.push_back(new QPushButton(QString("col 1 row %1").arg(i)));
        col2.push_back(new QPushButton(QString("col 2 row %1").arg(i)));
        col3.push_back(new QPushButton(QString("col 3 row %1").arg(i)));
    }

    for (QPushButton * b : col1) {
        b->setVisible(false);
    }
    for (QPushButton * b : col2) {
        b->setVisible(false);
    }
    for (QPushButton * b : col3) {
        b->setVisible(false);
    }

    connect(b1, &QPushButton::clicked, this, [=](){
        updateButton(b1);
    });
    connect(b2, &QPushButton::clicked, this, [=](){
        updateButton(b2);
    });
    connect(b3, &QPushButton::clicked, this, [=](){
        updateButton(b3);
    });
    mainVl->addLayout(buttonsLayout);

    QGridLayout *gl = new QGridLayout();
    // Initial layout structure

    for (int i= 0; i<getTotalChannelsChannels(); i++){
        gl->addWidget(col1[i], i, 0);
        gl->addWidget(col2[i], i, 1);
        gl->addWidget(col3[i], i, 2);
    }

    for(int i: getActiveChannels()){

        col1[i]->setVisible(true);
        col2[i]->setVisible(true);
        col3[i]->setVisible(true);
    }

    // Create and add widgets to the layout
    mainVl->addLayout(gl);
}

void MeasurementsOverviewDockWidget::updateButton(QPushButton * bt){

//    if (bt->isChecked()){
//        bt->setChecked(false);
//    } else {
//        bt->setChecked(true);
//    }
    onUpdate();
}

void MeasurementsOverviewDockWidget::setAllButtonsInvisible(std::vector<QPushButton *> bts){
    foreach (auto bt, bts) {
        bt->setVisible(false);
    }
}

void MeasurementsOverviewDockWidget::setActiveChannelsVisible(std::vector<QPushButton *> bts, std::vector<int> active_channels){
    for (auto i : active_channels) {
        bts[i]->setVisible(true);
    }
}

void MeasurementsOverviewDockWidget::onUpdate(){
    std::vector<int> activeChannels = getActiveChannels();
    setAllButtonsInvisible(col1);
    setAllButtonsInvisible(col2);
    setAllButtonsInvisible(col3);
    if (b1->isChecked()){
        setActiveChannelsVisible(col1, activeChannels);
    }
    if (b2->isChecked()){
        setActiveChannelsVisible(col2, activeChannels);
    }
    if (b3->isChecked()){
        setActiveChannelsVisible(col3, activeChannels);
    }
    std::cout << "onUpdate"<< std::endl;
}

void MeasurementsOverviewDockWidget::onNewMeasurement(std::vector<Measurement_t> measurements){
    std::cout << "onNewMeasurement"<< std::endl;
}

void MeasurementsOverviewDockWidget::onResult(StatisticsResult * result) {
    Measurement_t meas = {result->getStdCurrent()[0], UnitPfx::UnitPfxNone, "A"};
    std::cout<< meas.niceLabel() << std::endl;
    col1[1]->setText(QString::fromStdString(meas.niceLabel()));
}

std::vector<int> MeasurementsOverviewDockWidget::getActiveChannels(){
    std::vector<int> v = {0,1,2,3,4,5,6,6,7,8,9,90,91,92,93,94,95,96,97,98,99,};
    return v;
}

int MeasurementsOverviewDockWidget::getTotalChannelsChannels(){
    return 100;
}
