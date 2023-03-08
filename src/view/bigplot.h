#ifndef BIGPLOT_H
#define BIGPLOT_H

#include <QWheelEvent>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "qwt_plot_grid.h"
#include "qwt_text_label.h"

class Rect4;
class ConversionScaleDraw;

class BigPlot : public QwtPlot {
    Q_OBJECT

public:
    BigPlot(QString titleString, QString xUnitString, QString yUnitString, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void drawCanvas(QPainter * p) override;

    QwtText getPlotTitle();
    void setPlotTitle(QwtText text);
    void setTitleText(QString text);
    void setXUnit(QwtText text);
    QwtText getXUnit();
    void setXUnitText(QString text);
    QString getXUnitText();
    void setYUnit(QwtText text);
    QwtText getYUnit();
    void setYUnitText(QString text);
    QString getYUnitText();
    void pushZoomStack();
    Rect4 popZoomStack();
    void clearZoomStack();
    bool isEmptyZoomStack();
    virtual Rect4 resetZoomStack();
    void shiftVertAxis(Axis axis, double shiftValue);

public slots:
    void onZoomInRequest(Rect4 * rect);
    void onHorzZoomInRequest(Rect4 * rect);
    void onVertZoomInRequest(Rect4 * rect);
    void onVertZoomFullRequest();
    void onZoomOutRequest();
    void onZoomResetRequest();
    void onUpdateBaseline(Axis axisIdx, double baseline);

protected:
    typedef enum {
        PickerZoomRect,
        PickerZoomHorz,
        PickerZoomVert
    } PickerZoomType_t;

    virtual void resizeEvent(QResizeEvent * e) override;
    virtual void wheelEvent(QWheelEvent * e) override;
    virtual void recomputeXAxisFactor(double duration);

    QwtPlotPicker * zoomInPicker;
    QwtPlotPicker * zoomOutPicker;
    QwtPlotPicker * zoomResetPicker;
    PickerZoomType_t pickerZoomType = PickerZoomRect;
    double pickerZoomDiscriminantNorm = 1.0;

    QwtTextLabel * plotTitle;
    QwtTextLabel * xUnit;
    QwtTextLabel * yUnit;

    int xAxisMaxMajor;
    int yAxisMaxMajor;

    double yScale;

    QPointF pickerFirstCornerPos;

    ConversionScaleDraw * xBottomScaleDraw;

protected slots:
    virtual void onZoomInPickerAppended(const QPointF &p);
    virtual void onZoomInPickerMoved(const QPointF &p);
    virtual void onZoomInPickerSelected(const QRectF &r) = 0;
    virtual void onZoomOutPickerSelected(const QPointF &p) = 0;
    virtual void onZoomResetPickerSelected(const QPointF &p) = 0;

private:
    QVector <Rect4> zoomStack;

signals:
    void zoomInRequest(Rect4 * rect);
    void zoomOutRequest();
    void zoomResetRequest();
};

class Rect4 : public QVector <QwtInterval> {
public:
    Rect4(QwtPlot * plot);
    Rect4(QRectF rect);
    Rect4();

    Rect4 & operator = (const Rect4 &other);
};

class ConversionScaleDraw : public QwtScaleDraw{
public:
    ConversionScaleDraw(double conversionFactor = 1.0);

    void setConversionFactor(double value);
    QwtText label(double value) const override;

private:
    double conversionFactor;
};

class DoubleClickMachine: public QwtPickerMachine {
public:
    DoubleClickMachine(Qt::MouseButton btn);

    virtual QList <Command> transition(const QwtEventPattern &, const QEvent * event);

private:
    Qt::MouseButton btn;
};

#endif // BIGPLOT_H
