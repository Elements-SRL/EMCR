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
        auto &result = results[ch];
        this->setCellText(row, 11, QString("%1").arg(result.value));
        this->setCellText(row, 12, QString::fromStdString(result.getFullUnit()));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setLiquidJunctionResult(std::vector<Measurement_t> results) {
    //the first row is the header
    int row = 1;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        this->setCellText(row, 13, QString("%1").arg(result.value));
        this->setCellText(row, 14, QString::fromStdString(result.getFullUnit()));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onLiveStatisticsResult(std::vector<StatisticsResult> results) {
    //the first row is the header
    int row = 1;
    for (auto ch : activeChannels) {
        auto &statisticResult = results[ch];
        setStatisticsResultsInRowaRow(row, statisticResult);
        row++;
    }
}

void MeasurementsOverviewDockWidget::setStatisticsResultsInRowaRow(int row, StatisticsResult &r) {
    int col = 0;
    this->setCellText(row, col++, QString("%1").arg((r.chIdx + 1)));
    this->setCellText(row, col++, QString("%1").arg((r.meanVoltage.value)));
    this->setCellText(row, col++, QString::fromStdString(r.meanVoltage.getFullUnit()));
    this->setCellText(row, col++, QString("%1").arg((r.stdVoltage.value)));
    this->setCellText(row, col++, QString::fromStdString(r.stdVoltage.getFullUnit()));
    this->setCellText(row, col++, QString("%1").arg((r.meanCurrent.value)));
    this->setCellText(row, col++, QString::fromStdString(r.meanCurrent.getFullUnit()));
    this->setCellText(row, col++, QString("%1").arg((r.stdCurrent.value)));
    this->setCellText(row, col++, QString::fromStdString(r.stdCurrent.getFullUnit()));
    this->setCellText(row, col++, QString("%1").arg((r.conductivity.value)));
    this->setCellText(row, col++, QString::fromStdString(r.conductivity.getFullUnit()));
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
