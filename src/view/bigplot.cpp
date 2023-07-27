#include "bigplot.h"

#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_canvas.h"

BigPlot::BigPlot(QString titleString, QString xUnitString, QString yUnitString, QWidget * parent) :
    QwtPlot(parent) {

    this->plotLayout()->setAlignCanvasToScales(true);

    for (int axis = 0; axis < axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
    }

    xBottomScaleDraw = new ConversionScaleDraw;
    this->setAxisScaleDraw(xBottom, xBottomScaleDraw);
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

    plotTitle = new QwtTextLabel(this);
    plotTitle->setText(titleString);
    plotTitle->setFont(font);
    plotTitle->setMargin(0);
    plotTitle->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    xUnit = new QwtTextLabel(this);
    xUnit->setText(xUnitString);
    xUnit->setFont(font);
    xUnit->setMargin(0);
    xUnit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    yUnit = new QwtTextLabel(this);
    yUnit->setText(yUnitString);
    yUnit->setFont(font);
    yUnit->setMargin(0);
    yUnit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    /*! Zoom in picker */
    zoomInPicker = new QwtPlotPicker(this->canvas());
    zoomInPicker->setStateMachine(new QwtPickerDragRectMachine());
    zoomInPicker->setTrackerMode(QwtPlotPicker::ActiveOnly);
    zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
    zoomInPicker->setRubberBandPen(QColor(Qt::blue));
    connect(zoomInPicker, &QwtPlotPicker::appended, this, &BigPlot::onZoomInPickerAppended);
    connect(zoomInPicker, &QwtPlotPicker::moved, this, &BigPlot::onZoomInPickerMoved);
    connect(zoomInPicker, QOverload <const QRectF &> ::of(&QwtPlotPicker::selected), this, &BigPlot::onZoomInPickerSelected);

    /*! Zoom out picker */
    zoomOutPicker = new QwtPlotPicker(this->canvas());
    zoomOutPicker->setStateMachine(new QwtPickerClickPointMachine());
    zoomOutPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomOutPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomOutPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &BigPlot::onZoomOutPickerSelected);

    /*! Zoom reset picker */
    zoomResetPicker = new QwtPlotPicker(this->canvas());
    zoomResetPicker->setStateMachine(new DoubleClickMachine(Qt::RightButton));
    zoomResetPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomResetPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomResetPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &BigPlot::onZoomResetPickerSelected);

    this->canvas()->setCursor(Qt::CrossCursor);

//    connect(this, &BigPlot::zoomInRequest, this, &BigPlot::onZoomInRequest);
//    connect(this, &BigPlot::zoomOutRequest, this, &BigPlot::onZoomOutRequest);
//    connect(this, &BigPlot::zoomResetRequest, this, &BigPlot::onZoomResetRequest);

    xAxisMaxMajor = this->axisMaxMajor(xBottom);
    yAxisMaxMajor = this->axisMaxMajor(yLeft);

    yScale = 0.5*this->axisInterval(yLeft).width();

    rangeInitialized.resize(axisCnt);
    rangeInitialized.fill(false);
}

QSize BigPlot::sizeHint() const {
    return QSize(200, 300);
}

QSize BigPlot::minimumSizeHint() const {
    return QSize(200, 300);
}

