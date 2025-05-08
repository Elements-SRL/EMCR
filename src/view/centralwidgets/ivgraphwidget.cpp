#include "ivgraphwidget.h"
#include <QSplitter>

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
    splitter->setHandleWidth(20);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    auto buttonsHl = new QHBoxLayout();

    auto startButton = new QPushButton(this);
    startButton->setIcon(QIcon(QPixmap(":/imgs/start protocol.png")));
    startButton->setToolTip("Start the iv graph analysis if it was previously stopped");
    startButton->setIconSize(QSize(30, 30));
    startButton->setFixedSize(32, 32);
    buttonsHl->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &IvGraphWidget::sigStartIvGraph);

    auto stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(QPixmap(":/imgs/stop protocol.png")));
    stopButton->setToolTip("Stop the iv graph analysis if it was previously strted");
    stopButton->setIconSize(QSize(30, 30));
    stopButton->setFixedSize(32, 32);
    buttonsHl->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &IvGraphWidget::sigStopIvGraph);

    auto exportButton = new QPushButton(this);
    exportButton->setIcon(QIcon(QPixmap(":/imgs/export protocol.png")));
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

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonsHl->addWidget(spacer);

    mainVl->addLayout(buttonsHl);
    dataTable = new CopyableTable(this);
    dataTable->setColumnCount(9);
    dataTable->setRowCount(channelsNum + 1);
    dataTable->horizontalHeader()->hide();
    dataTable->verticalHeader()->hide();
    mainVl->addWidget(dataTable);
    this->installEventFilter(dataTable);
    dataTable->setItem(0, 0, new QTableWidgetItem("Channel index"));
    dataTable->setItem(0, 1, new QTableWidgetItem("Conductance"));
    dataTable->setItem(0, 2, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, 3, new QTableWidgetItem("Resistance"));
    dataTable->setItem(0, 4, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, 5, new QTableWidgetItem("Inversion potential"));
    dataTable->setItem(0, 6, new QTableWidgetItem("Unit"));
    dataTable->setItem(0, 7, new QTableWidgetItem("Current offset"));
    dataTable->setItem(0, 8, new QTableWidgetItem("Unit"));

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
    for (auto &entry: params) {
        const auto key = entry.first;
        const auto values = entry.second;
        const auto tabRow = key + 1;
        dataTable->setItem(tabRow, 0, new QTableWidgetItem(QString::fromStdString(std::to_string(tabRow))));
        for (uint32_t i=0; i<values.size(); i++) {
//            for each data we have both a value and a prefix and the offset is due to the chIdx column
            auto idx = i * 2 + 1;
            auto m = values[i];
            dataTable->setItem(tabRow, idx, new QTableWidgetItem(QString::fromStdString(std::to_string(m.value))));
            dataTable->setItem(tabRow, idx + 1, new QTableWidgetItem(QString::fromStdString(m.getFullUnit())));
        }
    }
}
