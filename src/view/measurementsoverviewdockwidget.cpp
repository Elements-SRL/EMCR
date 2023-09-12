#include "measurementsoverviewdockwidget.h"
#include <QScrollBar>
#include <QScrollArea>
#include <iostream>

MeasurementsOverviewDockWidget::MeasurementsOverviewDockWidget(std::vector<int> activeChannels, int voltageChannels, int currentChannels, QWidget * parent) :
    QDockWidget(parent), activeChannels(activeChannels), voltageChannels(voltageChannels), currentChannels(currentChannels){

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
    stdCurrentBtn = new QPushButton("Current RMS");
    conductivityBtn = new QPushButton("Conductivity");
    liquidJunctionBtn = new QPushButton("Conductivity");

    meanVoltageBtn->setCheckable(true);
    meanCurrentBtn->setCheckable(true);
    stdCurrentBtn->setCheckable(true);
    conductivityBtn->setCheckable(true);
    liquidJunctionBtn->setCheckable(true);

    meanVoltageBtn->setChecked(true);
    meanCurrentBtn->setChecked(true);
    stdCurrentBtn->setChecked(true);
    conductivityBtn->setChecked(true);
    liquidJunctionBtn->setChecked(true);

    buttonsLayout->addWidget(channelIndexesLabel);
    buttonsLayout->addWidget(meanVoltageBtn);
    buttonsLayout->addWidget(meanCurrentBtn);
    buttonsLayout->addWidget(stdCurrentBtn);
    buttonsLayout->addWidget(conductivityBtn);
    buttonsLayout->addWidget(liquidJunctionBtn);

    mainVl->addLayout(buttonsLayout);

    channelIndexesLabel->setVisible(false);
    meanVoltageBtn->setVisible(false);
    meanCurrentBtn->setVisible(false);
    stdCurrentBtn->setVisible(false);
    conductivityBtn->setVisible(false);
    liquidJunctionBtn->setVisible(false);

    for (int i=0; i< currentChannels; i++) {
        activeChannelsLabels.push_back(new QLabel(QString(" %1").arg(i+1), mainWg));
        meanVoltageLabels.push_back(new QLabel(QString("-"), mainWg));
        meanCurrentLabels.push_back(new QLabel(QString("-"), mainWg));
        stdCurrentLabels.push_back(new QLabel(QString("-"), mainWg));
        conductivityLabels.push_back(new QLabel(QString("-"), mainWg));
        liquidJunctionLabels.push_back(new QLabel(QString("-"), mainWg));
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
    connect(liquidJunctionBtn, &QPushButton::clicked, this, [=](){
        updateButton(liquidJunctionBtn);
    });

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

    for (int i=0; i<currentChannels; i++){
        gl->addWidget(new QLabel(QString::fromStdString(" Active Channels")), 0, 0);
        gl->addWidget(new QLabel(QString::fromStdString(" Mean Voltage")), 0, 1);
        gl->addWidget(new QLabel(QString::fromStdString( "Mean Current")), 0, 2);
        gl->addWidget(new QLabel(QString::fromStdString(" Current RMS")), 0, 3);
        gl->addWidget(new QLabel(QString::fromStdString(" Conductivity")), 0, 4);
        gl->addWidget(new QLabel(QString::fromStdString(" Liquid Junction")), 0, 5);

        gl->addWidget(activeChannelsLabels[i], i+1, 0);
        gl->addWidget(meanVoltageLabels[i], i+1, 1);
        gl->addWidget(meanCurrentLabels[i], i+1, 2);
        gl->addWidget(stdCurrentLabels[i], i+1, 3);
        gl->addWidget(conductivityLabels[i], i+1, 4);
        gl->addWidget(liquidJunctionLabels[i], i+1, 5);
    }
    setAllWidgetsInvisible(activeChannelsLabels);
    setAllWidgetsInvisible(meanVoltageLabels);
    setAllWidgetsInvisible(meanCurrentLabels);
    setAllWidgetsInvisible(stdCurrentLabels);
    setAllWidgetsInvisible(conductivityLabels);
    setAllWidgetsInvisible(liquidJunctionLabels);

    for(int i: activeChannels){
        activeChannelsLabels[i]->setVisible(true);
        meanVoltageLabels[i]->setVisible(true);
        meanCurrentLabels[i]->setVisible(true);
        stdCurrentLabels[i]->setVisible(true);
        conductivityLabels[i]->setVisible(true);
        liquidJunctionLabels[i]->setVisible(true);
    }
    QPushButton * extractBtn = new QPushButton("extract");
    connect(extractBtn, &QPushButton::clicked, this, [=](){
        QString filePath = QFileDialog::getSaveFileName(
                nullptr,
                "Save File",
                QDir::homePath(), // Default directory
                "Csv Files (*.csv);;All Files (*)"
            );

            if (!filePath.isEmpty()) {
                emit extract(filePath);
            }
    });
    scrollVl->addWidget(extractBtn);
    scrollVl->addLayout(gl);
}

void MeasurementsOverviewDockWidget::updateButton(QPushButton *){
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
    setAllWidgetsInvisible(activeChannelsLabels);
    setAllWidgetsInvisible(meanVoltageLabels);
    setAllWidgetsInvisible(meanCurrentLabels);
    setAllWidgetsInvisible(stdCurrentLabels);
    setAllWidgetsInvisible(conductivityLabels);
    setAllWidgetsInvisible(liquidJunctionLabels);
    setActiveChannelsVisible(activeChannelsLabels, activeChannels);
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
    if (liquidJunctionBtn->isChecked()){
        setActiveChannelsVisible(liquidJunctionLabels, activeChannels);
    }
}

void MeasurementsOverviewDockWidget::updateActiveChannels(std::vector<int> newActiveChannels){
    activeChannels = newActiveChannels;
    onUpdate();
}

void MeasurementsOverviewDockWidget::setLiquidJunctionResult(QVector <Measurement_t> result) {
    applyTextFromMeasurements(liquidJunctionLabels, result);
}

void MeasurementsOverviewDockWidget::onLiveStatisticsResult(StatisticsResult * result) {
    applyTextFromValuesAndPfx(stdCurrentLabels, result->stdCurrent.toStdVector(), "A");
    applyTextFromValuesAndPfx(meanCurrentLabels, result->meanCurrent.toStdVector(), "A");
    applyTextFromValuesAndPfx(meanVoltageLabels, result->meanVoltage.toStdVector(), "V");
    applyTextFromValuesAndPfx(conductivityLabels, result->conductivity.toStdVector(), "S");
}

template<typename T>
void MeasurementsOverviewDockWidget::applyTextFromValuesAndPfx(const std::vector<T>& widgets, std::vector<double> values, std::string pfx){
    QVector<Measurement_t> measurements;
    for (int i = 0; i<currentChannels; i++) {
        measurements.push_back({values[i], UnitPfx::UnitPfxNone, pfx});
    }
    applyTextFromMeasurements(widgets, measurements);
}

template<typename T>
void MeasurementsOverviewDockWidget::applyTextFromMeasurements(const std::vector<T>& widgets, QVector<Measurement_t> meas) {
    for (int i = 0; i<currentChannels; i++) {
        widgets[i]->setText(QString::fromStdString(meas[i].niceLabel()));
    }
}
