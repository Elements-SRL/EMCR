#ifndef PROTOCOLPREVIEW_H
#define PROTOCOLPREVIEW_H

#include <QWidget>

#include "qwt_plot_textlabel.h"

#include "plot.h"
#include "protocoldropitem.h"
#include "protocolitem.h"
#include "protocolsection.h"
#include "protocolitemdroplist.h"
#include "cursorsmanager.h"
#include "e4gcommlib.h"

namespace e4gcl = e4gCommLib;

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
    ProtocolPreview(e4gcl::CommLib * commLib, e4gcl::RangedMeasurement_t timeRange, e4gcl::RangedMeasurement_t stimulusRange, QString title = "Protocol preview");
    ~ProtocolPreview();

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus);
    bool exportEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus);

    void setProtocol(ProtocolWidget * protocol);
    void setStimulusRange(e4gcl::RangedMeasurement_t &range);
    void setHoldingDelta(e4gcl::Measurement_t &holdingDelta);
    void setAnalysisPidl(AnalysisProtocolItemDropList * analysisPidl);
    void setTooManyTriggersWarning(bool flag);

public slots:
    void updateView();

protected:
    e4gcl::CommLib * commLib;
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
    e4gcl::RangedMeasurement_t stimulusRange; /*!< This is the full range of the stimulus */
    e4gcl::RangedMeasurement_t stimulusActiveRange; /*!< This is the range when the holding delta is taken into consideration */
    e4gcl::Measurement_t holdingDelta = {0.0, e4gcl::UnitPfxNone, ""};
    QString stimulusUnit;
    e4gcl::RangedMeasurement_t timeRange;
    QString timeUnit;

private:
    ItemsProcStatus_t interpretProtocolItems(ProtocolWidget * protocol);

    ProtocolWidget * protocol;
    ProtocolPlot * protocolPlot;
    QwtText * errorText;
    QwtPlotTextLabel * errorItem;
    vector <Curve *> protocolPlotCurve;
    vector <double *> protocolPlotData;
    ProtocolSections * protocolSections = nullptr;
    CursorsManager * cursorsManager = nullptr;
};

class MinimalProtocolPreview : public ProtocolPreview {
    Q_OBJECT

public:
    MinimalProtocolPreview(e4gcl::CommLib * commLib, e4gcl::RangedMeasurement_t timeRange, e4gcl::RangedMeasurement_t stimulusRange, QString title = "Protocol preview");
};

#endif // PROTOCOLPREVIEW_H
