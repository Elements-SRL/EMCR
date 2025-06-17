#ifndef IVGRAPHWIDGET_H
#define IVGRAPHWIDGET_H

#include <tuple>
#include <map>

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QString>

#include <QTableWidget>
#include "e384commlib_global_addendum.h"
#include <qwt_plot.h>

class IvGraphWidget : public QWidget {
    Q_OBJECT

private:
    QTableWidget * dataTable;
    bool status = false;

public:
    IvGraphWidget(uint32_t channelsNum, QwtPlot* plot, QWidget * parent = nullptr);
    void setParams(std::map <uint32_t, std::vector <e384CommLib::Measurement_t>> params);

signals:
    void sigExportIvGraph();
    void sigCalcMeanSquared();
    void sigStartIvGraph();
    void sigStopIvGraph();
    void sigAutoZoom();
};

#endif // IVGRAPHWIDGET_H
