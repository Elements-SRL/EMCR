#include "plot.h"

#include <QBitmap>

#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_canvas.h"

#include "protocolwidget.h"

Plot::Plot(QWidget * parent, QString titleString, QString unitString) :
    QwtPlot(parent) {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->plotLayout()->setAlignCanvasToScales(true);

    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
    }

    xBottomScaleDraw = new ConversionScaleDraw;
    this->setAxisScaleDraw(QwtPlot::xBottom, xBottomScaleDraw);

//    this->enableAxis(QwtPlot::yLeft, false);
//    yLeftScaleItem = new QwtPlotScaleItem(QwtScaleDraw::RightScale, 0);
////    yLeftScaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
//    yLeftScaleItem->attach(this);

//    this->enableAxis(QwtPlot::xBottom, false);
//    xBottomScaleItem = new QwtPlotScaleItem(QwtScaleDraw::TopScale, 0);
//    xBottomScaleItem->attach(this);

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

    title = new QwtTextLabel(this);
    title->setText(titleString);
    title->setFont(font);
    title->setMargin(0);
    title->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    xUnit = new QwtTextLabel(this);
    xUnit->setText(unitString);
    xUnit->setFont(font);
    xUnit->setMargin(0);
    xUnit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    yUnit = new QwtTextLabel(this);
    yUnit->setText(unitString);
    yUnit->setFont(font);
    yUnit->setMargin(0);
    yUnit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    /*! Zoom in picker */
    zoomInPicker = new QwtPlotPicker(this->canvas());
    zoomInPicker->setStateMachine(new QwtPickerDragRectMachine());
    zoomInPicker->setTrackerMode(QwtPlotPicker::ActiveOnly);
    zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
    zoomInPicker->setRubberBandPen(QColor(Qt::blue));
    connect(zoomInPicker, &QwtPlotPicker::appended, this, [=] (QPointF p) {
        pickerFirstCornerPos = p;
        pickerZoomDiscriminantNorm = (this->axisInterval(QwtPlot::yLeft).width()*(double)this->canvas()->width())/
                (this->axisInterval(QwtPlot::xBottom).width()*(double)this->canvas()->height());
        zoomInPicker->setRubberBand(QwtPlotPicker::RectRubberBand);
        pickerZoomType = PickerZoomRect;
    });
    connect(zoomInPicker, &QwtPlotPicker::moved, this, [=] (QPointF p) {
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
    });
    connect(zoomInPicker, QOverload <const QRectF &> ::of(&QwtPlotPicker::selected), this, [=] (QRectF r) {
        switch (pickerZoomType) {
        case PickerZoomRect:
            this->onZoomInRequest(r);
            break;

        case PickerZoomHorz:
            /*! \todo FCON non mi è chiaro perchè deo settare il valore minimo al top ed il massimo al bottom */
            r.setTop(this->axisInterval(QwtPlot::yLeft).minValue());
            r.setBottom(this->axisInterval(QwtPlot::yLeft).maxValue());
            this->onZoomInRequest(r);
            break;

        case PickerZoomVert:
            r.setLeft(this->axisInterval(QwtPlot::xBottom).minValue());
            r.setRight(this->axisInterval(QwtPlot::xBottom).maxValue());
            this->onZoomInRequest(r);
            break;
        }
    });

    /*! Zoom out picker */
    zoomOutPicker = new QwtPlotPicker(this->canvas());
    zoomOutPicker->setStateMachine(new QwtPickerClickPointMachine());
    zoomOutPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomOutPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomOutPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, [=] () {
        this->onZoomOutRequest();
    });

    /*! Zoom reset picker */
    zoomResetPicker = new QwtPlotPicker(this->canvas());
    zoomResetPicker->setStateMachine(new DoubleClickMachine(Qt::RightButton));
    zoomResetPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    zoomResetPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(zoomResetPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, [=] () {
        this->onZoomResetRequest();
    });

    this->canvas()->setCursor(Qt::CrossCursor);

    xAxisMaxMajor = this->axisMaxMajor(QwtPlot::xBottom);
    yAxisMaxMajor = this->axisMaxMajor(QwtPlot::yLeft);
}

