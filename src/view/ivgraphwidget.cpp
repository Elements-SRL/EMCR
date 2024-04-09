#include "ivgraphwidget.h"

IvGraphWidget::IvGraphWidget(uint32_t channelsNum, QWidget *parent) {
    this->setObjectName("ivGraph");
    this->setWindowTitle("Iv Graph");

    auto containerWidget = new QWidget(this); // Create a container widget
    auto mainVl = new QVBoxLayout(containerWidget); // Set layout on the container widget

    auto exportButton = new QPushButton("Export to csv");
    mainVl->addWidget(exportButton);
    connect(exportButton, &QPushButton::clicked, this, [=](){
        emit exportIvGraph();
    });
    auto calcLineButton = new QPushButton("Calculate least square line");
    mainVl->addWidget(calcLineButton);
    connect(calcLineButton, &QPushButton::clicked, this, [=](){
        emit calcMeanSquared();
    });

    dataTable = new CopyableTable(containerWidget);
    dataTable->setColumnCount(5);
    dataTable->setRowCount(channelsNum + 1);
    dataTable->horizontalHeader()->hide();
    dataTable->verticalHeader()->hide();
    mainVl->addWidget(dataTable);
    this->layout()->addWidget(containerWidget);
    dataTable->setItem(0, 0, new QTableWidgetItem("Channel index"));
    dataTable->setItem(0, 1, new QTableWidgetItem("Conductance"));
    dataTable->setItem(0, 2, new QTableWidgetItem("Resistance"));
    dataTable->setItem(0, 3, new QTableWidgetItem("Inversion potential"));
    dataTable->setItem(0, 4, new QTableWidgetItem("Current offset"));

    setWidget(containerWidget);
}

void IvGraphWidget::setParams(std::map<uint32_t, std::vector<double>> params) {
    for (auto &entry: params) {
        const auto key = entry.first;
        const auto values = entry.second;
        const auto tabRow = key + 1;
        dataTable->setItem(tabRow, 0, new QTableWidgetItem(QString::fromStdString(std::to_string(key))));
        for (uint32_t i=0; i<values.size(); i++) {
            dataTable->setItem(tabRow, i + 1, new QTableWidgetItem(QString::fromStdString(std::to_string(values[i]))));
        }
    }
}
