#include "measurementsoverviewdockwidget.h"
#include <QScrollBar>
#include <QScrollArea>
#include <iostream>

MeasurementsOverviewDockWidget::MeasurementsOverviewDockWidget(int numberOfChannels, QWidget * parent) :
    QDockWidget(parent), numberOfChannels(numberOfChannels){

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

    channelIndexesLabel = new QLabel("Active channels");

    meanVoltageBtn = new QPushButton("Mean Voltage");
    meanCurrentBtn = new QPushButton("Mean Current");
    stdCurrentBtn = new QPushButton("Std Voltage");
    conductivityBtn = new QPushButton("Conductivity");

    meanVoltageBtn->setCheckable(true);
    meanCurrentBtn->setCheckable(true);
    stdCurrentBtn->setCheckable(true);
    conductivityBtn->setCheckable(true);

    meanVoltageBtn->setChecked(true);
    meanCurrentBtn->setChecked(true);
    stdCurrentBtn->setChecked(true);
    conductivityBtn->setChecked(true);

    buttonsLayout->addWidget(channelIndexesLabel);
    buttonsLayout->addWidget(meanVoltageBtn);
    buttonsLayout->addWidget(meanCurrentBtn);
    buttonsLayout->addWidget(stdCurrentBtn);
    buttonsLayout->addWidget(conductivityBtn);

    for (int i=0; i< numberOfChannels; i++) {
        meanVoltageLabels.push_back(new QLabel(QString("-")));
        meanCurrentLabels.push_back(new QLabel(QString("-")));
        stdCurrentLabels.push_back(new QLabel(QString("-")));
        conductivityLabels.push_back(new QLabel(QString("-")));
    }

    connect(meanVoltageBtn, &QPushButton::clicked, this, [=](){
        updateButton(meanVoltageBtn);
    });
    connect(meanCurrentBtn, &QPushButton::clicked, this, [=](){
        updateButton(meanCurrentBtn);
    });
    connect(stdCurrentBtn, &QPushButton::clicked, this, [=](){
        updateButton(stdCurrentBtn);
    });
    connect(conductivityBtn, &QPushButton::clicked, this, [=](){
        updateButton(conductivityBtn);
    });

    mainVl->addLayout(buttonsLayout);

    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainVl->addWidget(scrollArea);

    QWidget * scrollWg = new QWidget;
    scrollArea->setWidget(scrollWg);

    QVBoxLayout * scrollVl = new QVBoxLayout;
    scrollVl->setContentsMargins(0, 0, 0, 0);
    scrollVl->setSpacing(1);
    scrollWg->setLayout(scrollVl);

    QGridLayout *gl = new QGridLayout();
    // Initial layout structure

    for (int i=0; i<numberOfChannels; i++){
        gl->addWidget(meanVoltageLabels[i], i, 0);
        gl->addWidget(meanCurrentLabels[i], i, 1);
        gl->addWidget(stdCurrentLabels[i], i, 2);
        gl->addWidget(conductivityLabels[i], i, 3);
    }

    setAllWidgetsInvisible(meanVoltageLabels);
    setAllWidgetsInvisible(meanCurrentLabels);
    setAllWidgetsInvisible(stdCurrentLabels);
    setAllWidgetsInvisible(conductivityLabels);

    for(int i: getActiveChannels()){
        meanVoltageLabels[i]->setVisible(true);
        meanCurrentLabels[i]->setVisible(true);
        stdCurrentLabels[i]->setVisible(true);
        conductivityLabels[i]->setVisible(true);
    }

    // Create and add widgets to the layout
//    mainVl->addLayout(gl);
    scrollVl->addLayout(gl);

}

void MeasurementsOverviewDockWidget::updateButton(QPushButton * bt){
    onUpdate();
}

template<typename T>
void MeasurementsOverviewDockWidget::setAllWidgetsInvisible(const std::vector<T>& widgets){
    for (const auto& widget : widgets) {
        widget->setVisible(false);
    }
}

template<typename T>
void MeasurementsOverviewDockWidget::setActiveChannelsVisible(const std::vector<T>& widgets, std::vector<int> active_channels){
    for (auto i : active_channels) {
        widgets[i]->setVisible(true);
    }
}

void MeasurementsOverviewDockWidget::onUpdate(){
    std::vector<int> activeChannels = getActiveChannels();
    setAllWidgetsInvisible(meanVoltageLabels);
    setAllWidgetsInvisible(meanCurrentLabels);
    setAllWidgetsInvisible(stdCurrentLabels);
    setAllWidgetsInvisible(conductivityLabels);
    if (meanVoltageBtn->isChecked()){
        setActiveChannelsVisible(meanVoltageLabels, activeChannels);
    }
    if (meanCurrentBtn->isChecked()){
        setActiveChannelsVisible(meanCurrentLabels, activeChannels);
    }
    if (stdCurrentBtn->isChecked()){
        setActiveChannelsVisible(stdCurrentLabels, activeChannels);
    }
    if (conductivityBtn->isChecked()){
        setActiveChannelsVisible(conductivityLabels, activeChannels);
    }
    std::cout << "onUpdate"<< std::endl;
}

void MeasurementsOverviewDockWidget::onNewMeasurement(std::vector<Measurement_t> measurements){
    std::cout << "onNewMeasurement"<< std::endl;
}

void MeasurementsOverviewDockWidget::onResult(StatisticsResult * result) {
    applyTextFromValuesAndaPfx(stdCurrentLabels, result->getStdCurrent().toStdVector(), "A");
    applyTextFromValuesAndaPfx(meanCurrentLabels, result->getMeanCurrent().toStdVector(), "A");
    applyTextFromValuesAndaPfx(meanVoltageLabels, result->getMeanVoltage().toStdVector(), "V");
    applyTextFromValuesAndaPfx(conductivityLabels, result->getConductivity().toStdVector(), "TODO");
}

template<typename T>
void MeasurementsOverviewDockWidget::applyTextFromValuesAndaPfx(const std::vector<T>& widgets, std::vector<double> values, std::string pfx){
    for (int i = 0; i<numberOfChannels; i++) {
        Measurement_t m = {values[i], UnitPfx::UnitPfxNone, pfx};
        widgets[i]->setText(QString::fromStdString(m.niceLabel()));
    }
}

std::vector<int> MeasurementsOverviewDockWidget::getActiveChannels(){
    std::vector<int> v = {0,1,2,3,4,5,6,6,7,8,9,90,91,92,93,94,95,96,97,98,99,};
    return v;
}
