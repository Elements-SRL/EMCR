#include "ivgraphwidget.h"

IvGraphWidget::IvGraphWidget(QWidget *parent) : QDockWidget(parent) {
    this->setObjectName("ivGraph");
    this->setWindowTitle("Iv Graph");
    auto mainVl = new QVBoxLayout();
    this->setLayout(mainVl);
    auto exportButton = new QPushButton("Export to csv", this);
    mainVl->addWidget(exportButton);
    connect(exportButton, &QPushButton::clicked, this, [=](){
        emit exportIvGraph();
    });
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}
