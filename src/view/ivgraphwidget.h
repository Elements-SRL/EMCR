#ifndef IVGRAPHWIDGET_H
#define IVGRAPHWIDGET_H

#include <QDockWidget>
#include <QBoxLayout>
#include <QPushButton>
#include "copyabletable.h"
#include <tuple>
#include <map>
#include <QHeaderView>
#include <QString>

class IvGraphWidget : public QDockWidget
{
    Q_OBJECT
private:
    QTableWidget * dataTable;

public:
    IvGraphWidget(uint32_t channelsNum, QWidget *parent = nullptr);
    void setParams(std::map<uint32_t, std::vector<double>> params);

signals:
    void exportIvGraph();
    void calcMeanSquared();
};

#endif // IVGRAPHWIDGET_H
