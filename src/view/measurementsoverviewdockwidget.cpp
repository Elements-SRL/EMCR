#include "measurementsoverviewdockwidget.h"
#include <QScrollBar>
#include <QScrollArea>
#include <QHeaderView>

MeasurementsOverviewDockWidget::MeasurementsOverviewDockWidget(std::vector<uint16_t> activeChannels, int voltageChannels, int currentChannels, QWidget * parent) :
    QDockWidget(parent),
    activeChannels(activeChannels),
    voltageChannels(voltageChannels),
    currentChannels(currentChannels) {

    this->setVisible(false);

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

    auto exportButton = new QPushButton(this);
    exportButton->setIcon(QIcon(QPixmap(":/imgs/export protocol.png")));
    exportButton->setToolTip("Export to csv");
    exportButton->setIconSize(QSize(30, 30));
    exportButton->setFixedSize(32, 32);
    connect(exportButton, &QPushButton::clicked, this, [=]() {
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
    mainVl->addWidget(exportButton);

    dataTable = new CopyableTable(scrollWidget);
    dataTable->setColumnCount(15);
    dataTable->setRowCount(currentChannels + 1);
    dataTable->horizontalHeader()->hide();
    dataTable->verticalHeader()->hide();
    mainVl->addWidget(dataTable);
    this->installEventFilter(dataTable);
    int col = 0;
    dataTable->setItem(0, col++, new QTableWidgetItem("Channel index"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Mean Voltage"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Voltage RMS"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Mean Current"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Current RMS"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Conductivity"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Offset recalibration"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Liquid junction"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Unit"));
    mainVl->addWidget(dataTable);
}

void MeasurementsOverviewDockWidget::onUpdate(){
    dataTable->setRowCount(activeChannels.size() + 1);
}

void MeasurementsOverviewDockWidget::updateActiveChannels(std::vector<uint16_t> newActiveChannels){
    activeChannels = newActiveChannels;
    onUpdate();
}

void MeasurementsOverviewDockWidget::setOffsetRecalibrationResult(std::vector<Measurement_t> results) {
    //the first row is the header
    int row = 1;
    for (auto ch : activeChannels) {
        auto result = results[ch];
        dataTable->setItem(row, 11, new QTableWidgetItem(QString::fromStdString(std::to_string(result.value))));
        dataTable->setItem(row, 12, new QTableWidgetItem(QString::fromStdString(result.getFullUnit())));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setLiquidJunctionResult(std::vector<Measurement_t> results) {
    //the first row is the header
    int row = 1;
    for (auto ch : activeChannels) {
        auto result = results[ch];
        dataTable->setItem(row, 13, new QTableWidgetItem(QString::fromStdString(std::to_string(result.value))));
        dataTable->setItem(row, 14, new QTableWidgetItem(QString::fromStdString(result.getFullUnit())));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onLiveStatisticsResult(std::vector<StatisticsResult> results) {
    //the first row is the header
    int row = 1;
    for (auto ch : activeChannels) {
        const auto statisticResult = results[ch];
        setStatisticsResultsInRowaRow(row, statisticResult);
        row++;
    }
}

void MeasurementsOverviewDockWidget::setStatisticsResultsInRowaRow(int row, StatisticsResult r) {
    int col = 0;
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(std::to_string(r.chIdx + 1))));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(std::to_string(r.meanVoltage.value))));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(r.meanVoltage.getFullUnit())));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(std::to_string(r.stdVoltage.value))));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(r.stdVoltage.getFullUnit())));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(std::to_string(r.meanCurrent.value))));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(r.meanCurrent.getFullUnit())));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(std::to_string(r.stdCurrent.value))));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(r.stdCurrent.getFullUnit())));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(std::to_string(r.conductivity.value))));
    dataTable->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(r.conductivity.getFullUnit())));
}
