#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include <QWheelEvent>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_symbol.h"
//#include "qwt_plot_scaleitem.h"
#include "qwt_scale_draw.h"
#include "qwt_text_label.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "qwt_plot_histogram.h"

#include "protocolcursor.h"
#include "epmlmanager.h"
#include "cursor.h"
#include "e4gcommlib.h"

typedef enum {
    CurveTypePlotSolid,
    CurveTypePlotFaint,
    CurveTypeAnalysisDashed,
    CurveTypeProtocolPreviewSolid,
    CurveTypeProtocolPreviewDotted,
    CurveTypeProtocolPreviewDashed,
    CurveTypeScatterPlot,
    CurveTypeNone
} CurveType_t;

namespace e4gcl = e4gCommLib;

class ProtocolWidget;
class ConversionScaleDraw;

class Plot : public QwtPlot {
    Q_OBJECT

public:
    Plot(QWidget * parent = nullptr, QString titleString = "", QString unitString = "");
    virtual ~Plot();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void drawCanvas(QPainter * p) override;

    void setTitle(QString text);
    void setXUnit(QString text);
    QString getXUnit();
    void setYUnit(QString text);
    QString getYUnit();
    void pushZoomStack(QRectF r);
    void pushZoomStack();
    QRectF popZoomStack();
    void clearZoomStack();
    bool isEmptyZoomStack();
    QRectF resetZoomStack();
    void shiftVertAxis(QwtPlot::Axis axis, double shiftValue);
    void blockXAxis(bool flag);

public slots:
    void onZoomInRequest(QRectF rect, bool synchOtherPlots = true);
    void onHorzZoomInRequest(QRectF rect, bool synchOtherPlots = false); /*!< This plot is usually triggered for all plots at once, so no further synch needed */
    void onVertZoomInRequest(QRectF rect, bool synchOtherPlots = true);
    void onVertZoomFullRequest(bool synchOtherPlots = true);
    void onZoomOutRequest(bool synchOtherPlots = true);
    void onZoomResetRequest(bool synchOtherPlots = true);

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

    QwtTextLabel * title;
    QwtTextLabel * xUnit;
    QwtTextLabel * yUnit;

    int xAxisMaxMajor;
    int yAxisMaxMajor;

    bool xAxisBlocked = false;
    bool autoScaleOnEmptyZoomStack = false;

    QPointF pickerFirstCornerPos;

//    QwtPlotScaleItem * yLeftScaleItem;
//    QwtPlotScaleItem * xBottomScaleItem;

    ConversionScaleDraw * xBottomScaleDraw;

private:
    QVector <QRectF> zoomStack;

signals:
    void resizePlot();
    void xAxisZoomed(double, double);
    void zoomPopped(double, double);
    void zoomReset(double, double);
};

class ProtocolPlot : public Plot {
    Q_OBJECT

public:
    ProtocolPlot(e4gcl::CommLib * commLib, QWidget * parent = nullptr, QString titleString = "", QString unitString = "");
    ~ProtocolPlot();

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus);
    bool exportEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus);

    void setProtocol(ProtocolWidget * protocol);
    QVector <ProtocolCursor *> * getProtocolCursors();
    void setCursorsVisibility(bool visible);

    std::vector <YAML::Cursor> getYamlCursors();

    void setCursorsFromYaml(const std::vector <YAML::Cursor> &yamlCursors);

public slots:
    void onEnableCursorManagement(bool);
    void onCursorAddRequest(QPointF p);
    void onCursorMoveRequest(QRectF r);
    void onCursorMoving(QPointF p);
    void onCursorOpenPropertiesRequest(QPointF p);
    void onCursorOpenPropertiesRequest(int cursorIdx);
    void onCursorPropertiesAccepted();
    void onCursorDeleteRequest(int cursorIdx);

private:
    bool importCursor(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus);
    bool importCursor(const YAML::Cursor &yamlCursor);
    void updateCursorsSections(QVector <int> &map);

    inline bool getClosestCursor(QPointF p, int &cursorIdx);
    inline bool checkOnCursorBin(double x, double y);

    e4gcl::CommLib * commLib = nullptr;
    QVector <ProtocolCursor *> * protocolCursors;

    QwtPlotPicker * cursorAddPicker;
    QwtPlotPicker * cursorRemovePicker;
    QwtPlotPicker * cursorMovePicker;

    bool movingCursor = false;
    bool movingCursorTry = false;
    int movedCursorIdx = -1;
    bool cursorGrabFail = false;
    double cursorInitialPosition = 0.0;
    bool cursorMovingLeft = false;

    ProtocolWidget * protocol = nullptr;
    bool cursorsVisible = true;
    bool mouseOnCursorBin = false;

signals:
    void addCursors();
    void moveCursors();
    void editCursors();
    void removeCursors(QVector <int>);
    void openCursorBin(bool);
};

class ProtocolDataPlot : public Plot {
    Q_OBJECT

public:
    typedef enum {
        NoTrigger,
        RiseTrigger,
        FallTrigger
    } TriggerType_t;

    ProtocolDataPlot(int channelIdx, QWidget * parent = nullptr, QString titleString = "", QString unitString = "");
    ~ProtocolDataPlot();

    void initializeRange(e4gcl::RangedMeasurement_t newRange);

public slots:
    void onEnableTriggerPicker();
    void onDisableTriggerPicker();
    void onEnableTriggerMarker(double x, double y);
    void onDisableTriggerMarker();
    void onRangeUpdated(e4gcl::RangedMeasurement_t newRange);
    void onDurationUpdated(double duration);
    void onRiseTriggerRequest(QPointF p, bool notifyPlotTab = true);
    void onFallTriggerRequest(QPointF p, bool notifyPlotTab = true);

protected:
    void recomputeXAxisFactor(double duration) override;

private:
    int channelIdx;

    QwtPlotPicker * riseTriggerPicker;
    QwtPlotPicker * fallTriggerPicker;

    QwtPlotMarker * triggerMarkerHorz;
    QwtPlotMarker * triggerMarkerVert;
    double triggerX = 0.0;
    double triggerY = 0.0;

    QCursor * triggerCursor;

    e4gcl::RangedMeasurement_t range0;
    e4gcl::RangedMeasurement_t currentRange;
    e4gcl::Measurement_t sweepDuration = {1.0, e4gcl::UnitPfxNone, "s"};
    e4gcl::UnitPfx_t xAxisPrefix = e4gcl::UnitPfxNone;

signals:
    void disableTriggerPicker();
    void triggerEnabled(ProtocolDataPlot::TriggerType_t, double, double, int, bool);
    void updateTriggerX(double);
    void updateTriggerY(double);
};

class AnalysisPlot : public Plot {
public:
    AnalysisPlot(QWidget * parent, QString titleString, QString unitString);
    ~AnalysisPlot();
};

class Curve : public QwtPlotCurve {
public:
    Curve(CurveType_t curveType = CurveTypeProtocolPreviewSolid, double size = 1.0);
    Curve(const Curve &curve);

    CurveType_t getCurveType() const;
    double getSize() const;
    void setColor(QColor color);
    void setColor(CurveType_t color);
    QColor getColor();

private:
    CurveType_t curveType;
    QColor color;
    double size;
};

class HistogramCurve : public QwtPlotHistogram {
public:
    HistogramCurve();
    ~HistogramCurve();

    void drawHist(int binsNum, double lowerBound, double binWidth, QVector <int> histCount);

private:
    QwtIntervalSeriesData * series = nullptr;
    QVector <QwtIntervalSample> * samples = nullptr;
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

#endif // PLOT_H
