#include "measurementsoverviewdockwidget.h"

#include <QScrollBar>
#include <QScrollArea>
#include <QHeaderView>
#include <QFileDialog>
#include <QCoreApplication>
#include <QKeyEvent>

MeasurementsOverviewDockWidget::MeasurementsOverviewDockWidget(std::vector<uint16_t> activeChannels, int voltageChannels, int currentChannels, QWidget * parent) :
    QDockWidget(parent),
    activeChannels(activeChannels),
    voltageChannels(voltageChannels),
    currentChannels(currentChannels) {

    this->setVisible(false);

    setWindowTitle("Measurements Overview");
    setObjectName("measurementsOverviewDW");
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QWidget * centralWidget = new QWidget();
    centralWidget->setObjectName("measurementsCentralWidget");
    QVBoxLayout* externalLayout = new QVBoxLayout(centralWidget);
    externalLayout->setContentsMargins(0, 0, 0, 0);
    externalLayout->setSpacing(0);

    setWidget(centralWidget);

    // Window border management when floating
    connect(this, &QDockWidget::topLevelChanged, this, [centralWidget](bool isFloating) {
        if (isFloating) {
            centralWidget->setStyleSheet("#measurementsScrollContainer { border: none; }"
                                         "#customTitleBar { border-left: none; border-right: none; }");
        } else {
            centralWidget->setStyleSheet("");
        }
    });

    // Create a scroll widget
    // Scroll area is embedded in CopyableTable
    QFrame * scrollWidget = new QFrame();
    scrollWidget->setObjectName("measurementsScrollContainer");
    auto scrollLayout = new QGridLayout(scrollWidget);
    scrollWidget->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSizeConstraint(QLayout::SetNoConstraint);

    // TOP BAR
    QWidget* customTitleBar = new QWidget();
    customTitleBar->setObjectName("customTitleBar");
    QHBoxLayout* topBarLayout = new QHBoxLayout(customTitleBar);
    topBarLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    auto exportButton = new QPushButton(this);
    exportButton->setObjectName("exportCsvButton");
    exportButton->setText(" Export");
    exportButton->setToolTip("Export to csv");
    exportButton->setIconSize(QSize(12, 12));
    exportButton->setFixedSize(80, 25);
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

    auto copyButton = new QPushButton(this);
    copyButton->setObjectName("copyButton");
    copyButton->setText(" Copy");
    copyButton->setToolTip("Copy selected rows to clipboard");
    copyButton->setIconSize(QSize(12, 12));
    copyButton->setFixedSize(64, 25);

    topBarLayout->addWidget(exportButton);
    topBarLayout->addWidget(copyButton);

    dataTable = new CopyableTable(scrollWidget);
    dataTable->setColumnCount(ColumnsNum);
    dataTable->setRowCount(currentChannels + 1);
    this->installEventFilter(dataTable);

    externalLayout->addWidget(customTitleBar);
    scrollLayout->addWidget(dataTable);

    connect(copyButton, &QPushButton::clicked, this, [=]() {
        QKeyEvent * ctrl_c = new QKeyEvent(QEvent::Type::KeyPress, Qt::Key_C, Qt::ControlModifier);
        QCoreApplication::sendEvent(dataTable, ctrl_c);
    });

    emptyStateLabel = new QLabel(centralWidget);
    emptyStateLabel->setObjectName("emptyStateLabel");
    emptyStateLabel->setText("Empty channel selection");
    emptyStateLabel->hide();
    topBarLayout->addWidget(emptyStateLabel);

    QHeaderView * vHeader = dataTable->verticalHeader();
    vHeader->setObjectName("tableVHeader");
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Unit measures are added as postfix when data is read
    dataTable->setHorizontalHeaderLabels({"Mean Voltage",
                                          "Voltage RMS",
                                          "Mean Current",
                                          "Current RMS",
                                          "Resistance",
                                          "Pipette capacitance",
                                          "Membrane capacitance",
                                          "Access resistance",
                                          "Membrane resistance",
                                          "Offset recalibration",
                                          "Liquid junction"});
    externalLayout->addWidget(scrollWidget);
}

void MeasurementsOverviewDockWidget::onUpdate(){
    if (activeChannels.empty()) {
        dataTable->setRowCount(currentChannels);

        // Reset vertical header
        for (int i = 0; i < currentChannels; ++i) {
            dataTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i + 1)));
        }

        dataTable->clearContents();
        emptyStateLabel->show();

    } else {
        emptyStateLabel->hide();
        dataTable->setRowCount(activeChannels.size());
    }
}

void MeasurementsOverviewDockWidget::updateActiveChannels(std::vector<uint16_t> newActiveChannels){
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    activeChannels = newActiveChannels;
    onUpdate();
}

