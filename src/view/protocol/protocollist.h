#ifndef PROTOCOLLIST_H
#define PROTOCOLLIST_H

#include <QListWidget>
#include <QDir>
#include <QEvent>
#include <QKeyEvent>
#include <QAction>

#include "protocolmanager.h"
#include "protocolwidget.h"
#include "protocolpropertydialog.h"
#include "impexpprotocoldialog.h"
#include "protocolssettingsdialog.h"
#include "protocols.h"

class ProtocolList : public QListWidget {
    Q_OBJECT

public:
    typedef enum {
        Success,
        ErrorProtocolAlreadyExists,
        ErrorNoProtocolSelected,
        ErrorLoadNullProtocolsFail,
        ErrorLoadOffsetCompensationProtocolFail,
        ErrorLoadRestingPotentialProtocolFail,
        ErrorLoadLastExecutedProtocolFail,
        ErrorLoadLastExecutionProtocolsFail,
        ErrorLoadDefaultProtocolsFail
    } ProtocolListStatus_t;

    ProtocolList(ModelDevice * mDev, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent = nullptr);
    virtual ~ProtocolList();

    QVector <ProtocolWidget *> * getProtocols();
    ProtocolsSettingsDialog * getProtocolsSettingsDialog();

    void startVhold0Protocol();
    void startIhold0Protocol();
    void recordIhold0Protocol();
    void setStopProtocolHold(Measurement_t hold);
    void inhibitProtocols(bool inhibitFlag);
    void startProtocol(int shortCutIdx);
    void recordProtocol(int shortCutIdx);
    void setClampingModality(ClampingModality_t clampingModalitySet);
    void saveAndClosePropertyDialog();
    void setSecondaryDevice(bool flag);

public slots:
    void onStartProtocol(bool recordFlag = false);
    void onRecordProtocol();
    void onSaveLastProtocol();
    void onStopProtocol();
    void onAddProtocol();
    void onRemoveProtocol();
    void onEditProtocol();
    void onCopyProtocol();
    void onSetProtocolsShortCuts();
    void onImportProtocols();
    void onExportProtocols();
    void onProtocolsSettings();
    void onHoldingDeltaChanged(Measurement_t newHoldingDelta);
    void onProtocolNameChanged(QString oldName, QString newName);
    void onPlotting(bool flag, ProtocolType_t protocolType);
    void onControlsChanged();
    void onItemDoubleClicked(QListWidgetItem * item);
    void onProtocolEnded();

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
    void setNullProtocolHolding(ProtocolWidget * protocol);
    void exportLastRunProtocol(ProtocolWidget * protocol);
    void exportLastProtocols();
    void importNullProtocol();
    void importVhold0Protocol();
    void importIhold0Protocol();
    void importLastRunProtocol();
    void importLastProtocols();
    bool importProtocols(QString fullFileName = YAML_DEFAULT_FILE);
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

    YAML::Protocols_t getYamlProtocols();

    ModelDevice * mDev;
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

    ProtocolManager * protocolManager = nullptr;
    ProtocolWidget * nullGapfreeProtocol = nullptr;
    ProtocolWidget * nullEpisodicProtocol = nullptr;
    ProtocolWidget * vhold0Protocol = nullptr;
    ProtocolWidget * ihold0Protocol = nullptr;
    ProtocolWidget * lastRunProtocol = nullptr;
    bool nullProtocolFlag = false;
    bool vhold0ProtocolFlag = false;
    bool ihold0ProtocolFlag = false;
    bool lastRunProtocolFlag = false;
    Measurement_t holdingDelta = {0.0, UnitPfxNone, ""};
    ProtocolsSettingsDialog * protocolsSettingsDlg;
    QGridLayout * shortCutsDlgLo;
    QVector <QComboBox *> shortCutIdxCbxs;

    QAction * startProtocolAct;
    QAction * recordProtocolAct;
    QAction * copyProtocolAct;
    QAction * editProtocolAct;
    QAction * openProtocolPropertiesAct;
    QAction * removeProtocolAct;

    bool plottingFlag = false;
    bool controlsChangedFlag = false;

protected slots:
    void onAcceptShortCutsDialog();
    void onRejectShortCutsDialog();

signals:
    void protocolStarted(unsigned int, ProtocolWidget *);
    void protocolSaveRequest(unsigned int, ProtocolWidget *);
    void currentApplied();
    void increaseProtocolId();
    void enableSaveLastProtocol(bool);
    void requestCurrentRange(int);
    void requestVoltageRange(int);
    void protocolAppliedRange(RangedMeasurement_t);
    void requestSamplingRate(int);
    void newRecordPath();
};

class VoltageProtocolList : public ProtocolList {
    Q_OBJECT

public:
    VoltageProtocolList(ModelDevice * mDev, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent = nullptr);
    ~VoltageProtocolList();

    ProtocolWidget * newGapfreeProtocol(QString name) override;
    ProtocolWidget * newEpisodicProtocol(QString name) override;
};

class CurrentProtocolList : public ProtocolList {
    Q_OBJECT

public:
    CurrentProtocolList(ModelDevice * mDev, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent = nullptr);
    ~CurrentProtocolList();

    ProtocolWidget * newGapfreeProtocol(QString name) override;
    ProtocolWidget * newEpisodicProtocol(QString name) override;
};

#endif // PROTOCOLLIST_H
