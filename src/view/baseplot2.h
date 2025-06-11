#ifndef BASEPLOT2_H
#define BASEPLOT2_H

#include <QObject>
#include "qwt_plot.h"
#include "qwt_text_label.h"
#include <memory>
#include "plotmodel.h"
#include <map>

class BasePlot2 : public QwtPlot
{
    Q_OBJECT
private:
    // QwtTextLabel * xUnit;
    // QwtTextLabel * yLeftUnit;
    // QwtTextLabel * yRightUnit;

    void handleLabelsPosition();
    std::shared_ptr<PlotModel> pm;
    std::map<QwtPlot::Axis, QwtTextLabel*> labels;
    QwtTextLabel* createTextLabel(std::string, QwtPlot::Axis);

private slots:
    // void onZoomInPickerAppended(const QPointF &p);
    // void ongZoomInPickerMoved(const QPointF &p);
    // void ongZoomInPickerSelected(const QRectF &r);

protected:
    virtual void resizeEvent(QResizeEvent * e) override;
    virtual void wheelEvent(QWheelEvent * e) override;

public:
    explicit BasePlot2(std::shared_ptr<PlotModel> pm, QWidget *parent = nullptr);

    void updateRect();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void drawCanvas(QPainter * p) override;

signals:
    void sigZoomInPickerAppended(const QPointF &p);
    void sigZoomInPickerMoved(const QPointF &p);
    void sigZoomInPickerSelected(const QRectF &r);
    void sigZoomOut();
    void sigZoomReset();
};

#endif // BASEPLOT2_H