Plot::~Plot() {

}

QSize Plot::sizeHint() const {
    return QSize(100, 100);
}

QSize Plot::minimumSizeHint() const {
    return QSize(100, 100);
}

void Plot::drawCanvas(QPainter * p) {
    QwtPlot::drawCanvas(p);
    this->resizeEvent(nullptr);
}

void Plot::setTitle(QString text) {
    title->setPlainText(text);
    QwtText t = title->text();
    t.setRenderFlags(Qt::AlignRight | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    title->setText(t);
}

void Plot::setXUnit(QString text) {
    xUnit->setPlainText("[" + text + "]");
    QwtText t = xUnit->text();
    t.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    xUnit->setText(t);
}

QString Plot::getXUnit() {
    return xUnit->plainText();
}

void Plot::setYUnit(QString text) {
    yUnit->setPlainText("[" + text + "]");
    QwtText t = yUnit->text();
    t.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    yUnit->setText(t);
}

QString Plot::getYUnit() {
    return yUnit->plainText();
}

void Plot::pushZoomStack(QRectF r) {
    zoomStack.push_back(r);
}

void Plot::pushZoomStack() {
    QRectF r;
    r.setX(this->axisInterval(QwtPlot::xBottom).minValue());
    r.setWidth(this->axisInterval(QwtPlot::xBottom).width());
    r.setY(this->axisInterval(QwtPlot::yLeft).minValue());
    r.setHeight(this->axisInterval(QwtPlot::yLeft).width());

    this->pushZoomStack(r);
}

QRectF Plot::popZoomStack() {
    QRectF r = zoomStack.back();
    zoomStack.pop_back();
    return r;
}

void Plot::clearZoomStack() {
    zoomStack.clear();
}

bool Plot::isEmptyZoomStack() {
    return zoomStack.empty();
}

QRectF Plot::resetZoomStack() {
    QRectF r;
    if (!this->isEmptyZoomStack()) {
        r = zoomStack[0];
        zoomStack.clear();

    } else {
        r = QRectF(0.0, 0.0, 1000.0, 1000.0);
    }
    return r;
}

void Plot::onZoomInRequest(QRectF rect, bool synchOtherPlots) {
    if ((rect.height() == 0.0) || (rect.width() == 0.0)) {
        return;
    }

    this->pushZoomStack();

    if (xAxisBlocked) {
        rect.setX(this->axisInterval(QwtPlot::xBottom).minValue());
        rect.setWidth(this->axisInterval(QwtPlot::xBottom).width());
    }

    this->setAxisScale(QwtPlot::xBottom, rect.x(), rect.x()+rect.width());
    this->recomputeXAxisFactor(rect.width());
    this->setAxisScale(QwtPlot::yLeft, rect.y(), rect.y()+rect.height());

    this->replot();

    if (synchOtherPlots) {
        emit xAxisZoomed(rect.x(), rect.width());
    }
}

//void Plot::replot() {
//    QwtPlot::replot();

//    yLeftScaleItem->setPosition(this->axisInterval(QwtPlot::xBottom).minValue());
//    xBottomScaleItem->setPosition(this->axisInterval(QwtPlot::yLeft).minValue());
//}

void Plot::onHorzZoomInRequest(QRectF rect, bool synchOtherPlots) {
    if ((rect.width() == 0.0) || xAxisBlocked) {
        return;
    }

    rect.setY(this->axisInterval(QwtPlot::yLeft).minValue());
    rect.setHeight(this->axisInterval(QwtPlot::yLeft).width());

    this->onZoomInRequest(rect, synchOtherPlots);
}

void Plot::onVertZoomInRequest(QRectF rect, bool synchOtherPlots) {
    if (rect.height() == 0.0) {
        return;
    }

    rect.setX(this->axisInterval(QwtPlot::xBottom).minValue());
    rect.setWidth(this->axisInterval(QwtPlot::xBottom).width());

    this->onZoomInRequest(rect, synchOtherPlots);
}

void Plot::onVertZoomFullRequest(bool synchOtherPlots) {
    this->pushZoomStack();
    this->setAxisAutoScale(QwtPlot::yLeft);

    this->replot();

    this->setAxisAutoScale(QwtPlot::yLeft, false);

    QRectF rect;
    rect.setWidth(this->axisInterval(QwtPlot::xBottom).width());
    rect.setY(this->axisInterval(QwtPlot::yLeft).minValue());
    rect.setHeight(this->axisInterval(QwtPlot::yLeft).width());

    if (synchOtherPlots) {
        emit xAxisZoomed(rect.x(), rect.width());
    }
}

void Plot::onZoomOutRequest(bool synchOtherPlots) {
    if (this->isEmptyZoomStack()) {
        this->setAxisAutoScale(QwtPlot::xBottom);
        this->setAxisAutoScale(QwtPlot::yLeft);
        this->replot();

        if (!autoScaleOnEmptyZoomStack) {
            QwtInterval xInt = this->axisInterval(QwtPlot::xBottom);
            QwtInterval yInt = this->axisInterval(QwtPlot::yLeft);
            this->setAxisScale(QwtPlot::xBottom, xInt.minValue(), xInt.maxValue());
            this->setAxisScale(QwtPlot::yLeft, yInt.minValue(), yInt.maxValue());
        }

    } else {
        QRectF r = this->popZoomStack();

        if (!xAxisBlocked) {
            this->setAxisScale(QwtPlot::xBottom, r.x(), r.x() + r.width());
        }

        this->setAxisScale(QwtPlot::yLeft, r.y(), r.y() + r.height());
    }
    this->replot();

    this->recomputeXAxisFactor(this->axisInterval(QwtPlot::xBottom).width());

    if (synchOtherPlots) {
        double minValue = this->axisInterval(QwtPlot::xBottom).minValue();
        double width = this->axisInterval(QwtPlot::xBottom).width();
        emit zoomPopped(minValue, width);
    }
}

void Plot::onZoomResetRequest(bool synchOtherPlots) {
    if (this->isEmptyZoomStack()) {
        this->setAxisAutoScale(QwtPlot::xBottom);
        this->setAxisAutoScale(QwtPlot::yLeft);

        if (!autoScaleOnEmptyZoomStack) {
            QwtInterval xInt = this->axisInterval(QwtPlot::xBottom);
            QwtInterval yInt = this->axisInterval(QwtPlot::yLeft);
            this->setAxisScale(QwtPlot::xBottom, xInt.minValue(), xInt.maxValue());
            this->setAxisScale(QwtPlot::yLeft, yInt.minValue(), yInt.maxValue());
        }

    } else {
        QRectF r = this->resetZoomStack();

        if (!xAxisBlocked) {
            this->setAxisScale(QwtPlot::xBottom, r.x(), r.x() + r.width());
        }

        this->setAxisScale(QwtPlot::yLeft, r.y(), r.y() + r.height());
    }
    this->replot();

    this->recomputeXAxisFactor(this->axisInterval(QwtPlot::xBottom).width());

    if (synchOtherPlots) {
        double minValue = this->axisInterval(QwtPlot::xBottom).minValue();
        double width = this->axisInterval(QwtPlot::xBottom).width();
        emit zoomReset(minValue, width);
    }
}

void Plot::recomputeXAxisFactor(double) {
    /*! Implemented only for some derived classes */
    return;
}

void Plot::resizeEvent(QResizeEvent * e) {
    if (e != nullptr) {
        QwtPlot::resizeEvent(e);
    }

    int newXAxisMaxMajor = this->width()/40;
    int newYAxisMaxMajor = this->height()/40;
    if ((xAxisMaxMajor != newXAxisMaxMajor) || (yAxisMaxMajor != newYAxisMaxMajor)) {
        xAxisMaxMajor = newXAxisMaxMajor;
        yAxisMaxMajor = newYAxisMaxMajor;

        this->setAxisMaxMajor(QwtPlot::xBottom, xAxisMaxMajor);
        this->setAxisMaxMajor(QwtPlot::yLeft, yAxisMaxMajor);
        this->setAxisMaxMajor(QwtPlot::yRight, yAxisMaxMajor);

        /*! \todo FCON questo accade troppo spesso, perchè il numero di tick non è necessariamente uguale al numero massimo di tick
                       quindi a volte cambia il massimo ma non il numero */
        this->replot();
    }

    QSize siz = title->minimumSizeHint();
    title->setGeometry(this->canvas()->x()+this->canvas()->width()-siz.width(), this->canvas()->y(), siz.width(), siz.height());

    siz = xUnit->minimumSizeHint();
    xUnit->setGeometry(this->canvas()->x()+this->canvas()->width()-siz.width(), this->canvas()->y()+this->canvas()->height()-siz.height(), siz.width(), siz.height());

    siz = yUnit->minimumSizeHint();
    yUnit->setGeometry(this->canvas()->x(), this->canvas()->y(), siz.width(), siz.height());
}

void Plot::wheelEvent(QWheelEvent * we) {
    QwtPlot::Axis vertAxis = QwtPlot::yLeft;
    double p = this->axisInterval(vertAxis).width()*0.05;
    this->shiftVertAxis(vertAxis, we->delta() < 0 ? p : -p);
}

void Plot::shiftVertAxis(QwtPlot::Axis axis, double shiftValue) {
    this->setAxisScale(axis, this->axisInterval(axis).minValue()+shiftValue, this->axisInterval(axis).maxValue()+shiftValue);
    this->replot();
}

void Plot::blockXAxis(bool flag) {
    xAxisBlocked = flag;
}

ProtocolPlot::ProtocolPlot(e4gcl::CommLib * commLib, QWidget * parent, QString titleString, QString unitString) :
    Plot(parent, titleString, unitString),
    commLib(commLib) {

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

    autoScaleOnEmptyZoomStack = true;
}

ProtocolPlot::~ProtocolPlot() {
    for (int cursorIdx = 0; cursorIdx < protocolCursors->size(); cursorIdx++) {
        delete protocolCursors->at(cursorIdx);
    }
    protocolCursors->clear();
}

bool ProtocolPlot::importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) {
    int depth = EPML_CURSOR_DEPTH;
    QString tag = "cursor";
    while (epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
        if (protocol != nullptr) {
            if (!(this->importCursor(epmlManager, epmlStatus))) {
                return false;
            }
        }
    }
    emit addCursors();
    return true;
}

