#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include <QWheelEvent>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_symbol.h"
#include "qwt_scale_draw.h"
#include "qwt_text_label.h"
#include "qwt_plot_picker.h"
#include "messagedispatcher.h"
#include "protocolcursor.h"
#include "cursor.h"
#include "conversionscaledraw.h"
#include "rect4.h"

#include "e384commlib_global.h"

namespace commlib = e384CommLib;

class ProtocolWidget;

class ProtocolPlot : public QwtPlot {
    Q_OBJECT

public:
    ProtocolPlot(MessageDispatcher * msgDisp, QString titleString, QString xUnitString, QString yUnitString, QWidget * parent = nullptr);
    ~ProtocolPlot();

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

    void setProtocol(ProtocolWidget * protocol);
    QVector <ProtocolCursor *> * getProtocolCursors();
    void setCursorsVisibility(bool visible);

    std::vector <YAML::Cursor> getYamlCursors();
    void setCursorsFromYaml(const std::vector <YAML::Cursor> &yamlCursors);

public slots:
    void onZoomInRequest(Rect4 * rect);
    void onHorzZoomInRequest(Rect4 * rect);
    void onVertZoomInRequest(Rect4 * rect);
    void onVertZoomFullRequest();
    void onZoomOutRequest();
    void onZoomResetRequest();

    void onEnableCursorManagement(bool);
    void onCursorAddRequest(QPointF p);
    void onCursorMoveRequest(QRectF r);
    void onCursorMoving(QPointF p);
    void onCursorOpenPropertiesRequest(QPointF p);
    void onCursorOpenPropertiesRequest(int cursorIdx);
    void onCursorPropertiesAccepted();
    void onCursorDeleteRequest(int cursorIdx);

protected:
    typedef enum {
        PickerZoomRect,
        PickerZoomHorz,
        PickerZoomVert
    } PickerZoomType_t;

    virtual void resizeEvent(QResizeEvent * e) override;
    virtual void wheelEvent(QWheelEvent * e) override;
    virtual void recomputeXAxisFactor(double duration);

private:
    bool importCursor(const YAML::Cursor &yamlCursor);
    void updateCursorsSections(QVector <int> &map);

    inline bool getClosestCursor(QPointF p, int &cursorIdx);
    inline bool checkOnCursorBin(double x, double y);

    MessageDispatcher * msgDisp = nullptr;

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

    commlib::RangedMeasurement_t currentRange[axisCnt];
    commlib::Measurement_t sweepDuration = {1.0, commlib::UnitPfxNone, "s"};
    commlib::UnitPfx_t xAxisPrefix = commlib::UnitPfxNone;

    QVector <Rect4> zoomStack;
    QVector <bool> rangeInitialized;

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

private slots:
    virtual void onZoomInPickerAppended(const QPointF &p);
    virtual void onZoomInPickerMoved(const QPointF &p);
    virtual void onZoomInPickerSelected(const QRectF &r);
    virtual void onZoomOutPickerSelected(const QPointF &p);
    virtual void onZoomResetPickerSelected(const QPointF &p);

signals:
    void zoomInRequest(Rect4 * rect);
    void zoomOutRequest();
    void zoomResetRequest();

    void addCursors();
    void moveCursors();
    void editCursors();
    void removeCursors(QVector <int>);
    void openCursorBin(bool);
};

#endif // PLOT_H
