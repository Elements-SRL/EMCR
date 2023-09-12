#include "stampplot.h"

#include "qwt_plot_layout.h"
#include "qwt_plot_canvas.h"
#include <QApplication>
#include "globaldefines.h"

StampPlot::StampPlot(int channelIdx, int idealPlotWidth, int idealPlotHeight, QWidget * parent) :
    QwtPlot(parent),
    channelIdx(channelIdx),
    idealPlotWidth(idealPlotWidth),
    idealPlotHeight(idealPlotHeight) {

    this->plotLayout()->setAlignCanvasToScales(true);

    for (int axis = 0; axis < axisCnt; axis++) {
        this->axisWidget(axis)->setMargin(0);
        this->enableAxis(axis, false);
    }

    QwtPlotCanvas * canvas = new QwtPlotCanvas();
    canvas->setFrameStyle(QFrame::NoFrame);
    this->setCanvas(canvas);
    this->setCanvasBackground(Qt::black);

    QFont font;
    font.setPointSize(7);

    QwtText text;
    text.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    text.setColor(Qt::white);

    channelIdxLbl = new QwtTextLabel(this);
    text.setText(QString("%1").arg(channelIdx+1));
    channelIdxLbl->setText(text);
    channelIdxLbl->setFont(font);
    channelIdxLbl->setMargin(0);
    channelIdxLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    stateLbl = new QwtTextLabel(this);
    text.setText("");
    stateLbl->setText(text);
    stateLbl->setFont(font);
    stateLbl->setMargin(0);
    stateLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    /*! select picker */
    selectPicker = new QwtPlotPicker(this->canvas());
    selectPicker->setStateMachine(new QwtPickerClickPointMachine());
    selectPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    selectPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton);
    connect(selectPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &StampPlot::onClicked);

    /*! deselect picker */
    deselectPicker = new QwtPlotPicker(this->canvas());
    deselectPicker->setStateMachine(new QwtPickerClickPointMachine());
    deselectPicker->setTrackerMode(QwtPlotPicker::AlwaysOff);
    deselectPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(deselectPicker, QOverload <const QPointF &> ::of(&QwtPlotPicker::selected), this, &StampPlot::onUnclicked);

    rangeInitialized.resize(axisCnt);
    rangeInitialized.fill(false);

    xAxisMaxMajor = this->axisMaxMajor(xBottom);
    yAxisMaxMajor = this->axisMaxMajor(yLeft);

    selected = false;
    this->setStyleSheet(STP_STYLE_PLOT_INACTIVE);
}

QSize StampPlot::sizeHint() const {
    return QSize(idealPlotWidth, idealPlotHeight);
}

QSize StampPlot::minimumSizeHint() const {
    return QSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
}

void StampPlot::setSelected(bool flag) {
    if (flag != selected) {
        selected = flag;
        if (selected) {
            this->setStyleSheet(STP_STYLE_PLOT_ACTIVE);

        } else {
            this->setStyleSheet(STP_STYLE_PLOT_INACTIVE);
        }
    }
}

void StampPlot::mousePressEvent(QMouseEvent *event){
    emit clicked(event);
//    if (event->button() == Qt::LeftButton){
//        emit clicked(true);
//    } else {
//        emit clicked(false);
//    }
    // Call the base class implementation for normal processing
    QwtPlot::mousePressEvent(event);
}

void StampPlot::setState(States_t newState) {
    state = newState;
    QString stateText = "";
    bool anyLabelAssigned = false;
    if (state & StateSwitchedOff) {
        if (anyLabelAssigned) {
            stateText += ",";
        }
        stateText += "O";
        anyLabelAssigned = true;
    }

    if (state & StateStimuliDisabled) {
        if (anyLabelAssigned) {
            stateText += ",";
        }
        stateText += "X";
        anyLabelAssigned = true;
    }

    if (state & StateOffsetCompensation) {
        if (anyLabelAssigned) {
            stateText += ",";
        }
        stateText += "C";
        anyLabelAssigned = true;
    }

    if (state & StateTraceExpanded) {
        if (anyLabelAssigned) {
            stateText += ",";
        }
        stateText += "E";
        anyLabelAssigned = true;
    }

    stateLbl->setText(stateText);
    this->resizeEvent(nullptr);
}

void StampPlot::addState(States_t newState) {
    this->setState((States_t)(state | newState));
}

void StampPlot::removeState(States_t newState) {
    this->setState((States_t)(state &~ newState));
}

void StampPlot::onRangeUpdated(RangedMeasurement_t newRange, Axis axisIdx) {
    if (rangeInitialized[axisIdx]) {
        if (newRange != currentRange[axisIdx]) {
            currentRange[axisIdx] = newRange;

            this->setAxisScale(axisIdx, newRange.min, newRange.max);
        }

    } else {
        currentRange[axisIdx] = newRange;
        this->setAxisScale(axisIdx, currentRange[axisIdx].min, currentRange[axisIdx].max);
        rangeInitialized[axisIdx] = true;
    }
    this->replot();
}

void StampPlot::onDurationUpdated(Measurement_t duration) {
    sweepDuration = duration;
    sweepDuration.convertValue(UnitPfxNone);
    this->setAxisScale(xBottom, 0.0, sweepDuration.value);

    this->replot();
}

void StampPlot::onClicked() {
//    if (QApplication::keyboardModifiers() & Qt::ControlModifier){
//        // Ctrl key is pressed
//        // Do something specific when Ctrl is pressed during the button click
//        qDebug() << "Ctrl key is pressed";
//    }
//    if (QApplication::keyboardModifiers() & Qt::ShiftModifier){
//        // Shift key is pressed
//        // Do something specific when Shift is pressed during the button click
//        qDebug() << "Shift key is pressed";
//    }
//    if (QApplication::keyboardModifiers() & Qt::AltModifier){
//        // Alt key is pressed
//        // Do something specific when Alt is pressed during the button click
//        qDebug() << "AltModifier key is pressed";
//     }
//    emit clicked(true);
}

void StampPlot::onUnclicked() {
//    emit clicked(false);
}

void StampPlot::resizeEvent(QResizeEvent * e) {
    if (e != nullptr) {
        QwtPlot::resizeEvent(e);
    }

    QSize siz = channelIdxLbl->minimumSizeHint();
    channelIdxLbl->setGeometry(this->canvas()->x(), this->canvas()->y(), siz.width(), siz.height());

    siz = stateLbl->minimumSizeHint();
    stateLbl->setGeometry(this->canvas()->x(), this->canvas()->y()+this->canvas()->height()-siz.height(), siz.width(), siz.height());
}
