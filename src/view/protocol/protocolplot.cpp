#include "protocolplot.h"

#include <QBitmap>

#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_canvas.h"

#include "protocolwidget.h"

ProtocolPlot::ProtocolPlot(MessageDispatcher * msgDisp, QString titleString, QString xUnitString, QString yUnitString, QWidget * parent) :
    QwtPlot(parent),
    msgDisp(msgDisp) {

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->plotLayout()->setAlignCanvasToScales(true);

    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
    }

    xBottomScaleDraw = new ConversionScaleDraw;
    this->setAxisScaleDraw(QwtPlot::xBottom, xBottomScaleDraw);
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
    connect(zoomInPicker, &QwtPlotPicker::appended, this, &ProtocolPlot::onZoomInPickerAppended);
    connect(zoomInPicker, &QwtPlotPicker::moved, this, &ProtocolPlot::onZoomInPickerMoved);
    connect(zoomInPicker, QOverload <const QRectF &> ::of(&QwtPlotPicker::selected), this, &ProtocolPlot::onZoomInPickerSelected);

    /*! Zoom out picker */
    zoomOutPicker = new QwtPlotPicker(this->canvas());
    zoomOutPicker->setStateMachine(new QwtPickerClickPointMachine());
    zoomOutPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomOutPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomOutPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &ProtocolPlot::onZoomOutPickerSelected);

    /*! Zoom reset picker */
    zoomResetPicker = new QwtPlotPicker(this->canvas());
    zoomResetPicker->setStateMachine(new DoubleClickMachine(Qt::RightButton));
    zoomResetPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomResetPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomResetPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &ProtocolPlot::onZoomResetPickerSelected);

    connect(this, &ProtocolPlot::zoomInRequest, this, &ProtocolPlot::onZoomInRequest);
    connect(this, &ProtocolPlot::zoomOutRequest, this, &ProtocolPlot::onZoomOutRequest);
    connect(this, &ProtocolPlot::zoomResetRequest, this, &ProtocolPlot::onZoomResetRequest);

    /*! Add cursor picker */
    cursorAddPicker = new QwtPlotPicker(this->canvas());
    cursorAddPicker->setStateMachine(new DoubleClickMachine(Qt::LeftButton));
    cursorAddPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    cursorAddPicker->setEnabled(false);
    connect(cursorAddPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &ProtocolPlot::onCursorAddRequest);

    /*! Remove cursor picker */
    cursorRemovePicker = new QwtPlotPicker(this->canvas());
    cursorRemovePicker->setStateMachine(new QwtPickerClickPointMachine());
    cursorRemovePicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    cursorRemovePicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    cursorRemovePicker->setEnabled(false);
    connect(cursorRemovePicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, QOverload <QPointF> ::of(&ProtocolPlot::onCursorOpenPropertiesRequest));

    /*! Cursor move picker */
    cursorMovePicker = new QwtPlotPicker(this->canvas());
    cursorMovePicker->setStateMachine(new QwtPickerDragRectMachine());
    cursorMovePicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    cursorMovePicker->setEnabled(false);
    connect(cursorMovePicker, QOverload <const QRectF &> ::of(&QwtPlotPicker::selected), this, &ProtocolPlot::onCursorMoveRequest);
    connect(cursorMovePicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::moved), this, &ProtocolPlot::onCursorMoving);

    protocolCursors = new QVector <ProtocolCursor *>;

    this->canvas()->setCursor(Qt::CrossCursor);

    xAxisMaxMajor = this->axisMaxMajor(QwtPlot::xBottom);
    yAxisMaxMajor = this->axisMaxMajor(QwtPlot::yLeft);

    yScale = 0.5*this->axisInterval(yLeft).width();

    rangeInitialized.resize(axisCnt);
    rangeInitialized.fill(false);
}

ProtocolPlot::~ProtocolPlot() {
    for (int cursorIdx = 0; cursorIdx < protocolCursors->size(); cursorIdx++) {
        delete protocolCursors->at(cursorIdx);
    }
    protocolCursors->clear();
}

