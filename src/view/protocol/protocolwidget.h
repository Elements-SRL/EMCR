#ifndef PROTOCOLWIDGET_H
#define PROTOCOLWIDGET_H

#include <QListWidgetItem>
#include <QDialog>
#include <QGridLayout>

#include "protocoleditor.h"
#include "protocolpropertydialog.h"
#include "protocolpreview.h"
#include "protocolitem.h"
#include "protocoldefs.h"
#include "voltageprotocol.h"
#include "currentprotocol.h"
#include "protocolmodel.h"
#include "messagedispatcher.h"

class ProtocolCtrlDispatcher;

class ProtocolWidget : public QObject, public QListWidgetItem {
    Q_OBJECT

public:
    typedef enum {
        ClosureRequestSave,
        ClosureRequestDontSave,
        ClosureRequestRejected
    } ClosureRequestReply_t;

    ProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog, ProtocolType_t type, ClampingModality_t clampingModality);
    ProtocolWidget();
    ~ProtocolWidget();

    void initConsumerRequests();
    bool consumerRequested(ProtocolConsumerType_t consumerType);
    void setConsumerRequest(ProtocolConsumerType_t consumerType);
    void resetConsumerRequests();
    QString getName();
    void setShortCutIdx(int value);
    int getShortCutIdx();
    ProtocolPreview * getProtocolPreview();
    void openPropertyDialog(QStringList pn);
    bool isPropertyDialogOpened(QString &protocolName);
    void populatePropertyDialog();
    void clearPropertyDialog();
    ClosureRequestReply_t askForPropertyDialogClosure();
    int openProtocolEditor();
    ProtocolType_t getType();
    ClampingModality_t getClampingModality();
    QVector <ProtocolDropItem *> * getProtocolDropItems();
    void pushProtocolItems(QVector <ProtocolItem *> items);
    void setProtocolItems();
    QVector <ProtocolItem *> getProtocolItems();
    void pushProcessingStatus(ItemsProcStatus_t status);
    void setProcessingStatus();
    ItemsProcStatus_t getProcessingStatus();
    void setInhibition(bool inhibitFlag);
    bool isInhibited();
    void pushTriggerCursors(QVector <TriggerCursor *> cursors);
    void setTriggerCursors();
    QVector <TriggerCursor *> getTriggerCursors();
    void setProtocolSections(ProtocolSections * sections);
    ProtocolSection * getSectionByItem(int itemIdx, int repsIdx, int sweepIdx);
    void setHold(Measurement_t hold);
    Measurement_t getHold();
    void setHoldingDelta(Measurement_t &holdingDelta);
    virtual void setStimulusRangeIndex(int idx) = 0;
    virtual int getStimulusRangeIndex() = 0;
    UnitPfx_t getStimulusPrefix();
    bool getHoldRef();
    int getSweepsNum();
    virtual int getCurrentRangeIndex() = 0;
    virtual int getVoltageRangeIndex() = 0;
    int getSamplingRateIndex();
    void setProtocolValid(bool valid);
    void setAppliedRange(RangedMeasurement_t &newAppliedRange);
    RangedMeasurement_t getAppliedRange();
    bool getAnalysisType(YAML::AnalysisType_t &type);

    virtual Measurement_t getTotalDuration() = 0;
    void getSweepTrigger(double &value, bool &rising);
    bool hasInfiniteRepetition();
    virtual ProtocolSection * getItemAtTime(double time, int sweepIdx, double &offset) = 0;
    virtual ProtocolSection * getItemAtTime(double time, int itemIdx, int repsIdx, int sweepIdx, double &offset) = 0;

    YAML::VoltageProtocol getYamlVoltageProtocol();
    YAML::CurrentProtocol getYamlCurrentProtocol();

    void setProtocolFromYaml(const YAML::VoltageProtocol &yamlProtocol);
    void setProtocolFromYaml(const YAML::CurrentProtocol &yamlProtocol);

public slots:
    void onAcceptPropertyDialog();
    void onRejectPropertyDialog();

protected:
    void updateText();
    void setValidityColor();

    ProtocolModel * model = nullptr;
    MessageDispatcher * msgDisp = nullptr;
    QString name;
    ProtocolPropertyDialog * dialog;
    ProtocolType_t type;
    ClampingModality_t clampingModality;
    QGridLayout * propertyCtrlLo;
    QHBoxLayout * propertyBtnsHl;
    QLineEdit * nameEdit;
    int shortCutIdx = -1;
    Measurement_t hold;
    RangedMeasurement_t appliedRange;

    QVector <bool> consumerRequests;

    bool propertyChangedFlag = false;

    QDoubleSpinBox * holdEdit = nullptr;
    QDoubleSpinBox * holdEditOrig;

    QCheckBox * holdRefEdit;
    QCheckBox * holdRefEditOrig;

    QString holdName;

    QLabel * sweepsNumLbl;
    QSpinBox * sweepsNumEdit;
    QSpinBox * sweepsNumEditOrig;

    QComboBox * currentRangeEdit;
    QComboBox * currentRangeEditOrig;

    QComboBox * voltageRangeEdit;
    QComboBox * voltageRangeEditOrig;

    QComboBox * samplingRateEdit;
    QComboBox * samplingRateEditOrig;

    QPushButton * propertyOkButton = nullptr;
    QPushButton * propertyCancButton = nullptr;

    QVector <ProtocolDropControlItem *> * ctrlItems = nullptr;
    QVector <ProtocolCtrlDispatcher *> * ctrlDispatchers = nullptr;
    ProtocolEditor * protocolEditor = nullptr;
    QStringList protocolsNames;
    std::vector <YAML::Analysis_t> analysesNode;

    QVector <ProtocolItem *> pushedProtocolItems;
    QVector <ProtocolItem *> protocolItems;
    ProtocolSections * protocolSections = nullptr;
    ItemsProcStatus_t pushedProcessingStatus = ItemsNotProcessed;
    ItemsProcStatus_t processingStatus = ItemsNotProcessed;

    QVector <TriggerCursor *> pushedTriggerCursors;
    QVector <TriggerCursor *> triggerCursors;
    unsigned int maxTriggerEvents = 1;

    bool protocolValid = true;
    bool protocolInhibited = false;
    bool cursorsValid = true;