bool ProtocolPlot::exportEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) {
    int depth = EPML_CURSOR_DEPTH;
    QString tag = "cursor";

    for (int cursorIdx = 0; cursorIdx < protocolCursors->size(); cursorIdx++) {
        if (!(epmlManager->createSection(tag, depth, parentTag, epmlStatus))) {
            return false;
        }
        if (!(protocolCursors->at(cursorIdx)->exportEpml(epmlManager, epmlStatus))) {
            return false;
        }
    }
    return true;
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
    protocolCursors->append(new ProtocolCursor(commLib, this, p.x(), protocolCursors->size()+1));
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

bool ProtocolPlot::importCursor(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) {
    int depth = EPML_CURSOR_PARAM_DEPTH;
    QString parentTag = "cursor";

    QString tag = "xvalue";

    if (!epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
        epmlStatus = EpmlSyntaxError;
        return false;
    }

    double xvalue = epmlManager->getDouble(tag, depth, epmlStatus);
    if (epmlStatus != EpmlValueFound) {
        epmlStatus = EpmlSyntaxError;
        return false;
    }

    tag = "itemidx";

    if (!epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
        epmlStatus = EpmlSyntaxError;
        return false;
    }

    int itemIdx = epmlManager->getInt(tag, depth, epmlStatus);
    if (epmlStatus != EpmlValueFound) {
        epmlStatus = EpmlSyntaxError;
        return false;
    }

    protocolCursors->append(new ProtocolCursor(commLib, this, xvalue, protocolCursors->size()+1));
    if (!(protocolCursors->back()->importEpml(epmlManager, epmlStatus))) {
        return false;
    }

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
    double minDist = numeric_limits <double> ::max();
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

ProtocolDataPlot::ProtocolDataPlot(int channelIdx, QWidget * parent, QString titleString, QString unitString) :
    Plot(parent, titleString, unitString),
    channelIdx(channelIdx) {

    /*! Rise trigger picker */
    riseTriggerPicker = new QwtPlotPicker(this->canvas());
    riseTriggerPicker->setEnabled(false);
    riseTriggerPicker->setStateMachine(new QwtPickerClickPointMachine());
    riseTriggerPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    riseTriggerPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton);
    connect(riseTriggerPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, [=] (QPointF p) {
        /*! Must use anonymous function because second argument's default value of the slot does not work */
        this->onRiseTriggerRequest(p);
    });

    /*! Fall trigger picker */
    fallTriggerPicker = new QwtPlotPicker(this->canvas());
    fallTriggerPicker->setEnabled(false);
    fallTriggerPicker->setStateMachine(new QwtPickerClickPointMachine());
    fallTriggerPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    fallTriggerPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(fallTriggerPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, [=] (QPointF p) {
        /*! Must use anonymous function because second argument's default value of the slot does not work */
        this->onFallTriggerRequest(p);
    });

    QBitmap bitmap = QBitmap::fromImage(QImage(":/imgs/cursor rise.png"), Qt::ThresholdDither);
    triggerCursor = new QCursor(bitmap, bitmap);

    triggerMarkerHorz = new QwtPlotMarker();
    triggerMarkerHorz->setLineStyle(QwtPlotMarker::HLine);
    triggerMarkerHorz->setLinePen(QColor("green"), 1.0, Qt::DashLine);
    triggerMarkerHorz->attach(this);
    triggerMarkerHorz->setVisible(false);

    triggerMarkerVert = new QwtPlotMarker();
    triggerMarkerVert->setLineStyle(QwtPlotMarker::VLine);
    triggerMarkerVert->setLinePen(QColor("green"), 1.0, Qt::DashLine);
    triggerMarkerVert->attach(this);
    triggerMarkerVert->setVisible(false);

    zoomInPicker->setEnabled(true);
    zoomOutPicker->setEnabled(true);
    zoomResetPicker->setEnabled(true);

    this->canvas()->setCursor(Qt::CrossCursor);
}

ProtocolDataPlot::~ProtocolDataPlot() {

}

void ProtocolDataPlot::initializeRange(e4gcl::RangedMeasurement_t newRange) {
    /*! \todo FCON range0 è salvato nel caso si voglia combinare qualcosa con il range settato inizialmente,
                   tipo settarlo come valore di default quando si fa un reset degli assi */
    range0 = newRange;
    currentRange = range0;
    this->setAxisScale(QwtPlot::yLeft, currentRange.min, currentRange.max);
    this->setYUnit(QString::fromStdString(currentRange.getFullUnit()));
}

void ProtocolDataPlot::onEnableTriggerPicker() {
    zoomInPicker->setEnabled(false);
    zoomOutPicker->setEnabled(false);
    zoomResetPicker->setEnabled(false);
    riseTriggerPicker->setEnabled(true);
    fallTriggerPicker->setEnabled(true);
    this->canvas()->setCursor(* triggerCursor);
}

void ProtocolDataPlot::onDisableTriggerPicker() {
    zoomInPicker->setEnabled(true);
    zoomOutPicker->setEnabled(true);
    zoomResetPicker->setEnabled(true);
    riseTriggerPicker->setEnabled(false);
    fallTriggerPicker->setEnabled(false);
    this->canvas()->setCursor(Qt::CrossCursor);
}

void ProtocolDataPlot::onEnableTriggerMarker(double x, double y) {
    triggerX = x/this->axisInterval(QwtPlot::xBottom).width();
    triggerY = y;
    triggerMarkerHorz->setYValue(y);
    triggerMarkerVert->setXValue(x);

    triggerMarkerHorz->setVisible(true);
    triggerMarkerVert->setVisible(true);

    this->replot();
}

void ProtocolDataPlot::onDisableTriggerMarker() {
    triggerMarkerHorz->setVisible(false);
    triggerMarkerVert->setVisible(false);

    this->replot();
}

void ProtocolDataPlot::onRangeUpdated(e4gcl::RangedMeasurement_t newRange) {
    if (newRange != currentRange) {
        currentRange.max = 1.0;
        currentRange.convertValues(newRange.prefix);
        double coeff = currentRange.max;
        currentRange = newRange;

        double min = coeff*this->axisInterval(QwtPlot::yLeft).minValue();
        double max = coeff*this->axisInterval(QwtPlot::yLeft).maxValue();

        this->setAxisScale(QwtPlot::yLeft, min, max);
        this->setYUnit(QString::fromStdString(currentRange.getFullUnit()));

        if (triggerMarkerHorz->isVisible()) {
            triggerY *= coeff;
            triggerMarkerHorz->setYValue(triggerY);
            emit updateTriggerY(triggerY);
        }
    }
    this->replot();
}

void ProtocolDataPlot::onDurationUpdated(double duration) {
    this->setAxisScale(QwtPlot::xBottom, 0.0, duration);
    this->recomputeXAxisFactor(duration);

    if (triggerMarkerVert->isVisible()) {
        double x = triggerX*duration;
        triggerMarkerVert->setXValue(x);
        emit updateTriggerX(x);
    }

    this->replot();
}

void ProtocolDataPlot::onRiseTriggerRequest(QPointF p, bool notifyPlotTab) {
    emit triggerEnabled(RiseTrigger, p.x(), p.y(), channelIdx, notifyPlotTab);
    this->onEnableTriggerMarker(p.x(), p.y());

    emit disableTriggerPicker();
}

void ProtocolDataPlot::onFallTriggerRequest(QPointF p, bool notifyPlotTab) {
    emit triggerEnabled(FallTrigger, p.x(), p.y(), channelIdx, notifyPlotTab);
    this->onEnableTriggerMarker(p.x(), p.y());

    emit disableTriggerPicker();
}

void ProtocolDataPlot::recomputeXAxisFactor(double duration) {
    sweepDuration.value = duration;
    sweepDuration.prefix = e4gcl::UnitPfxNone;
    sweepDuration.nice();

    if (xAxisPrefix != sweepDuration.prefix) {
        xAxisPrefix = sweepDuration.prefix;
        xBottomScaleDraw->setConversionFactor(1.0/sweepDuration.multiplier());
        this->setXUnit(QString::fromStdString(sweepDuration.getFullUnit()));
    }
}

AnalysisPlot::AnalysisPlot(QWidget * parent, QString titleString, QString unitString) :
    Plot(parent, titleString, unitString) {

    autoScaleOnEmptyZoomStack = true;
}

AnalysisPlot::~AnalysisPlot() {

}

Curve::Curve(CurveType_t curveType, double size) :
    QwtPlotCurve(),
    curveType(curveType),
    size(size) {

    color = QColor(Qt::blue);
    QwtSymbol * symbol;
    switch (curveType) {
    case CurveTypePlotSolid:
        this->setPen(color, size, Qt::SolidLine);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypePlotFaint:
        color = QColor(Qt::darkGray);
        this->setPen(color, size, Qt::SolidLine);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeAnalysisDashed:
        color = QColor(Qt::red);
        this->setPen(color, size, Qt::DashLine);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeProtocolPreviewSolid:
        this->setPen(color, size, Qt::SolidLine);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeProtocolPreviewDotted:
        this->setPen(color, size, Qt::DotLine);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeProtocolPreviewDashed:
        this->setPen(color, size, Qt::DashLine);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeScatterPlot:
        this->setPen(color, size, Qt::NoPen);

        symbol = new QwtSymbol(QwtSymbol::Ellipse);
        symbol->setPen(color, 3.0);
        symbol->setBrush(QBrush(color));
        symbol->setSize(9);
        break;

    default:
        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
    }
    this->setSymbol(symbol);
}

Curve::Curve(const Curve &curve) :
    Curve(curve.getCurveType(), curve.getSize()) {

    QVector <QPointF> data((int)(curve.dataSize()));
    for (int pointIdx = 0; pointIdx < (int)(curve.dataSize()); pointIdx++) {
        data[pointIdx] = curve.sample(pointIdx);
    }
    this->setSamples(data);
}

CurveType_t Curve::getCurveType() const {
    return curveType;
}

double Curve::getSize() const {
    return size;
}

void Curve::setColor(QColor color) {
    this->color = color;
    QPen pen = this->pen();
    pen.setColor(color);
    this->setPen(pen);

    QwtSymbol * symbol = const_cast <QwtSymbol *> (this->symbol());
    switch (curveType) {
    case CurveTypeScatterPlot:
        symbol->setPen(color, 3.0);
        symbol->setBrush(QBrush(color));
        break;

    default:
        break;
    }
    this->setSymbol(symbol);
}

void Curve::setColor(CurveType_t type) {
    switch (type) {
    case CurveTypePlotSolid:
        color = QColor(Qt::blue);
        break;

    case CurveTypePlotFaint:
        color = QColor(Qt::darkGray);
        break;

    case CurveTypeAnalysisDashed:
        color = QColor(Qt::red);
        break;

    case CurveTypeProtocolPreviewSolid:
        color = QColor(Qt::blue);
        break;

    case CurveTypeProtocolPreviewDotted:
        color = QColor(Qt::blue);
        break;

    case CurveTypeProtocolPreviewDashed:
        color = QColor(Qt::blue);
        break;

    case CurveTypeScatterPlot:
        color = QColor(Qt::blue);
        break;

    default:
        color = QColor(Qt::blue);
    }

    this->setColor(color);
}

QColor Curve::getColor() {
    return color;
}

HistogramCurve::HistogramCurve() {
    this->setStyle(QwtPlotHistogram::Outline);
    this->setSymbol(nullptr);

    QPen pen(Qt::blue, 0);
    QColor color(Qt::blue);
    this->setBrush(QBrush(color));
    this->setPen(pen);

    series = new QwtIntervalSeriesData;
    samples = new QVector <QwtIntervalSample>;
}

HistogramCurve::~HistogramCurve() {
    /*! Series is destroyed by the dtor of QwtPlotHistogram */

    if (samples != nullptr) {
        delete samples;
        samples = nullptr;
    }
}

void HistogramCurve::drawHist(int binsNum, double lowerBound, double binWidth, QVector <int> histCount) {
    samples->resize(binsNum);
    double offset = lowerBound;
    for (int binIdx = 0; binIdx < binsNum; binIdx++) {
        QwtInterval interval(offset, offset+binWidth);
        offset += binWidth;
        interval.setBorderFlags(QwtInterval::ExcludeMaximum); /*! \todo FCON io ci vorrei mettere includeborders */

        samples->replace(binIdx, QwtIntervalSample(histCount[binIdx], interval));
    }

    series->setSamples(* samples);
    this->setData(series);
}

ConversionScaleDraw::ConversionScaleDraw(double conversionFactor) :
    conversionFactor(conversionFactor) {
}

void ConversionScaleDraw::setConversionFactor(double value) {
    conversionFactor = value;
    this->invalidateCache();
}

QwtText ConversionScaleDraw::label(double value) const {
    return QwtScaleDraw::label(value*conversionFactor);
}

DoubleClickMachine::DoubleClickMachine(Qt::MouseButton btn) :
    QwtPickerMachine(PointSelection),
    btn(btn) {
}

QList <QwtPickerMachine::Command> DoubleClickMachine::transition(const QwtEventPattern &, const QEvent * event) {
    QList <QwtPickerMachine::Command> cmdList;
    if ((event->type() == QEvent::MouseButtonDblClick) &&
            (((const QMouseEvent *)event)->button() == btn)) {
        cmdList += Begin;
        cmdList += Append;
        cmdList += End;
    }

    return cmdList;
}
