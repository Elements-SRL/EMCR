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
    dataTable->setColumnCount(8);
    dataTable->setRowCount(currentChannels + 1);
    dataTable->horizontalHeader()->hide();
    dataTable->verticalHeader()->hide();
    mainVl->addWidget(dataTable);
    this->installEventFilter(dataTable);
    int col = 0;

    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    dataTable->setItem(0, col++, new QTableWidgetItem("Channel index"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Mean Voltage [mV]"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Voltage RMS [mV]"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Mean Current [pA]"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Current RMS [pA]"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Resistance [MOHm]"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Offset recalibration [pA]"));
    dataTable->setItem(0, col++, new QTableWidgetItem("Liquid junction [mV]"));
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

void MeasurementsOverviewDockWidget::setOffsetRecalibrationResult(std::vector<Measurement_t> results) {
    this->setCellText(0, 6, QString::fromStdString("Offset recalibration [" + results[0].getFullUnit())+ "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, 6, QString("%1").arg(result.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setLiquidJunctionResult(std::vector <Measurement_t> results) {
    this->setCellText(0, 6, QString::fromStdString("Liquid junction [" + results[0].getFullUnit())+ "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, 7, QString("%1").arg(result.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onLiveStatisticsResult(std::vector <StatisticsResult_t> results) {
    this->setCellText(0, 1, QString::fromStdString("Mean Voltage [" + results[0].meanVoltage.getFullUnit())+ "]");
    this->setCellText(0, 2, QString::fromStdString("Voltage RMS [" + results[0].stdVoltage.getFullUnit())+ "]");
    this->setCellText(0, 3, QString::fromStdString("Mean Current [" + results[0].meanCurrent.getFullUnit())+ "]");
    this->setCellText(0, 4, QString::fromStdString("Current RMS [" + results[0].stdCurrent.getFullUnit())+ "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &statisticResult = results[ch];
        setStatisticsResultsInRowaRow(row, statisticResult);
        row++;
    }
}

void MeasurementsOverviewDockWidget::onResistanceEstimationResult(std::vector <SingleMeasResult_t> results) {
    this->setCellText(0, 5, QString::fromStdString("Resistance [" + results[0].meas.getFullUnit()) + "]");
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, 5, QString("%1").arg(result.meas.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setStatisticsResultsInRowaRow(int row, StatisticsResult &r) {
    int col = 0;
    this->setCellText(row, col++, QString("%1").arg((r.chIdx + 1)));
    this->setCellText(row, col++, QString("%1").arg((r.meanVoltage.value)));
    this->setCellText(row, col++, QString("%1").arg((r.stdVoltage.value)));
    this->setCellText(row, col++, QString("%1").arg((r.meanCurrent.value)));
    this->setCellText(row, col++, QString("%1").arg((r.stdCurrent.value)));
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
