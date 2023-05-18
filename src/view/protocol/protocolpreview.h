#ifndef PROTOCOLPREVIEW_H
#define PROTOCOLPREVIEW_H

#include <QWidget>

#include "qwt_plot_textlabel.h"

#include "protocol/protocolplot.h"
#include "protocoldropitem.h"
#include "protocolitem.h"
#include "protocolsection.h"
#include "protocolitemdroplist.h"
#include "cursorsmanager.h"
#include "cursor.h"
#include "curve.h"

#define PPW_MAX_PLOT_DATA 1024
#define PPW_MAX_PTS_PER_ITEM 21
#define PPW_MAX_REPS 3

typedef enum {
    ItemsProcOk,
    ItemsProcErrorNotEnoughItemsForSequence,
    ItemsProcErrorOverlappingSequences,
    ItemsProcErrorMidInfiniteSequence,
    ItemsOverflow,
    ItemsNotFound,
    ItemsOverStimulus,
    ItemsUnderStimulus,
    ItemsUnderDuration,
    ItemsNotProcessed
} ItemsProcStatus_t;

class ProtocolWidget;

class ProtocolPreview : public QWidget {
    Q_OBJECT

public:
    ProtocolPreview(ModelDevice * mDev, RangedMeasurement_t timeRange, RangedMeasurement_t stimulusRange, QString title = "Protocol preview");
    ~ProtocolPreview();

    void setProtocol(ProtocolWidget * protocol);
    void setStimulusRange(RangedMeasurement_t &range);
    void setHoldingDelta(Measurement_t &holdingDelta);
    void setAnalysisPidl(AnalysisProtocolItemDropList * analysisPidl);
    void setTooManyTriggersWarning(bool flag);

    std::vector <YAML::Cursor> getYamlCursors();

    void setCursorsFromYaml(const std::vector <YAML::Cursor> &yamlCursors);

public slots:
    void updateView();

protected:
    ModelDevice * mDev;
    QVector <ProtocolItem *> protocolItems;
    QFrame * cursorsWid;
    QPushButton * manageCursorBtn;
    QLabel * cursorsBinLbl;
    QLabel * cursorsWarningLbl;
    unsigned int maxOutputTriggers;
    QLabel * cursorsWarningPlaceHolderLbl;
    QPixmap cursonBinClosedPxm;
    QPixmap cursonBinOpenPxm;
    bool minimal = false;
    unsigned int maxProtocolItems;
    RangedMeasurement_t stimulusRange; /*!< This is the full range of the stimulus */
    RangedMeasurement_t stimulusActiveRange; /*!< This is the range when the holding delta is taken into consideration */
    Measurement_t holdingDelta = {0.0, UnitPfxNone, ""};
    QString stimulusUnit;
    RangedMeasurement_t timeRange;
    QString timeUnit;

private:
    ItemsProcStatus_t interpretProtocolItems(ProtocolWidget * protocol);

    ProtocolWidget * protocol;
    ProtocolPlot * protocolPlot;
    QwtText * errorText;
    QwtPlotTextLabel * errorItem;
    std::vector <Curve *> protocolPlotCurve;
    std::vector <double *> protocolPlotData;
    ProtocolSections * protocolSections = nullptr;
    CursorsManager * cursorsManager = nullptr;
};

class MinimalProtocolPreview : public ProtocolPreview {
    Q_OBJECT

public:
    MinimalProtocolPreview(ModelDevice * mDev, RangedMeasurement_t timeRange, RangedMeasurement_t stimulusRange, QString title = "Protocol preview");
};

#endif // PROTOCOLPREVIEW_H
