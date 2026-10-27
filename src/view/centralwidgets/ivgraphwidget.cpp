#include "ivgraphwidget.h"
#include <QSplitter>
#include <qlabel.h>

IvGraphWidget::IvGraphWidget(uint32_t channelsNum, BigPlot* plot, QWidget * parent) :
    QWidget(parent) {

    this->setObjectName("ivGraph");
    this->setWindowTitle("Iv Graph");

    auto outerLayout = new QVBoxLayout(this);
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    outerLayout->addWidget(splitter);
    auto sideWidget = new QWidget(splitter);
    auto mainLayout = new QVBoxLayout(sideWidget);
    mainLayout->setSpacing(12);

    splitter->addWidget(plot);
    splitter->addWidget(sideWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);

    // --- Section 1. CONTROLS
    auto controlsTitle = new QLabel("ANALYSIS CONTROLS", sideWidget);
    controlsTitle->setObjectName("controlsTitle");
    mainLayout->addWidget(controlsTitle);

    auto controlsRow = new QHBoxLayout();
    controlsRow->setAlignment(Qt::AlignLeft);
    controlsRow->setSpacing(8);
    mainLayout->addLayout(controlsRow);

    auto startButton = new QPushButton(this);
    startButton->setObjectName("startButton");
    startButton->setToolTip("Start the iv graph analysis if it was previously stopped");
    startButton->setIconSize(QSize(30, 30));
    startButton->setFixedSize(32, 32);
    controlsRow->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &IvGraphWidget::sigStartIvGraph);

    auto stopButton = new QPushButton(this);
    stopButton->setObjectName("stopButton");
    stopButton->setToolTip("Stop the iv graph analysis if it was previously started");
    stopButton->setIconSize(QSize(30, 30));
    stopButton->setFixedSize(32, 32);
    controlsRow->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &IvGraphWidget::sigStopIvGraph);

    // Vertical Separator
    QFrame* vline = new QFrame(sideWidget);
    vline->setObjectName("lineSeparator");
    vline->setFrameShape(QFrame::VLine);
    controlsRow->addWidget(vline);

    auto btnAutoZoom = new QPushButton("   Zoom");
    btnAutoZoom->setObjectName("autoZoom");
    btnAutoZoom->setToolTip("Auto zoom");
    btnAutoZoom->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnAutoZoom->setCursor(Qt::PointingHandCursor);
    connect(btnAutoZoom, &QPushButton::clicked, this, &IvGraphWidget::sigAutoZoom);

    controlsRow->addWidget(btnAutoZoom);

    // --- Section 2. LEAST SQUARE TABLE

    // Separator
    QFrame* line = new QFrame(sideWidget);
    line->setObjectName("lineSeparator");
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    auto tableTitle = new QLabel("Linear Fit Table", sideWidget);
    tableTitle->setObjectName("controlsTitle");
    mainLayout->addWidget(tableTitle);

    auto tableControls = new QHBoxLayout();
    tableControls->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    tableControls->setSpacing(6);
    mainLayout->addLayout(tableControls);

    auto exportButton = new QPushButton(this);
    exportButton->setObjectName("exportCsvButton");
    exportButton->setText(" EXPORT");
    exportButton->setToolTip("Export to csv");
    exportButton->setFixedSize(80, 32);
    connect(exportButton, &QPushButton::clicked, this, [=](){
        emit sigExportIvGraph();
    });

    auto calcLineButton = new QPushButton(this);
    calcLineButton->setIcon(QIcon(QPixmap(":/imgs/analysis linear fit.png")));
    calcLineButton->setToolTip("Calculate least square line");
    calcLineButton->setIconSize(QSize(30, 30));
    calcLineButton->setFixedSize(32, 32);
    connect(calcLineButton, &QPushButton::clicked, this, [=](){
        emit sigCalcMeanSquared();
    });

    auto infoBox = new QLabel();
    infoBox->setObjectName("infoBox");
    infoBox->setToolTip(QString("The table shows only the expanded channels"));
    infoBox->setFixedSize(12, 12);

    tableControls->addWidget(exportButton);
    tableControls->addWidget(calcLineButton);
    tableControls->addWidget(infoBox);

    // --- TABLE
    dataTable = new CopyableTable(this);
    dataTable->setColumnCount(4);
    dataTable->setRowCount(channelsNum);

    mainLayout->addWidget(dataTable);
    this->installEventFilter(dataTable);

    QHeaderView * vHeader = dataTable->verticalHeader();
    vHeader->setObjectName("tableVHeader");
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    dataTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Conductance"));
    dataTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Resistance"));
    dataTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Inversion potential"));
    dataTable->setHorizontalHeaderItem(3, new QTableWidgetItem("Current offset"));

}

void IvGraphWidget::setParams(std::map<uint32_t, std::vector<Measurement>> params) {

    // Reset vertical header
    for (int i = 0; i < dataTable->rowCount(); ++i) {
        dataTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i + 1)));
    }

    if (params.empty()) {
        // Empty table
        dataTable->setRowCount(1);
        dataTable->clearContents();
        return;
    }

    dataTable->clearContents();
    dataTable->clear();
    dataTable->setRowCount(params.size());


    // Creating horizontalHeader with unit measures taken from first data element
    const auto& firstRowMeasurements = params.begin()->second;
    std::vector<std::string> baseNames = { "Conductance", "Resistance", "Inversion potential", "Current offset" };

    for (auto col = 0; col < firstRowMeasurements.size() && col < 4; ++col) {
        auto temp = firstRowMeasurements[col];
        auto headerText = baseNames[col] + " [" + temp.getFullUnit() + "]";
        dataTable->setHorizontalHeaderItem(col, new QTableWidgetItem(QString::fromStdString(headerText)));
    }

    // Table shows data for active channels only.
    // The shown order follows channels selection.

    auto row = 0;
    for (auto &entry: params) {
        const auto chIdx = entry.first;
        const auto values = entry.second;

        // vHeader is set with chIdx
        dataTable->setVerticalHeaderItem(row, new QTableWidgetItem(QString::fromStdString(std::to_string(chIdx + 1))));

        for (uint32_t i=0; i<values.size(); i++) {
            auto m = values[i];

            // Converting row data to the same unit measure chosen for the column
            UnitPfx_t targetPrefix = firstRowMeasurements[i].prefix;
            m.convertValue(targetPrefix);

            dataTable->setItem(row, i, new QTableWidgetItem(QString::fromStdString(std::to_string(m.value))));
        }
        row++;
    }
}
