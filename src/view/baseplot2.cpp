#include "baseplot2.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "doubleclickmachine.h"
#include <QColor>
#include <QPen>
#include <QWheelEvent>
#include "qwt_series_data.h"
#include "qwt_scale_map.h"

BasePlot2::BasePlot2(std::shared_ptr<PlotModel> pm, QWidget *parent)
    : QwtPlot{parent}, pm(pm) {
    this->plotLayout()->setAlignCanvasToScales(true);
    for (int axis = 0; axis < axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
    }

    // xBottomScaleDraw = new ConversionScaleDraw;
    // this->setAxisScaleDraw(xBottom, xBottomScaleDraw);
    QwtPlotCanvas * canvas = new QwtPlotCanvas();
    canvas->setFrameStyle(QFrame::NoFrame);
    this->setCanvas(canvas);
    this->setCanvasBackground(Qt::white);

    QwtPlotGrid * grid = new QwtPlotGrid();
    grid->setPen(Qt::lightGray, 0.5);
    grid->setMinorPen(Qt::lightGray, 0.3);
    grid->enableXMin(true);
    grid->attach(this);
    grid->show();

    QFont font;
    font.setPointSize(10);

    /*! Zoom in picker */
    zoomInPicker = new QwtPlotPicker(this->canvas());
    zoomInPicker->setStateMachine(new QwtPickerDragRectMachine());
    zoomInPicker->setTrackerMode(QwtPlotPicker::ActiveOnly);
    zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
    zoomInPicker->setRubberBandPen(QColor(Qt::blue));
    connect(zoomInPicker, &QwtPlotPicker::appended, this, [=](const QPointF &p) {
        emit sigZoomInPickerAppended(p, this->canvas());
        zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
    });
    connect(zoomInPicker, &QwtPlotPicker::moved, this, &BasePlot2::sigZoomInPickerMoved);
    connect(zoomInPicker, QOverload <const QRectF &> ::of(&QwtPlotPicker::selected), this, &BasePlot2::sigZoomInPickerSelected);

    /*! Zoom out picker */
    auto zoomOutPicker = new QwtPlotPicker(this->canvas());
    zoomOutPicker->setStateMachine(new QwtPickerClickPointMachine());
    zoomOutPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomOutPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomOutPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &BasePlot2::sigZoomOut);

    /*! Zoom reset picker */
    auto zoomResetPicker = new QwtPlotPicker(this->canvas());
    zoomResetPicker->setStateMachine(new DoubleClickMachine(Qt::RightButton));
    zoomResetPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomResetPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomResetPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &BasePlot2::sigZoomReset);

    const auto pmLabels = pm->getUnitLabes();
    for (auto l : pmLabels) {
        // enable axis
        this->axisEnabled(l.first);
        // create label
        labels[l.first] = createTextLabel(l.second, l.first);
    }
    handleLabelsPosition();
}

QSize BasePlot2::sizeHint() const {
    return QSize(200, 300);
}

QSize BasePlot2::minimumSizeHint() const {
    return QSize(200, 300);
}

void BasePlot2::drawCanvas(QPainter * p) {
    QwtPlot::drawCanvas(p);
    this->handleLabelsPosition();
}

void BasePlot2::handleLabelsPosition() {
    auto cx = this->canvas()->x();
    auto cy = this->canvas()->y();
    auto cw = this->canvas()->width();
    auto ch = this->canvas()->height();
    QSize siz;
    for (auto &&l : labels) {
        switch (l.first) {
        case QwtPlot::Axis::yRight:
            siz = l.second->minimumSizeHint();
            l.second->setGeometry(cx+cw-siz.width(), cy, siz.width(), siz.height());
            break;
        case QwtPlot::Axis::xBottom:
            siz = l.second->minimumSizeHint();
            l.second->setGeometry(cx+cw-siz.width(), cy+ch-siz.height(), siz.width(), siz.height());
            break;
        case QwtPlot::Axis::yLeft:
            siz = l.second->minimumSizeHint();
            l.second->setGeometry(cx, cy, siz.width(), siz.height());
            break;
        }
    }
}

void BasePlot2::wheelEvent(QWheelEvent * we) {
    /*! If the right y-axis is enabled and the pointer is on the right side of the plot, scroll the y-axis */
    Axis vertAxis = (this->axisEnabled(yRight) && we->position().x() > this->width()/2.0) ? yRight : yLeft;
    auto key = we->modifiers();
    // Get the angle delta of the wheel event
    QPoint angleDelta = we->angleDelta();
    // Check the vertical rotation
    int verticalRotation = angleDelta.y();

    // Get the mouse position in global coordinates
    QPointF globalPos = we->globalPosition();
    // Convert the global mouse position to the plot's local coordinates
    QPoint plotPos = mapFromGlobal(QPoint(globalPos.x(), globalPos.y()));
    // Get the corresponding position in the plot's coordinate system
    QwtPointSeriesData data;
    QwtScaleMap xMap = canvasMap(xBottom);
    QwtScaleMap yMap = canvasMap(vertAxis);
    QPointF plotCoordinates = QPointF(xMap.invTransform(plotPos.x()), yMap.invTransform(plotPos.y()));

    switch (key) {
    case Qt::Modifier::CTRL:
        emit singleAxisZoomRequest(vertAxis, verticalRotation, plotCoordinates);
        break;

    case Qt::Modifier::SHIFT:
        emit singleAxisZoomRequest(xBottom, verticalRotation, plotCoordinates);
        break;

    default:
        emit singleAxisShiftRequest(vertAxis, verticalRotation);
        break;
    }
}

void BasePlot2::resizeEvent(QResizeEvent * e) {
    if (e != nullptr) {
        QwtPlot::resizeEvent(e);
    }

    this->handleLabelsPosition();
}

QwtTextLabel* BasePlot2::createTextLabel(std::string l, QwtPlot::Axis axis) {
    QFont font;
    font.setPointSize(10);
    QwtTextLabel* a = new QwtTextLabel(this);
    a->setText(QString::fromStdString("[" + l + "]"));

    Qt::AlignmentFlag alignment = axis== QwtPlot::Axis::yRight ? Qt::AlignmentFlag::AlignRight : Qt::AlignmentFlag::AlignLeft;
    QwtText t = a->text();
    t.setRenderFlags(alignment | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);

    a->setFont(font);
    a->setMargin(0);
    a->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    return a;
}

void BasePlot2::updateRect() {
    auto r = pm->getZoom();
    for (int i =0; i < QwtPlot::Axis::axisCnt; i++) {
        this->setAxisScale(i, r[i].minValue(), r[i].maxValue());
    }
}

void BasePlot2::onRubberBandUpdated() {
    zoomInPicker->setRubberBand(pm->getRubberBand().value_or(QwtPlotPicker::RubberBand::NoRubberBand));
    replot();
}

void BasePlot2::onReplot(){
    updateRect();
    replot();
}
