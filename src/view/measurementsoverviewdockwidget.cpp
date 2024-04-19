#include "measurementsoverviewdockwidget.h"
#include <QScrollBar>
#include <QScrollArea>
#include <QHeaderView>

MeasurementsOverviewDockWidget::MeasurementsOverviewDockWidget(std::vector<uint16_t> activeChannels, int voltageChannels, int currentChannels, QWidget * parent) :
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

    dataTable = new CopyableTable();
    dataTable->setColumnCount(11);
    dataTable->setRowCount(currentChannels + 1);
    dataTable->horizontalHeader()->hide();
    dataTable->verticalHeader()->hide();
    mainVl->addWidget(dataTable);
    this->installEventFilter(dataTable);
    dataTable->setItem(0, 0, new QTableWidgetItem("Channel index"));
    dataTable->setItem(0, 1, new QTableWidgetItem("Mean Voltage"));
    dataTable->setItem(0, 2, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, 3, new QTableWidgetItem("Mean Current"));
    dataTable->setItem(0, 4, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, 5, new QTableWidgetItem("Current RMS"));
    dataTable->setItem(0, 6, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, 7, new QTableWidgetItem("Conductivity"));
    dataTable->setItem(0, 8, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, 9, new QTableWidgetItem("Liquid junction"));
    dataTable->setItem(0, 10, new QTableWidgetItem("Unit"));
    mainVl->addWidget(dataTable);


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

void MeasurementsOverviewDockWidget::onUpdate(){
    dataTable->setRowCount(activeChannels.size() + 1);
}

void MeasurementsOverviewDockWidget::updateActiveChannels(std::vector<uint16_t> newActiveChannels){
    activeChannels = newActiveChannels;
    onUpdate();
}

void MeasurementsOverviewDockWidget::setLiquidJunctionResult(std::vector<Measurement_t> results) {
    for (int i = 0; i < currentChannels; i++) {
        //the first row is the header
        const auto row = i + 1;
        const auto ch = activeChannels[i];
        auto result = results[i];
        dataTable->setItem(row, 9, new QTableWidgetItem(QString::fromStdString(std::to_string(result.value))));
        dataTable->setItem(row, 10, new QTableWidgetItem(QString::fromStdString(result.getFullUnit())));
    }
}

void MeasurementsOverviewDockWidget::onLiveStatisticsResult(std::vector<StatisticsResult> results) {
    for (int i = 0; i < currentChannels; i++) {
        //the first row is the header
        const auto row = i + 1;
        const auto statisticResult = results[i];
        setStatisticsResultsInRowaRow(row, statisticResult);
    }
}

void MeasurementsOverviewDockWidget::setStatisticsResultsInRowaRow(int row, StatisticsResult r) {
    dataTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(std::to_string(r.chIdx + 1))));
    dataTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(std::to_string(r.meanVoltage.value))));
    dataTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(r.meanVoltage.getFullUnit())));
    dataTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(std::to_string(r.meanCurrent.value))));
    dataTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(r.meanCurrent.getFullUnit())));
    dataTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(std::to_string(r.stdCurrent.value))));
    dataTable->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(r.stdCurrent.getFullUnit())));
    dataTable->setItem(row, 7, new QTableWidgetItem(QString::fromStdString(std::to_string(r.conductivity.value))));
    dataTable->setItem(row, 8, new QTableWidgetItem(QString::fromStdString(r.conductivity.getFullUnit())));
}
