#include "curve.h"

#include "qwt_symbol.h"

Curve::Curve(CurveType_t curveType, double size) :
    QwtPlotCurve(),
    curveType(curveType),
    size(size) {

    color = QColor(Qt::blue);
    QwtSymbol * symbol;
    switch (curveType) {
    case CurveTypePlotSolid:
        this->setPen(color, size);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeStampPlotSolid:
        color = QColor(Qt::white);
        this->setPen(color, size);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeAnalysisDashed:
        this->setPen(QColor(Qt::red), size, Qt::DashLine);

        symbol = new QwtSymbol(QwtSymbol::NoSymbol);
        break;

    case CurveTypeProtocolPreviewSolid:
        this->setPen(color, size);

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
        symbol->setPen(color, 2.0);
        symbol->setBrush(QBrush(color));
        symbol->setSize(4);
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
    QwtSymbol * symbol = const_cast <QwtSymbol *> (this->symbol());
    switch (curveType) {
    case CurveTypePlotSolid:
        this->setPen(color, size);
        break;

    case CurveTypeAnalysisDashed:
        this->setPen(color, size, Qt::DashLine);
        break;

    case CurveTypeProtocolPreviewSolid:
        this->setPen(color, size);
        break;

    case CurveTypeProtocolPreviewDotted:
        this->setPen(color, size, Qt::DotLine);
        break;

    case CurveTypeProtocolPreviewDashed:
        this->setPen(color, size, Qt::DashLine);
        break;

    case CurveTypeScatterPlot:
        this->setPen(color, size, Qt::NoPen);

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