void MeasurementsOverviewDockWidget::setOffsetRecalibrationResult(std::vector<e384cl::Measurement_t> results) {
    if (results.empty()) return;

    auto firstMeas = results.begin();
    updateHeader(dataTable, ColOffsetRecalibration, "Offset recalibration", results.begin()->getFullUnit());

    int row = 0;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        result.convertValue(firstMeas->prefix);
        this->setCellText(row, ColOffsetRecalibration, QString("%1").arg(result.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setLiquidJunctionResult(std::vector <e384cl::Measurement_t> results) {
    if (results.empty()) return;

    auto firstMeas = results.begin();
    updateHeader(dataTable, ColLiquidJunction, "Liquid junction", firstMeas->getFullUnit());

    int row = 0;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        result.convertValue(firstMeas->prefix);
        this->setCellText(row, ColLiquidJunction, QString("%1").arg(result.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onLiveStatisticsResult(StatisticsResultWrapper_t results) {
    if (results.empty()) return;

    // Reference item for unit measures used by header
    auto firstCh = activeChannels.front();
    auto &firstResult = results[firstCh];

    updateHeader(dataTable, ColMeanVoltage, "Mean Voltage", firstResult.meanVoltage.getFullUnit());
    updateHeader(dataTable, ColVoltageRms, "Voltage RMS", firstResult.stdVoltage.getFullUnit());
    updateHeader(dataTable, ColMeanCurrent, "Mean Current", firstResult.meanCurrent.getFullUnit());
    updateHeader(dataTable, ColCurrentRms, "Current RMS", firstResult.stdCurrent.getFullUnit());

    int row = 0;
    for (auto ch : activeChannels) {
        auto &statisticResult = results[ch];

        // Convert data to same prefix as first row items used as reference
        statisticResult.meanVoltage.convertValue(firstResult.meanVoltage.prefix);
        statisticResult.stdVoltage.convertValue(firstResult.stdVoltage.prefix);
        statisticResult.meanCurrent.convertValue(firstResult.meanCurrent.prefix);
        statisticResult.stdCurrent.convertValue(firstResult.stdCurrent.prefix);

        setStatisticsResultsInRowaRow(row, statisticResult);
        row++;
    }
}

void MeasurementsOverviewDockWidget::onResistanceEstimationResult(SingleMeasResultWrapper_t results) {
    if (results.empty()) return;

    auto firstMeas = results.begin()->meas;
    updateHeader(dataTable, ColResistance, "Resistance", firstMeas.getFullUnit());

    int row = 0;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        result.meas.convertValue(firstMeas.prefix);
        this->setCellText(row, ColResistance, QString("%1").arg(result.meas.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onPipetteCapacitanceEstimationResult(SingleMeasResultWrapper_t results) {
    if (results.empty()) return;

    auto firstMeas = results.begin()->meas;
    updateHeader(dataTable, ColPipetteCapacitance, "Pipette capacitance", firstMeas.getFullUnit());

    int row = 0;
    for (auto ch : activeChannels) {
        auto &result = results[ch];
        result.meas.convertValue(firstMeas.prefix);
        this->setCellText(row, ColPipetteCapacitance, QString("%1").arg(result.meas.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::onMembraneEstimationResult(MembraneResultWrapper_t results) {
    if (results.empty()) return;

    auto firstItem = results.begin();
    updateHeader(dataTable, ColMembraneCapacitance, "Membrane capacitance", firstItem->membraneCapacitance.getFullUnit());
    updateHeader(dataTable, ColAccessResistance, "Access resistance", firstItem->accessResistance.getFullUnit());
    updateHeader(dataTable, ColMembraneResistance, "Membrane resistance", firstItem->membraneResistance.getFullUnit());

    int row = 0;
    for (auto ch : activeChannels) {
        auto &result = results[ch];

        // Convert data to same prefix as first item used as reference
        result.membraneCapacitance.convertValue(firstItem->membraneCapacitance.prefix);
        result.accessResistance.convertValue(firstItem->accessResistance.prefix);
        result.membraneResistance.convertValue(firstItem->membraneResistance.prefix);

        this->setCellText(row, ColMembraneCapacitance, QString("%1").arg(result.membraneCapacitance.value));
        this->setCellText(row, ColAccessResistance, QString("%1").arg(result.accessResistance.value));
        this->setCellText(row, ColMembraneResistance, QString("%1").arg(result.membraneResistance.value));
        row++;
    }
}

void MeasurementsOverviewDockWidget::setStatisticsResultsInRowaRow(int row, StatisticsResult &r) {
    dataTable->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(r.chIdx + 1)));
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

// Updates header labels with unit measure
void MeasurementsOverviewDockWidget::updateHeader(QTableWidget* table, int col, const QString& baseName, const std::string& unit) {
    auto headerItem = table->horizontalHeaderItem(col);
    if (!headerItem) {
        headerItem = new QTableWidgetItem();
        table->setHorizontalHeaderItem(col, headerItem);
    }

    QString newHeaderText = QString("%1 [%2]").arg(baseName, QString::fromStdString(unit));
    if (headerItem->text() != newHeaderText) {
        headerItem->setText(newHeaderText);
        dataTable->resizeColumnToContents(col);
    }
}