void BigPlot::setTitle(QString text) {
    plotTitle->setPlainText(text);
    QwtText t = plotTitle->text();
    t.setRenderFlags(Qt::AlignRight | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    plotTitle->setText(t);
}

void BigPlot::setXUnit(QString text) {
    xUnit->setPlainText("[" + text + "]");
    QwtText t = xUnit->text();
    t.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    xUnit->setText(t);
}

void BigPlot::setYUnit(QString text) {
    yUnit->setPlainText("[" + text + "]");
    QwtText t = yUnit->text();
    t.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    yUnit->setText(t);
}

Rect4 BigPlot::getRect() {
    Rect4 r(this);
    return r;
}

void BigPlot::setRect(Rect4 r) {
    if (((r.at(yLeft).width() == 0.0) && (r.at(yRight).width() == 0.0)) || (r.at(xBottom).width() == 0.0)) {
        return;
    }
    this->setAxisScale(xBottom, r[xBottom].minValue(), r[xBottom].maxValue());
    this->setAxisScale(yLeft, r[yLeft].minValue(), r[yLeft].maxValue());
    yScale = 0.5*r[yLeft].width();
    if (this->axisEnabled(yRight)) {
        this->setAxisScale(yRight, r[yRight].minValue(), r[yRight].maxValue());
    }
    this->replot();
    this->recomputeXAxisFactor(this->axisInterval(xBottom).width());
}

void BigPlot::shiftVertAxis(Axis axis, double shiftValue) {
    this->setAxisScale(axis, this->axisInterval(axis).minValue()+shiftValue, this->axisInterval(axis).maxValue()+shiftValue);
    this->replot();
}


void BigPlot::onHorzZoomInRequest(Rect4 * rect) {
    if ((rect->at(xBottom).width() == 0.0)) {
        return;
    }

    Rect4 newRect = * rect;
    newRect[yLeft] = this->axisInterval(yLeft);
    if (this->axisEnabled(yRight)) {
        newRect[yRight] = this->axisInterval(yRight);
    }
    emit zoomInRequest(newRect);
}

void BigPlot::onVertZoomInRequest(Rect4 * rect) {
    if ((rect->at(yLeft).width() == 0.0) && (rect->at(yRight).width() == 0.0)) {
        return;
    }

    Rect4 newRect = * rect;
    newRect[xBottom] = this->axisInterval(xBottom);
    emit zoomInRequest(newRect);
}

void BigPlot::onVertZoomFullRequest() {
    this->setAxisAutoScale(yLeft);

    this->replot();

    this->setAxisAutoScale(yLeft, false);
}

void BigPlot::onUpdateBaseline(Axis axisIdx, double baseline) {
    this->setAxisScale(axisIdx, baseline-yScale, baseline+yScale);
}

void BigPlot::onRangeUpdated(commlib::RangedMeasurement_t newRange, Axis axisIdx) {
    if (rangeInitialized[axisIdx]) {
        if (newRange != currentRange[axisIdx]) {
            currentRange[axisIdx].max = 1.0;
            currentRange[axisIdx].convertValues(newRange.prefix);
            double coeff = currentRange[axisIdx].max;
            currentRange[axisIdx].max = newRange.max;
            currentRange[axisIdx].min = newRange.min;

            double min = coeff*this->axisInterval(axisIdx).minValue();
            double max = coeff*this->axisInterval(axisIdx).maxValue();

            this->setAxisScale(axisIdx, min, max);
            if (axisIdx == yLeft) {
                yScale = 0.5*coeff*this->axisInterval(axisIdx).width();
                this->setYUnit(QString::fromStdString(currentRange[axisIdx].getFullUnit()));

            } else {
                this->setTitle(QString::fromStdString(currentRange[axisIdx].getFullUnit()));
            }
        }

    } else {
        currentRange[axisIdx] = newRange;
        this->setAxisScale(axisIdx, currentRange[axisIdx].min, currentRange[axisIdx].max);
        if (axisIdx == yLeft) {
            yScale = 0.5*currentRange[axisIdx].delta();
            this->setYUnit(QString::fromStdString(currentRange[axisIdx].getFullUnit()));

        } else {
            /*! If both y-axis are defined the second unit goes into title */
            this->setTitle(QString::fromStdString(currentRange[axisIdx].getFullUnit()));
        }
        rangeInitialized[axisIdx] = true;
    }
    this->replot();
}

void BigPlot::onDurationUpdated(commlib::Measurement_t duration) {
    sweepDuration = duration;
    sweepDuration.convertValue(commlib::UnitPfxNone);
    this->setAxisScale(xBottom, 0.0, sweepDuration.value);
    this->recomputeXAxisFactor(sweepDuration.value);

    this->replot();
}

void BigPlot::resizeEvent(QResizeEvent * e) {
    if (e != nullptr) {
        QwtPlot::resizeEvent(e);
    }

    QSize siz = plotTitle->minimumSizeHint();
    plotTitle->setGeometry(this->canvas()->x()+this->canvas()->width()-siz.width(), this->canvas()->y(), siz.width(), siz.height());

    siz = xUnit->minimumSizeHint();
    xUnit->setGeometry(this->canvas()->x()+this->canvas()->width()-siz.width(), this->canvas()->y()+this->canvas()->height()-siz.height(), siz.width(), siz.height());

    siz = yUnit->minimumSizeHint();
    yUnit->setGeometry(this->canvas()->x(), this->canvas()->y(), siz.width(), siz.height());
}

void BigPlot::wheelEvent(QWheelEvent * we) {
    Axis vertAxis = yLeft;
    if (this->axisEnabled(yRight) && we->x() > this->width()/2.0) {
        /*! If the right y-axis is enabled and the pointer is on the right side of the plot, scroll the y-axis */
        vertAxis = yRight;
    }
    double p = this->axisInterval(vertAxis).width()*0.05;
    this->shiftVertAxis(vertAxis, we->delta() < 0 ? p : -p);
}

void BigPlot::recomputeXAxisFactor(double duration) {
    sweepDuration.value = duration;
    sweepDuration.prefix = commlib::UnitPfxNone;
    sweepDuration.nice();

    if (xAxisPrefix != sweepDuration.prefix) {
        xAxisPrefix = sweepDuration.prefix;
        xBottomScaleDraw->setConversionFactor(1.0/sweepDuration.multiplier());
        this->setXUnit(QString::fromStdString(sweepDuration.getFullUnit()));
    }
}

void BigPlot::onZoomInPickerAppended(const QPointF &p) {
    pickerFirstCornerPos = p;
    pickerZoomDiscriminantNorm = (this->axisInterval(yLeft).width()*(double)this->canvas()->width())/(this->axisInterval(xBottom).width()*(double)this->canvas()->height());
    zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
    pickerZoomType = PickerZoomRect;
}

void BigPlot::onZoomInPickerMoved(const QPointF &p) {
    QPointF deltaP = pickerFirstCornerPos-p;
    double zoomDiscriminantRatio = abs(deltaP.x()/deltaP.y())*pickerZoomDiscriminantNorm;
    if (zoomDiscriminantRatio < 0.1) {
        zoomInPicker->setRubberBand(QwtPlotPicker::VLineRubberBand);
        pickerZoomType = PickerZoomVert;

    } else if (zoomDiscriminantRatio > 10.0) {
        zoomInPicker->setRubberBand(QwtPlotPicker::HLineRubberBand);
        pickerZoomType = PickerZoomHorz;

    } else {
        zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
        pickerZoomType = PickerZoomRect;
    }
}

void BigPlot::onZoomInPickerSelected(const QRectF &r) {
    Rect4 rect(r);
    double yL = this->axisInterval(yLeft).minValue();
    double hL = this->axisInterval(yLeft).width();
    double yR = this->axisInterval(yRight).minValue();
    double hR = this->axisInterval(yRight).width();
    double rightLeftRatio = hR/hL;
    double rightOffset = yR+(r.y()-yL)*rightLeftRatio;
    double rightHeight = r.height()*rightLeftRatio;
    switch (pickerZoomType) {
    case PickerZoomRect:
        if (this->axisEnabled(yRight)) {
            rect[yRight].setInterval(rightOffset, rightOffset+rightHeight);
        }
        break;

    case PickerZoomHorz:
        rect[yLeft].setInterval(this->axisInterval(yLeft).minValue(), this->axisInterval(yLeft).maxValue());
        rect[yRight].setInterval(this->axisInterval(yRight).minValue(), this->axisInterval(yRight).maxValue());
        break;

    case PickerZoomVert:
        rect[xBottom].setInterval(this->axisInterval(xBottom).minValue(), this->axisInterval(xBottom).maxValue());
        if (this->axisEnabled(yRight)) {
            rect[yRight].setInterval(rightOffset, rightOffset+rightHeight);
        }
        break;
    }
    emit zoomInRequest(rect);
}

void BigPlot::onZoomOutPickerSelected(const QPointF &) {
    emit zoomOutRequest();
}

void BigPlot::onZoomResetPickerSelected(const QPointF &) {
    qDebug()<<"ciccia";
    emit zoomResetRequest();
}
