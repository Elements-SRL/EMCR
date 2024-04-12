#include "stampplot.h"

#include "qwt_plot_layout.h"
#include "qwt_plot_canvas.h"
#include <QApplication>
#include "globaldefines.h"
#include <qwt_symbol.h>

StampPlot::StampPlot(int channelIdx, std::string channelname, int idealPlotWidth, int idealPlotHeight, QWidget * parent) :
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
    this->setCanvasBackground(Qt::lightGray);

    QFont font;
    font.setPointSize(7);

    QwtText text;
    text.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    text.setColor(Qt::white);

    channelIdxLbl = new QwtTextLabel(this);
    text.setText(QString("%1").fromStdString(channelname));
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

    xAxisMaxMajor = this->axisMaxMajor(xBottom);
    yAxisMaxMajor = this->axisMaxMajor(yLeft);

    selected = false;
    colorMarker = new QwtPlotMarker();
    colorMarker->attach(this);
    auto x = this->width();
    auto y = this->height();
    colorMarker->setValue(x*.0093, y*0.4); // Set the position where you want to draw the solid color
    colorMarker->setLineStyle(QwtPlotMarker::NoLine);
    this->setStyleSheet("StampPlot { border: 1px solid black; }");
}

QSize StampPlot::sizeHint() const {
    return QSize(idealPlotWidth, idealPlotHeight);
}

QSize StampPlot::minimumSizeHint() const {
    return QSize(STAMP_PLOT_MIN_WIDTH, STAMP_PLOT_MIN_HEIGHT);
}

void StampPlot::setSelected(bool flag) {
    if (flag != selected) {
        selected = flag;
        if (selected) {
            this->setCanvasBackground(Qt::black);

        } else {
            this->setCanvasBackground(Qt::lightGray);
        }
    }
}

void StampPlot::mousePressEvent(QMouseEvent *event){
    emit clicked(event);
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

void StampPlot::onRangeUpdated(RangedMeasurement_t newRange) {
    Axis axisIdx = QwtPlot::yLeft;
    currentRange[axisIdx] = newRange;
    this->setAxisScale(axisIdx, currentRange[axisIdx].min, currentRange[axisIdx].max);
    this->replot();
}

void StampPlot::onDurationUpdated(Measurement_t duration) {
    sweepDuration = duration;
    sweepDuration.convertValue(UnitPfxNone);
    this->setAxisScale(xBottom, 0.0, sweepDuration.value);

    this->replot();
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

void StampPlot::setLegendColor(QColor color) {
    auto symbol = new QwtSymbol();  // Create a QwtSymbol object
    symbol->setStyle(QwtSymbol::Rect);  // Set the style to Rect
    symbol->setBrush(QBrush(color));  // Set the fill color
    symbol->setPen(QPen(color));      // Set the border color
    symbol->setSize(QSize(10, 10));    // Set the size of the block
    colorMarker->setSymbol(symbol);  // Use the setSymbol method to set the symbol
}

void StampPlot::setName(std::string name){
    QwtText text;
    text.setRenderFlags(Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine);
    text.setColor(Qt::white);
    QFont font;
    font.setPointSize(7);
    text.setText(QString("%1").fromStdString(name));
    channelIdxLbl->setText(text);
    channelIdxLbl->setFont(font);
    channelIdxLbl->setMargin(0);
    channelIdxLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}
