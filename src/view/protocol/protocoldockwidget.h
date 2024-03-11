#ifndef PROTOCOLDOCKWIDGET_H
#define PROTOCOLDOCKWIDGET_H

#include <QDockWidget>
#include <QPushButton>

#include "protocollist.h"
#include "protocolpropertydialog.h"
#include "addtagdialog.h"
#include "lcddisplay.h"
#include "globaldefines.h"
#include "messagedispatcher.h"

class ProtocolDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ProtocolDockWidget(MessageDispatcher * msgDisp, ClampingModality_t clampingModality, QWidget * parent = nullptr);
    ~ProtocolDockWidget();

    ProtocolList * getProtocolList();
    ProtocolList * getVoltageProtocolList();
    ProtocolList * getCurrentProtocolList();

public slots:
    void onSetClampingModality(ClampingModality_t clampingModality);

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

protected slots:
    void onStartProtocol(bool flag);
    void onRestartProtocol(bool flag);

private:
    void setProtocolListVisibility();

    MessageDispatcher * msgDisp = nullptr;
    ProtocolPropertyDialog * protocolPropertyDialog = nullptr;
    ProtocolList * voltageProtocolList = nullptr;
    ProtocolList * currentProtocolList = nullptr;
    ClampingModality_t clampingModality = e384CommLib::VOLTAGE_CLAMP;
    TimerDisplay * protocolTimer = nullptr;

signals:
    void startProtocol();
    void restartProtocol();
    void stopProtocol();
};

#endif // PROTOCOLDOCKWIDGET_H
