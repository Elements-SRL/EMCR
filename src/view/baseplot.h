#ifndef BASEPLOT_H
#define BASEPLOT_H

#include "qwt_plot.h"
#include "qwt_text_label.h"

class BasePlot : public QwtPlot {
    Q_OBJECT

public:
    BasePlot(std::string title, std::string xUnit, std::string yUnit, QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setLabel(std::string, QwtPlot::Axis);
    void drawCanvas(QPainter* p) override;
    BasePlot* getPlot();

protected:
    virtual void resizeEvent(QResizeEvent* e) override;

private:
    QwtTextLabel* title;
    QwtTextLabel* xUnit;
    QwtTextLabel* yUnit;

    QwtTextLabel* produceTextLabel(std::string, Qt::AlignmentFlag = Qt::AlignLeft);
    void setAndFormatText(std::string text, QwtTextLabel* label, Qt::AlignmentFlag = Qt::AlignLeft);
    void handleLabelsPosition();
};

#endif // PLOT_H
