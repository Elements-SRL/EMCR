#include "curve.h"

#include "qwt_symbol.h"
#include "themecontroller.h"

#include <QPen>

namespace {

// Cosmetic pen: the width is in physical pixels, so it does not scale with the
// DPI and needs no update when it changes. A non-cosmetic 1.0 pen at 125%
// scaling becomes 1.25 px and Qt leaves its fast thin-line drawing path.
QPen curvePen(const QColor &color, double width, Qt::PenStyle style = Qt::SolidLine) {
    QPen pen(color, width, style);
    pen.setCosmetic(true);
    return pen;
}

// Color of the chessboard mini-plot curve
QColor stampCurveColor() {
    const QColor c = ThemeController::color(QStringLiteral("@STAMP_CURVE"));
    return c.isValid() ? c : QColor(Qt::white);
}
}

Curve::Curve(CurveType_t curveType, double size) :
    QwtPlotCurve(),
    curveType(curveType),
    size(size) {

    color = QColor(Qt::blue);
    QwtSymbol * symbol;
    switch (curveType) {
    case CurveTypePlotSolid:
        this->setPen(curvePen(color, size));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypePlotFaint:
        color = QColor(Qt::darkGray);
        this->setPen(curvePen(color, size, Qt::SolidLine));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypePlotDashed:
        this->setPen(curvePen(color, size, Qt::DashLine));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypePlotFaintDashed:
        color = QColor(Qt::darkGray);
        this->setPen(curvePen(color, size, Qt::DashLine));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeStampPlotSolid:
        color = stampCurveColor();
        this->setPen(curvePen(color, size));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeAnalysisDashed:
        this->setPen(curvePen(QColor(Qt::red), size, Qt::DashLine));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeProtocolPreviewSolid:
        this->setPen(curvePen(color, size));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeProtocolPreviewDotted:
        this->setPen(curvePen(color, size, Qt::DotLine));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeProtocolPreviewDashed:
        this->setPen(curvePen(color, size, Qt::DashLine));

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeScatterPlot:
        this->setPen(curvePen(color, size, Qt::NoPen));

        symbol = new QwtSymbol(QwtSymbol::Ellipse);
        symbol->setPen(color, 2.0);
        symbol->setBrush(QBrush(color));
        symbol->setSize(4);
        break;

    default:
        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
    }
    this->setSymbol(symbol);

    // Limits the points to 4 per pixel column (first, min, max, last):
    // the drawn curve does not change. Ignored for scatter plots.
    if (curveType != CurveTypeScatterPlot) {
        this->setPaintAttribute(QwtPlotCurve::FilterPointsAggressive, true);
    }
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
    QwtSymbol * symbol = const_cast <QwtSymbol *> (this->symbol());
    switch (curveType) {
    case CurveTypePlotSolid:
        this->setPen(curvePen(color, size));
        break;

    case CurveTypePlotFaint:
        color = QColor(Qt::darkGray);
        break;

    case CurveTypePlotDashed:
        this->setPen(curvePen(color, size, Qt::DashLine));
        break;

    case CurveTypePlotFaintDashed:
        color = QColor(Qt::darkGray);
        this->setPen(curvePen(color, size, Qt::DashLine));
        break;

    case CurveTypeStampPlotSolid:
        this->setPen(curvePen(color, size));
        break;

    case CurveTypeAnalysisDashed:
        this->setPen(curvePen(color, size, Qt::DashLine));
        break;

    case CurveTypeProtocolPreviewSolid:
        this->setPen(curvePen(color, size));
        break;

    case CurveTypeProtocolPreviewDotted:
        this->setPen(curvePen(color, size, Qt::DotLine));
        break;

    case CurveTypeProtocolPreviewDashed:
        this->setPen(curvePen(color, size, Qt::DashLine));
        break;

    case CurveTypeScatterPlot:
        this->setPen(curvePen(color, size, Qt::NoPen));

        symbol->setPen(color, 3.0);
        symbol->setBrush(QBrush(color));
        break;

    default:
        break;
    }
    this->setSymbol(symbol);
}

QColor Curve::getColor() {
    return color;
}
