#include "measurementsoverviewdockwidget.h"

#include <QScrollBar>
#include <QScrollArea>
#include <QHeaderView>
#include <QFileDialog>

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
    dataTable->setColumnCount(ColumnsNum);
    dataTable->setRowCount(currentChannels + 1);
    dataTable->horizontalHeader()->hide();
    dataTable->verticalHeader()->hide();
    mainVl->addWidget(dataTable);
    this->installEventFilter(dataTable);

    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    dataTable->setItem(0, ColChannelIndex, new QTableWidgetItem("Channel index"));
    dataTable->setItem(0, ColMeanVoltage, new QTableWidgetItem("Mean Voltage [mV]"));
    dataTable->setItem(0, ColVoltageRms, new QTableWidgetItem("Voltage RMS [mV]"));
    dataTable->setItem(0, ColMeanCurrent, new QTableWidgetItem("Mean Current [pA]"));
    dataTable->setItem(0, ColCurrentRms, new QTableWidgetItem("Current RMS [pA]"));
    dataTable->setItem(0, ColResistance, new QTableWidgetItem("Resistance [MOHm]"));
    dataTable->setItem(0, ColPipetteCapacitance, new QTableWidgetItem("Pipette capacitance [pF]"));
    dataTable->setItem(0, ColOffsetRecalibration, new QTableWidgetItem("Offset recalibration [pA]"));
    dataTable->setItem(0, ColLiquidJunction, new QTableWidgetItem("Liquid junction [mV]"));
    mainVl->addWidget(dataTable);
}

void MeasurementsOverviewDockWidget::onUpdate(){
    dataTable->setRowCount(activeChannels.size() + 1);
}

void MeasurementsOverviewDockWidget::updateActiveChannels(std::vector<uint16_t> newActiveChannels){
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    activeChannels = newActiveChannels;
    onUpdate();
}

void MeasurementsOverviewDockWidget::setOffsetRecalibrationResult(std::vector<e384cl::Measurement_t> results) {
    this->setCellText(0, ColOffsetRecalibration, QString::fromStdString("Offset recalibration [" + results[0].getFullUnit())+ "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, ColOffsetRecalibration, QString("%1").arg(result.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setLiquidJunctionResult(std::vector <e384cl::Measurement_t> results) {
    this->setCellText(0, ColLiquidJunction, QString::fromStdString("Liquid junction [" + results[0].getFullUnit())+ "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, ColLiquidJunction, QString("%1").arg(result.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onLiveStatisticsResult(std::vector <StatisticsResult_t> results) {
    this->setCellText(0, ColMeanVoltage, QString::fromStdString("Mean Voltage [" + results[0].meanVoltage.getFullUnit())+ "]");
    this->setCellText(0, ColVoltageRms, QString::fromStdString("Voltage RMS [" + results[0].stdVoltage.getFullUnit())+ "]");
    this->setCellText(0, ColMeanCurrent, QString::fromStdString("Mean Current [" + results[0].meanCurrent.getFullUnit())+ "]");
    this->setCellText(0, ColCurrentRms, QString::fromStdString("Current RMS [" + results[0].stdCurrent.getFullUnit())+ "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &statisticResult = results[ch];
        setStatisticsResultsInRowaRow(row, statisticResult);
        row++;
    }
}

void MeasurementsOverviewDockWidget::onResistanceEstimationResult(std::vector <SingleMeasResult_t> results) {
    this->setCellText(0, ColResistance, QString::fromStdString("Resistance [" + results[0].meas.getFullUnit()) + "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, ColResistance, QString("%1").arg(result.meas.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onPipetteCapacitanceEstimationResult(std::vector <SingleMeasResult_t> results) {
    this->setCellText(0, ColPipetteCapacitance, QString::fromStdString("Pipette capacitance [" + results[0].meas.getFullUnit()) + "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, ColPipetteCapacitance, QString("%1").arg(result.meas.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setStatisticsResultsInRowaRow(int row, StatisticsResult &r) {
    this->setCellText(row, ColChannelIndex, QString("%1").arg((r.chIdx + 1)));
    this->setCellText(row, ColMeanVoltage, QString("%1").arg((r.meanVoltage.value)));
    this->setCellText(row, ColVoltageRms, QString("%1").arg((r.stdVoltage.value)));
    this->setCellText(row, ColMeanCurrent, QString("%1").arg((r.meanCurrent.value)));
    this->setCellText(row, ColCurrentRms, QString("%1").arg((r.stdCurrent.value)));
}

void MeasurementsOverviewDockWidget::setCellText(int row, int col, const QString text) {
    auto item = dataTable->item(row, col);
    if (item != nullptr) {
        item->setText(text);
    }
    else {
        dataTable->setItem(row, col, new QTableWidgetItem(text));
    }
}
