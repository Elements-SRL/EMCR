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
    auto subWidget = new QWidget(splitter);
    auto mainVl = new QVBoxLayout(subWidget); // Set layout on the container widget
    splitter->addWidget(plot);
    splitter->addWidget(subWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    auto buttonsHl = new QHBoxLayout();

    auto startButton = new QPushButton(this);
    startButton->setObjectName("startButton");
    startButton->setToolTip("Start the iv graph analysis if it was previously stopped");
    startButton->setIconSize(QSize(30, 30));
    startButton->setFixedSize(32, 32);
    buttonsHl->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &IvGraphWidget::sigStartIvGraph);

    auto stopButton = new QPushButton(this);
    stopButton->setObjectName("stopButton");
    stopButton->setToolTip("Stop the iv graph analysis if it was previously started");
    stopButton->setIconSize(QSize(30, 30));
    stopButton->setFixedSize(32, 32);
    buttonsHl->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &IvGraphWidget::sigStopIvGraph);

    auto exportButton = new QPushButton(this);
    exportButton->setObjectName("exportCsvButton");
    exportButton->setToolTip("Export to csv");
    exportButton->setIconSize(QSize(30, 30));
    exportButton->setFixedSize(32, 32);
    buttonsHl->addWidget(exportButton);
    connect(exportButton, &QPushButton::clicked, this, [=](){
        emit sigExportIvGraph();
    });

    auto calcLineButton = new QPushButton(this);
    calcLineButton->setIcon(QIcon(QPixmap(":/imgs/analysis linear fit.png")));
    calcLineButton->setToolTip("Calculate least square line");
    calcLineButton->setIconSize(QSize(30, 30));
    calcLineButton->setFixedSize(32, 32);
    buttonsHl->addWidget(calcLineButton);
    connect(calcLineButton, &QPushButton::clicked, this, [=](){
        emit sigCalcMeanSquared();
    });

    auto infoBox = new QLabel();
    infoBox->setObjectName("infoBox");
    infoBox->setToolTip(QString("The table shows only the expanded channels"));
    infoBox->setFixedSize(14, 14);
    buttonsHl->addWidget(infoBox);

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonsHl->addWidget(spacer);

    mainVl->addLayout(buttonsHl);
    dataTable = new CopyableTable(this);
    dataTable->setColumnCount(4);
    dataTable->setRowCount(channelsNum);

    mainVl->addWidget(dataTable);
    this->installEventFilter(dataTable);

    QHeaderView * vHeader = dataTable->verticalHeader();
    vHeader->setObjectName("tableVHeader");
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    dataTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Conductance"));
    dataTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Resistance"));
    dataTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Inversion potential"));
    dataTable->setHorizontalHeaderItem(3, new QTableWidgetItem("Current offset"));


    auto zoomButtonsHl = new QHBoxLayout();

    auto autoZoomButton = new QPushButton(this);
    autoZoomButton->setIcon(QIcon(QPixmap(":/imgs/zoom full.png")));
    autoZoomButton->setToolTip("Auto zoom");
    autoZoomButton->setIconSize(QSize(30, 30));
    autoZoomButton->setFixedSize(32, 32);
    zoomButtonsHl->addWidget(autoZoomButton);
    connect(autoZoomButton, &QPushButton::clicked, this, &IvGraphWidget::sigAutoZoom);

    {
        auto spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        zoomButtonsHl->addWidget(spacer);
    }
    mainVl->addLayout(zoomButtonsHl);
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