private slots:
    void onCheckNameAvailability(QString name);
    void onPropertyChanged();

signals:
    void nameChanged(QString, QString);
};

class VoltageProtocolWidget : virtual public ProtocolWidget {
public:
    VoltageProtocolWidget();

    void setStimulusRangeIndex(int idx) override;
    int getStimulusRangeIndex() override;
    int getCurrentRangeIndex() override;
    int getVoltageRangeIndex() override;
};

class CurrentProtocolWidget : virtual public ProtocolWidget {
public:
    CurrentProtocolWidget();

    void setStimulusRangeIndex(int idx) override;
    int getStimulusRangeIndex() override;
    int getCurrentRangeIndex() override;
    int getVoltageRangeIndex() override;
};

class GapfreeProtocolWidget : virtual public ProtocolWidget {
public:
    GapfreeProtocolWidget();

    Measurement_t getTotalDuration() override;
    ProtocolSection * getItemAtTime(double time, int sweepIdx, double &offset) override;
    ProtocolSection * getItemAtTime(double time, int itemIdx, int repsIdx, int sweepIdx, double &offset) override;
};

class EpisodicProtocolWidget : virtual public ProtocolWidget {
public:
    EpisodicProtocolWidget();

    Measurement_t getTotalDuration() override;
    ProtocolSection * getItemAtTime(double time, int sweepIdx, double &offset) override;
    ProtocolSection * getItemAtTime(double time, int itemIdx, int repsIdx, int sweepIdx, double &offset) override;
};

class GapfreeVoltageProtocolWidget : public VoltageProtocolWidget, public GapfreeProtocolWidget {
    Q_OBJECT

public:
    GapfreeVoltageProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog);
};

class EpisodicVoltageProtocolWidget : public VoltageProtocolWidget, public EpisodicProtocolWidget {
    Q_OBJECT

public:
    EpisodicVoltageProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog);
};

class GapfreeCurrentProtocolWidget : public CurrentProtocolWidget, public GapfreeProtocolWidget {
    Q_OBJECT

public:
    GapfreeCurrentProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog);
};

class EpisodicCurrentProtocolWidget : public CurrentProtocolWidget, public EpisodicProtocolWidget {
    Q_OBJECT

public:
    EpisodicCurrentProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog);
};

class ProtocolCtrlDispatcher : public QObject {
    Q_OBJECT

public:
    ProtocolCtrlDispatcher();

    void setCtrl(ProtocolDropControlItem * ctrl);

    virtual void setOrigValueBack() = 0;

protected:
    ProtocolDropControlItem * ctrl = nullptr;
};

class ProtocolBoolCtrlDispatcher : public ProtocolCtrlDispatcher {
    Q_OBJECT

public:
    ProtocolBoolCtrlDispatcher();

    void setOrigValue(bool value);

    virtual void setOrigValueBack() = 0;

protected:
    bool origValue;
};

class ProtocolCheckBoxCtrlDispatcher : public ProtocolBoolCtrlDispatcher {
    Q_OBJECT

public:
    ProtocolCheckBoxCtrlDispatcher();

    void setEdit(QCheckBox * e);

    virtual void setOrigValueBack() override;

public slots:
    void onCtrlChanged(bool value);

private:
    QCheckBox * objEdit = nullptr;
};

class ProtocolIntCtrlDispatcher : public ProtocolCtrlDispatcher {
    Q_OBJECT

public:
    ProtocolIntCtrlDispatcher();

    void setOrigValue(int value);

    virtual void setOrigValueBack() = 0;

protected:
    int origValue;
};

class ProtocolSpinBoxCtrlDispatcher : public ProtocolIntCtrlDispatcher {
    Q_OBJECT

public:
    ProtocolSpinBoxCtrlDispatcher();

    void setEdit(QSpinBox * objEdit);

    virtual void setOrigValueBack() override;

public slots:
    void onCtrlChanged(int value);

private:
    QSpinBox * objEdit = nullptr;
};

class ProtocolComboBoxCtrlDispatcher : public ProtocolIntCtrlDispatcher {
    Q_OBJECT

public:
    ProtocolComboBoxCtrlDispatcher();

    void setEdit(QComboBox * objEdit);

    virtual void setOrigValueBack() override;

public slots:
    void onCtrlChanged(int value);

private:
    QComboBox * objEdit = nullptr;
};

class ProtocolDoubleCtrlDispatcher : public ProtocolCtrlDispatcher {
    Q_OBJECT

public:
    ProtocolDoubleCtrlDispatcher();

    void setOrigValue(double value);

    virtual void setOrigValueBack() = 0;

protected:
    double origValue;
};

class ProtocolDoubleSpinBoxCtrlDispatcher : public ProtocolDoubleCtrlDispatcher {
    Q_OBJECT

public:
    ProtocolDoubleSpinBoxCtrlDispatcher();

    void setEdit(QDoubleSpinBox * objEdit);

    virtual void setOrigValueBack() override;

public slots:
    void onCtrlChanged(double value);

private:
    QDoubleSpinBox * objEdit = nullptr;
};

#endif // PROTOCOLWIDGET_H
