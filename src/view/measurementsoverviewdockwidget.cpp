#include "measurementsoverviewdockwidget.h"
#include <QScrollBar>
#include <QScrollArea>
#include <QHeaderView>

MeasurementsOverviewDockWidget::MeasurementsOverviewDockWidget(std::vector<uint16_t> activeChannels, int voltageChannels, int currentChannels, QWidget * parent) :
    QDockWidget(parent), activeChannels(activeChannels), voltageChannels(voltageChannels), currentChannels(currentChannels){

    setWindowTitle("Measurements Overview");
    setObjectName("measurementsOverviewDW");
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create a scroll area
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true); // Allow the widget inside the scroll area to resize with the scroll area
    setWidget(scrollArea);

    // Create a widget for the scroll area
    QWidget* scrollWidget = new QWidget();
    scrollArea->setWidget(scrollWidget);

    mainVl = new QVBoxLayout(scrollWidget);
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(1);

    dataTable = new CopyableTable(scrollWidget);
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
    mainVl->addWidget(extractBtn);
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
