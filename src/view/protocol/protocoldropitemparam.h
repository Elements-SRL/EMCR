#ifndef PROTOCOLDROPITEMPARAM_H
#define PROTOCOLDROPITEMPARAM_H

#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>

#include "protocolitemctrlmanager.h"

class ProtocolDropItem;
class ProtocolDropControlItem;

typedef enum {
    ProtocolItemCtrlVoltage,
    ProtocolItemCtrlCurrent,
    ProtocolItemCtrlTime,
    ProtocolItemCtrlFrequency,
    ProtocolItemCtrlNaturalNum,
    ProtocolItemCtrlNone
} ProtocolItemCtrlTypes_t;

class ProtocolDropItemParam : public QObject {
    Q_OBJECT

public:
    ProtocolDropItemParam(ProtocolItemCtrlManager * ctrlManager, ProtocolItemCtrlTypes_t ctrlType,
                          QString name, QString unit);
    virtual ~ProtocolDropItemParam();

    void setName(QString n);
    void setUnit(QString u);
    virtual void setVisible(bool visible) = 0;

    QLabel * getNameWidget();
    QLabel * getUnitWidget();
    QComboBox * getCtrlWidget();
    bool isEnabled();

    virtual void updateCtrlWidget() = 0;
    virtual void setValueToEditWidget() = 0;
    virtual void acceptEditWidget() = 0;
    virtual void rejectEditWidget() = 0;

protected:
    ProtocolItemCtrlManager * ctrlManager;
    ProtocolItemCtrlTypes_t ctrlType;

    QLabel * nameWidget = nullptr;
    QLabel * unitWidget = nullptr;
    QComboBox * ctrlWidget = nullptr;
    bool enabled = true;

    ProtocolDropItem * ctrlDelegateItem;
    QVector <ProtocolDropControlItem *> * ctrlItems;

public slots:
    virtual void onCtrlWidgetActivated(int cbxIdx) = 0;

signals:
    void setWidgetString();
};

class ProtocolDropItemIntParam : public ProtocolDropItemParam {
    Q_OBJECT

public:
    ProtocolDropItemIntParam(ProtocolItemCtrlManager * ctrlManager, ProtocolItemCtrlTypes_t ctrlType, int value,
                             QString name, QSpinBox * editWidget, QString unit);

    void setValue(int v);
    void setCtrlSign(int cs);
    void setVisible(bool visible) override;

    int getValue();
    QSpinBox * getEditWidget();
    int getCtrlSign();

    void updateCtrlWidget() override;
    void setValueToEditWidget() override;
    void acceptEditWidget() override;
    void rejectEditWidget() override;

public slots:
    void onSetCtrlValue(int cv);
    void onCtrlWidgetActivated(int cbxIdx) override;

private:
    int value = 1;

    QSpinBox * editWidget;

    int ctrlSign = 1;
};

class ProtocolDropItemDoubleParam : public ProtocolDropItemParam {
    Q_OBJECT

public:
    ProtocolDropItemDoubleParam(ProtocolItemCtrlManager * ctrlManager, ProtocolItemCtrlTypes_t ctrlType, double value,
                                QString name, QDoubleSpinBox * editWidget, QString unit);

    void setValue(double v);
    void setCtrlSign(double cs);
    void setVisible(bool visible) override;

    double getValue();
    QDoubleSpinBox * getEditWidget();
    double getCtrlSign();

    void updateCtrlWidget() override;
    void setValueToEditWidget() override;
    void acceptEditWidget() override;
    void rejectEditWidget() override;

public slots:
    void onSetCtrlValue(double cv);
    void onCtrlWidgetActivated(int cbxIdx) override;

private:
    double value = 0.0;

    QDoubleSpinBox * editWidget;

    double ctrlSign = 1.0;
};

#endif // PROTOCOLDROPITEMPARAM_H
