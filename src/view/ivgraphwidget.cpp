#include "ivgraphwidget.h"

IvGraphWidget::IvGraphWidget(uint32_t channelsNum, QWidget *) {
    this->setObjectName("ivGraph");
    this->setWindowTitle("Iv Graph");

    auto containerWidget = new QWidget(this); // Create a container widget
    auto mainVl = new QVBoxLayout(containerWidget); // Set layout on the container widget

    auto buttonsHl = new QHBoxLayout();

    auto startButton = new QPushButton(this);
    startButton->setIcon(QIcon(QPixmap(":/imgs/start protocol.png")));
    startButton->setToolTip("Start the iv graph analysis if it was previously stopped");
    buttonsHl->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &IvGraphWidget::startIvGraph);

    auto stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(QPixmap(":/imgs/stop protocol.png")));
    stopButton->setToolTip("Stop the iv graph analysis if it was previously strted");
    buttonsHl->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &IvGraphWidget::stopIvGraph);

    auto exportButton = new QPushButton(this);
    exportButton->setIcon(QIcon(QPixmap(":/imgs/export protocol.png")));
    exportButton->setToolTip("Export to csv");
    buttonsHl->addWidget(exportButton);
    connect(exportButton, &QPushButton::clicked, this, [=](){
        emit exportIvGraph();
    });

    auto calcLineButton = new QPushButton(this);
    calcLineButton->setIcon(QIcon(QPixmap(":/imgs/analysis linear fit.png")));
    calcLineButton->setToolTip("Calculate least square line");
    buttonsHl->addWidget(calcLineButton);
    connect(calcLineButton, &QPushButton::clicked, this, [=](){
        emit calcMeanSquared();
    });

    mainVl->addLayout(buttonsHl);
    dataTable = new CopyableTable(containerWidget);
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
    setWidget(containerWidget);
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
