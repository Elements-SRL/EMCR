#ifndef PROTOCOLLIST_H
#define PROTOCOLLIST_H

#include <QListWidget>
#include <QDir>
#include <QEvent>
#include <QKeyEvent>
#include <QAction>

#include "protocolwidget.h"
#include "protocolpropertydialog.h"
#include "impexpprotocoldialog.h"
#include "protocols.h"
#include "messagedispatcher.h"

class ProtocolList : public QListWidget {
    Q_OBJECT

public:
    typedef enum {
        Success,
        ErrorProtocolAlreadyExists,
        ErrorNoProtocolSelected,
        ErrorLoadOffsetCompensationProtocolFail,
        ErrorLoadRestingPotentialProtocolFail,
        ErrorLoadLastExecutedProtocolFail,
        ErrorLoadLastExecutionProtocolsFail,
        ErrorLoadAnalysisVoltageProtocolsFail,
        ErrorLoadDefaultProtocolsFail
    } ProtocolListStatus_t;

    ProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent = nullptr);
    virtual ~ProtocolList();

    QVector <ProtocolWidget *> * getProtocols();

    void startVhold0Protocol();
    void startIhold0Protocol();
    void inhibitProtocols(bool inhibitFlag);
    void startProtocolFromShortCutIndex(int shortCutIdx);
    void setClampingModality(ClampingModality_t clampingModalitySet);
    void saveAndClosePropertyDialog();

public slots:
    void onStartProtocol(bool recordFlag = false);
    void onStopProtocol();
    void onAddProtocol();
    void onRemoveProtocol();
    void onEditProtocol();
    void onCopyProtocol();
    void onSetProtocolsShortCuts();
    void onImportProtocols();
    void onExportProtocols();
    void onHoldingDeltaChanged(Measurement_t newHoldingDelta);
    void onItemDoubleClicked(QListWidgetItem * item);
    void onProtocolNameChanged(QString oldName, QString newName);
    void onProtocolRequestOutcome(ProtocolApplicationStatus_t status);

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;
    void contextMenuEvent(QContextMenuEvent * event) override;

    void createActions();
    void createShortCutsDialog();
    void populateShortCutsDialog();
    void clearShortCutsDialog();
    int getShortCutByProtocol(ProtocolWidget * protocol);
    virtual ProtocolWidget * newGapfreeProtocol(QString name) = 0;
    virtual ProtocolWidget * newEpisodicProtocol(QString name) = 0;
    void exportLastProtocols();
    void exportAnalysisProtocols();
    void importVhold0Protocol();
    void importIhold0Protocol();
    void importLastRunProtocol();
    void importLastProtocols();
    void importAnalysisVoltageProtocols();
    bool importProtocols(QString fullFileName = YAML_DEFAULT_FULL_FILE);
    bool importProtocols(ImportProtocolDialog * ipd);
    void importProtocol(const YAML::VoltageProtocol &yamlProtocol);
    void importProtocol(const YAML::CurrentProtocol &yamlProtocol);
    void importProtocolAs(const YAML::VoltageProtocol &yamlProtocol, QString name);
    void importProtocolAs(const YAML::CurrentProtocol &yamlProtocol, QString name);
    YAML::VoltageProtocol_t copyVoltageProtocol(ProtocolWidget * protocol);
    YAML::CurrentProtocol_t copyCurrentProtocol(ProtocolWidget * protocol);
    void pasteVoltageProtocol(const YAML::VoltageProtocol_t &yamlProtocol);
    void pasteCurrentProtocol(const YAML::CurrentProtocol_t &yamlProtocol);
    void removeProtocolByName(QString name);
    void removeProtocol(ProtocolWidget * protocol, QString name);
    ProtocolWidget * findProtocolByName(QString name);
    QString availableProtocolName(QString name);
    int getProtocolIndexFromAnalysis(YAML::AnalysisType_t type);

    YAML::Protocols_t getYamlProtocols();

    MessageDispatcher * msgDisp = nullptr;
    ProtocolPropertyDialog * protocolPropertyDialog;
    QWidget * parent;
    QVector <ProtocolWidget *> * protocols;
    QStringList * protocolsNames;
    QDialog * shortCutsDlg;
    QVector <ProtocolWidget *> shortCutsProtocols;
    QVector <int> shortCutsIndexes;
    QString protocolsGroupName;
    ClampingModality_t clampingModality; /*!< Clamping modality of this protocol list */
    ClampingModality_t clampingModalitySet; /*!< Clamping modality currently set by the GUI */
    ProtocolType_t lastStartedType = ProtocolTypeGapfree;

    ProtocolWidget * vhold0Protocol = nullptr;
    ProtocolWidget * ihold0Protocol = nullptr;
    ProtocolWidget * lastRunProtocol = nullptr;
    bool vhold0ProtocolFlag = false;
    bool ihold0ProtocolFlag = false;
    bool lastRunProtocolFlag = false;
    Measurement_t holdingDelta = {0.0, UnitPfxNone, ""};

    QGridLayout * shortCutsDlgLo;
    QVector <QComboBox *> shortCutIdxCbxs;

    QAction * startProtocolAct;
    QAction * copyProtocolAct;
    QAction * editProtocolAct;
    QAction * openProtocolPropertiesAct;
    QAction * removeProtocolAct;

protected slots:
    void onAcceptShortCutsDialog();
    void onRejectShortCutsDialog();

signals:
    void startProtocolRequest(ProtocolWidget * protocol);
    // void protocolStarted(unsigned int, ProtocolWidget *);
    void currentApplied();
    void increaseProtocolId();
    void requestCurrentRange(int);
    void requestVoltageRange(int);
    void protocolAppliedRange(RangedMeasurement_t);
    void requestSamplingRate(int);
};

class VoltageProtocolList : public ProtocolList {
    Q_OBJECT

public:
    VoltageProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent = nullptr);
    ~VoltageProtocolList();

    ProtocolWidget * newGapfreeProtocol(QString name) override;
    ProtocolWidget * newEpisodicProtocol(QString name) override;
};

class AnalysisVoltageProtocolList : public ProtocolList {
    Q_OBJECT

public:
    AnalysisVoltageProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent = nullptr);
    ~AnalysisVoltageProtocolList();

    ProtocolWidget * newGapfreeProtocol(QString name) override;
    ProtocolWidget * newEpisodicProtocol(QString name) override;

protected:
    void contextMenuEvent(QContextMenuEvent * event) override;
};

class CurrentProtocolList : public ProtocolList {
    Q_OBJECT

public:
    CurrentProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent = nullptr);
    ~CurrentProtocolList();

    ProtocolWidget * newGapfreeProtocol(QString name) override;
    ProtocolWidget * newEpisodicProtocol(QString name) override;
};

#endif // PROTOCOLLIST_H