QSize ProtocolPlot::sizeHint() const {
    return QSize(100, 100);
}

QSize ProtocolPlot::minimumSizeHint() const {
    return QSize(100, 100);
}

void ProtocolPlot::drawCanvas(QPainter * p) {
    QwtPlot::drawCanvas(p);
    this->resizeEvent(nullptr);
}

QwtText ProtocolPlot::getPlotTitle() {
    return plotTitle->text();
}

void ProtocolPlot::setPlotTitle(QwtText text) {
    plotTitle->setText(text);
}

void ProtocolPlot::setTitleText(QString text) {
    plotTitle->setPlainText(text);
    QwtText t = plotTitle->text();
    t.setRenderFlags(Qt::AlignRight | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    plotTitle->setText(t);
}

void ProtocolPlot::setXUnit(QwtText text) {
    xUnit->setText(text);
}

QwtText ProtocolPlot::getXUnit() {
    return xUnit->text();
}

void ProtocolPlot::setXUnitText(QString text) {
    xUnit->setPlainText("[" + text + "]");
    QwtText t = xUnit->text();
    t.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    xUnit->setText(t);
}

QString ProtocolPlot::getXUnitText() {
    return xUnit->plainText();
}

void ProtocolPlot::setYUnit(QwtText text) {
    yUnit->setText(text);
}

QwtText ProtocolPlot::getYUnit() {
    return yUnit->text();
}

void ProtocolPlot::setYUnitText(QString text) {
    yUnit->setPlainText("[" + text + "]");
    QwtText t = yUnit->text();
    t.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    yUnit->setText(t);
}

QString ProtocolPlot::getYUnitText() {
    return yUnit->plainText();
}

void ProtocolPlot::pushZoomStack() {
    Rect4 r(this);
    zoomStack.push_back(r);
}

Rect4 ProtocolPlot::popZoomStack() {
    Rect4 r;
    if (!this->isEmptyZoomStack()) {
        r = zoomStack.back();
        zoomStack.pop_back();

    } else {
        r = this->resetZoomStack();
    }
    return r;
}

void ProtocolPlot::clearZoomStack() {
    zoomStack.clear();
}

bool ProtocolPlot::isEmptyZoomStack() {
    return zoomStack.empty();
}

Rect4 ProtocolPlot::resetZoomStack() {
    Rect4 r;
    if (!this->isEmptyZoomStack()) {
        r = zoomStack[0];
        zoomStack.clear();

    } else {
        r[xBottom] = this->axisInterval(xBottom);
        r[yLeft] = this->axisInterval(yLeft);
        if (this->axisEnabled(yRight)) {
            r[yRight] = this->axisInterval(yRight);
        }
    }
    return r;
}

void ProtocolPlot::shiftVertAxis(Axis axis, double shiftValue) {
    this->setAxisScale(axis, this->axisInterval(axis).minValue()+shiftValue, this->axisInterval(axis).maxValue()+shiftValue);
    this->replot();
}

void ProtocolPlot::setProtocol(ProtocolWidget * protocol) {
    QVector <int> cursorsMap;
    this->protocol = protocol;
    this->updateCursorsSections(cursorsMap);
    emit removeCursors(cursorsMap);
}

QVector <ProtocolCursor *> * ProtocolPlot::getProtocolCursors() {
    return protocolCursors;
}

void ProtocolPlot::setCursorsVisibility(bool visible) {
    if (cursorsVisible != visible) {
        cursorsVisible = visible;
        for (int cursorsIdx = 0; cursorsIdx < protocolCursors->size(); cursorsIdx++) {
            protocolCursors->at(cursorsIdx)->setVisible(visible);
        }
    }
}

std::vector <YAML::Cursor> ProtocolPlot::getYamlCursors() {
    std::vector <YAML::Cursor> yamlCursors;
    for (int cursorIdx = 0; cursorIdx < protocolCursors->size(); cursorIdx++) {
        yamlCursors.push_back(protocolCursors->at(cursorIdx)->getYamlCursor());
    }
    return yamlCursors;
}

void ProtocolPlot::setCursorsFromYaml(const std::vector <YAML::Cursor> &yamlCursors) {
    if (protocol != nullptr) {
        for (auto yamlCursor : yamlCursors) {
            this->importCursor(yamlCursor);
        }
    }
    emit addCursors();
}

void ProtocolPlot::onZoomInRequest(Rect4 * rect) {
    if (((rect->at(yLeft).width() == 0.0) && (rect->at(yRight).width() == 0.0)) || (rect->at(xBottom).width() == 0.0)) {
        return;
    }

    Rect4 newRect = * rect;

    this->pushZoomStack();

    this->setAxisScale(xBottom, newRect[xBottom].minValue(), newRect[xBottom].maxValue());
    this->recomputeXAxisFactor(newRect[xBottom].width());
    this->setAxisScale(yLeft, newRect[yLeft].minValue(), newRect[yLeft].maxValue());
    yScale = 0.5*newRect[yLeft].width();
    if (this->axisEnabled(yRight)) {
        this->setAxisScale(yRight, newRect[yRight].minValue(), newRect[yRight].maxValue());
    }

    this->replot();
}

void ProtocolPlot::onHorzZoomInRequest(Rect4 * rect) {
    if ((rect->at(xBottom).width() == 0.0)) {
        return;
    }

    Rect4 newRect = * rect;
    newRect[yLeft] = this->axisInterval(yLeft);
    if (this->axisEnabled(yRight)) {
        newRect[yRight] = this->axisInterval(yRight);
    }

    this->onZoomInRequest(&newRect);
}

void ProtocolPlot::onVertZoomInRequest(Rect4 * rect) {
    if ((rect->at(yLeft).width() == 0.0) && (rect->at(yRight).width() == 0.0)) {
        return;
    }

    Rect4 newRect = * rect;
    newRect[xBottom] = this->axisInterval(xBottom);

    this->onZoomInRequest(&newRect);
}

void ProtocolPlot::onVertZoomFullRequest() {
    this->pushZoomStack();
    this->setAxisAutoScale(yLeft);

    this->replot();

    this->setAxisAutoScale(yLeft, false);
}

void ProtocolPlot::onZoomOutRequest() {
    if (this->isEmptyZoomStack()) {
        this->setAxisScale(yLeft, currentRange[yLeft].min, currentRange[yLeft].max);
        if (this->axisEnabled(yRight)) {
            this->setAxisScale(yRight, currentRange[yRight].min, currentRange[yRight].max);
        }

    } else {
        Rect4 r = this->popZoomStack();

        this->setAxisScale(xBottom, r[xBottom].minValue(), r[xBottom].maxValue());

        this->setAxisScale(yLeft, r[yLeft].minValue(), r[yLeft].maxValue());
        yScale = 0.5*r[yLeft].width();
        if (this->axisEnabled(yRight)) {
            this->setAxisScale(yRight, r[yRight].minValue(), r[yRight].maxValue());
        }
    }
    this->replot();

    this->recomputeXAxisFactor(this->axisInterval(xBottom).width());
}

void ProtocolPlot::onZoomResetRequest() {
    if (this->isEmptyZoomStack()) {
        this->setAxisScale(yLeft, currentRange[yLeft].min, currentRange[yLeft].max);
        if (this->axisEnabled(yRight)) {
            this->setAxisScale(yRight, currentRange[yRight].min, currentRange[yRight].max);
        }

    } else {
        Rect4 r = this->resetZoomStack();

        this->setAxisScale(xBottom, r[xBottom].minValue(), r[xBottom].maxValue());

        this->setAxisScale(yLeft, r[yLeft].minValue(), r[yLeft].maxValue());
        yScale = 0.5*r[yLeft].width();
        if (this->axisEnabled(yRight)) {
            this->setAxisScale(yRight, r[yRight].minValue(), r[yRight].maxValue());
        }
    }
    this->replot();

    this->recomputeXAxisFactor(this->axisInterval(xBottom).width());
}

void ProtocolPlot::onEnableCursorManagement(bool enabled) {
    zoomInPicker->setEnabled(!enabled);
    zoomOutPicker->setEnabled(!enabled);
    zoomResetPicker->setEnabled(!enabled);

    cursorAddPicker->setEnabled(enabled);
    cursorRemovePicker->setEnabled(enabled);
    cursorMovePicker->setEnabled(enabled);

    if (enabled) {
        this->canvas()->setCursor(Qt::OpenHandCursor);

    } else {
        this->canvas()->setCursor(Qt::CrossCursor);
    }
}

void ProtocolPlot::onCursorAddRequest(QPointF p) {
    protocolCursors->append(new ProtocolCursor(msgDisp, this, p.x(), protocolCursors->size()+1));
    double offset;
    ProtocolSection * section = protocol->getItemAtTime(p.x(), 0, offset);
    protocolCursors->back()->setSection(section, offset, protocol->getType());
    connect(protocolCursors->back(), &ProtocolCursor::cursorOpenPropertiesRequest, this, QOverload <int> ::of(&ProtocolPlot::onCursorOpenPropertiesRequest));
    connect(protocolCursors->back(), &ProtocolCursor::propertiesAccepted, this, &ProtocolPlot::onCursorPropertiesAccepted);
    connect(protocolCursors->back(), &ProtocolCursor::cursorDeleteRequest, this, &ProtocolPlot::onCursorDeleteRequest);
    this->replot();
    emit addCursors();
}

void ProtocolPlot::onCursorMoveRequest(QRectF r) {
    this->canvas()->setCursor(Qt::OpenHandCursor);
    if (movingCursor) {
        if (!mouseOnCursorBin) {
            double offset;
            ProtocolSection * section;
            if (cursorMovingLeft) {
                section = protocol->getItemAtTime(r.x(), protocolCursors->at(movedCursorIdx)->getSweepIdx(), offset);

            } else {
                section = protocol->getItemAtTime(r.x()+r.width(), protocolCursors->at(movedCursorIdx)->getSweepIdx(), offset);
            }

            protocolCursors->at(movedCursorIdx)->setSection(section, offset, protocol->getType());
            this->replot();

            emit moveCursors();

        } else {
            this->onCursorDeleteRequest(movedCursorIdx);

            mouseOnCursorBin = false;
            emit openCursorBin(false);
        }
    }
    movingCursor = false;
    movingCursorTry = false;
    movedCursorIdx = -1;
    cursorGrabFail = false;
}

void ProtocolPlot::onCursorMoving(QPointF p) {
    if (!movingCursorTry) {
        if (!cursorGrabFail) {
            int cursorIdx;
            if (this->getClosestCursor(p, cursorIdx)) {
                movingCursorTry = true;
                movingCursor = true;
                movedCursorIdx = cursorIdx;
                cursorInitialPosition = protocolCursors->at(movedCursorIdx)->getXValue();
                this->canvas()->setCursor(Qt::ClosedHandCursor);
                protocolCursors->at(movedCursorIdx)->setXValue(p.x());
                this->replot();

            } else {
                this->canvas()->setCursor(Qt::ForbiddenCursor);
                cursorGrabFail = true;
            }
            mouseOnCursorBin = false;
        }

    } else if (movingCursor) {
        protocolCursors->at(movedCursorIdx)->setXValue(p.x());
        cursorMovingLeft = (p.x() < cursorInitialPosition);
        double x = 0.0;
        double y = transform(QwtPlot::yLeft, p.y()-this->axisInterval(QwtPlot::yLeft).minValue())-
                transform(QwtPlot::yLeft, 0.0);
        if ((!mouseOnCursorBin) && checkOnCursorBin(x, y)) {
            mouseOnCursorBin = true;
            emit openCursorBin(true);

        } else if (mouseOnCursorBin && !checkOnCursorBin(x, y)) {
            mouseOnCursorBin = false;
            emit openCursorBin(false);
        }

        this->replot();
    }
}

void ProtocolPlot::onCursorOpenPropertiesRequest(QPointF p) {
    int cursorIdx;
    if (this->getClosestCursor(p, cursorIdx)) {
        this->onCursorOpenPropertiesRequest(cursorIdx);
    }
}

void ProtocolPlot::onCursorOpenPropertiesRequest(int cursorIdx) {
    if (cursorIdx < 0) {
        cursorIdx += protocolCursors->size();

    } else if (cursorIdx >= protocolCursors->size()) {
        cursorIdx -= protocolCursors->size();
    }

    protocolCursors->at(cursorIdx)->openPropertyDialog();
}

void ProtocolPlot::onCursorPropertiesAccepted() {
    this->replot();
    emit editCursors();
}

void ProtocolPlot::onCursorDeleteRequest(int cursorIdx) {
    QVector <int> cursorsMap(protocolCursors->size());
    delete protocolCursors->at(cursorIdx);
    protocolCursors->remove(cursorIdx);

    for (int idx = 0; idx < cursorIdx; idx++) {
        cursorsMap[idx] = idx;
    }
    cursorsMap[cursorIdx] = -1;

    for (; cursorIdx < protocolCursors->size(); cursorIdx++) {
        protocolCursors->at(cursorIdx)->setCursorIdx(cursorIdx+1);
        cursorsMap[cursorIdx+1] = cursorIdx;
    }

    emit removeCursors(cursorsMap);
    this->replot();
}

void ProtocolPlot::resizeEvent(QResizeEvent * e) {
    if (e != nullptr) {
        QwtPlot::resizeEvent(e);
    }

    int newXAxisMaxMajor = this->width()/40;
    int newYAxisMaxMajor = this->height()/40;
    if ((xAxisMaxMajor != newXAxisMaxMajor) || (yAxisMaxMajor != newYAxisMaxMajor)) {
        xAxisMaxMajor = newXAxisMaxMajor;
        yAxisMaxMajor = newYAxisMaxMajor;

        this->setAxisMaxMajor(xBottom, xAxisMaxMajor);
        this->setAxisMaxMajor(yLeft, yAxisMaxMajor);
        this->setAxisMaxMajor(yRight, yAxisMaxMajor);

        /*! \todo FCON questo accade troppo spesso, perchè il numero di tick non è necessariamente uguale al numero massimo di tick
                       quindi a volte cambia il massimo ma non il numero */
        this->replot();
    }

    QSize siz = plotTitle->minimumSizeHint();
    plotTitle->setGeometry(this->canvas()->x()+this->canvas()->width()-siz.width(), this->canvas()->y(), siz.width(), siz.height());

    siz = xUnit->minimumSizeHint();
    xUnit->setGeometry(this->canvas()->x()+this->canvas()->width()-siz.width(), this->canvas()->y()+this->canvas()->height()-siz.height(), siz.width(), siz.height());

    siz = yUnit->minimumSizeHint();
    yUnit->setGeometry(this->canvas()->x(), this->canvas()->y(), siz.width(), siz.height());
}

void ProtocolPlot::wheelEvent(QWheelEvent * we) {
    Axis vertAxis = yLeft;
    if (this->axisEnabled(yRight) && we->x() > this->width()/2.0) {
        /*! If the right y-axis is enabled and the pointer is on the right side of the plot, scroll the y-axis */
        vertAxis = yRight;
    }
    double p = this->axisInterval(vertAxis).width()*0.05;
    this->shiftVertAxis(vertAxis, we->delta() < 0 ? p : -p);
}

void ProtocolPlot::recomputeXAxisFactor(double duration) {
    sweepDuration.value = duration;
    sweepDuration.prefix = commlib::UnitPfxNone;
    sweepDuration.nice();

    if (xAxisPrefix != sweepDuration.prefix) {
        xAxisPrefix = sweepDuration.prefix;
        xBottomScaleDraw->setConversionFactor(1.0/sweepDuration.multiplier());
        this->setXUnitText(QString::fromStdString(sweepDuration.getFullUnit()));
    }
}

bool ProtocolPlot::importCursor(const YAML::Cursor &yamlCursor) {
    double xvalue = yamlCursor.xvalue;
    int itemIdx = yamlCursor.itemidx;

    protocolCursors->append(new ProtocolCursor(msgDisp, this, xvalue, protocolCursors->size()+1));
    protocolCursors->back()->setCursorFromYaml(yamlCursor);

    double offset;
    ProtocolSection * section = protocol->getItemAtTime(xvalue, itemIdx, protocolCursors->back()->getRepetitionIdx(), protocolCursors->back()->getSweepIdx(), offset);
    if (section != nullptr) {
        protocolCursors->back()->setSection(section, offset, protocol->getType());
        connect(protocolCursors->back(), &ProtocolCursor::cursorOpenPropertiesRequest, this, QOverload <int> ::of(&ProtocolPlot::onCursorOpenPropertiesRequest));
        connect(protocolCursors->back(), &ProtocolCursor::propertiesAccepted, this, &ProtocolPlot::onCursorPropertiesAccepted);
        connect(protocolCursors->back(), &ProtocolCursor::cursorDeleteRequest, this, &ProtocolPlot::onCursorDeleteRequest);

        return true;

    } else {
        return false;
    }
}

void ProtocolPlot::updateCursorsSections(QVector <int> &map) {
    double offset;
    ProtocolSection * section = protocol->getItemAtTime(0.0, 0, offset);
    if (section == nullptr) {
        return;
    }

    int cursorsNum = protocolCursors->size();
    map.resize(cursorsNum);
    int mapIdx = 0;
    for (int cursorIdx = 0; cursorIdx < cursorsNum; cursorIdx++, mapIdx++) {
        if (!protocolCursors->at(cursorIdx)->updateSection(section, protocol->getType())) {
            delete protocolCursors->at(cursorIdx);
            protocolCursors->remove(cursorIdx);
            cursorIdx--;
            cursorsNum--;
            map[mapIdx] = -1;

        } else {
            protocolCursors->at(cursorIdx)->setCursorIdx(cursorIdx+1);
            map[mapIdx] = cursorIdx;
        }
    }
}

bool ProtocolPlot::getClosestCursor(QPointF p, int &cursorIdx) {
    double minDist = std::numeric_limits <double> ::max();
    double dist;
    cursorIdx = -1;

    for (int idx = 0; idx < protocolCursors->size(); idx++) {
        dist = fabs(p.x()-protocolCursors->at(idx)->getXValue());
        if (dist < minDist) {
            cursorIdx = idx;
            minDist = dist;
        }
    }

    return ((cursorIdx >= 0) &&
            (fabs(transform(QwtPlot::xBottom, minDist)-transform(QwtPlot::xBottom, 0)) < 10));
}

bool ProtocolPlot::checkOnCursorBin(double, double y) {
    return (y > 20);
}

void ProtocolPlot::onZoomInPickerAppended(const QPointF &p) {
    pickerFirstCornerPos = p;
    pickerZoomDiscriminantNorm = (this->axisInterval(yLeft).width()*(double)this->canvas()->width())/(this->axisInterval(xBottom).width()*(double)this->canvas()->height());
    zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
    pickerZoomType = PickerZoomRect;
}

void ProtocolPlot::onZoomInPickerMoved(const QPointF &p) {
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

void ProtocolPlot::onZoomInPickerSelected(const QRectF &r) {
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
    emit zoomInRequest(&rect);
}

void ProtocolPlot::onZoomOutPickerSelected(const QPointF &) {
    emit zoomOutRequest();
}

void ProtocolPlot::onZoomResetPickerSelected(const QPointF &) {
    emit zoomResetRequest();
}
