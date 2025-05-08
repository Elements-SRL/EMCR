#ifndef PROTOCOLDOCKWIDGET_H
#define PROTOCOLDOCKWIDGET_H

#include <QDockWidget>
#include <QPushButton>
#include <QShortcut>

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

    ProtocolList * getVoltageProtocolList();
    ProtocolList * getAnalysisVoltageProtocolList();
    ProtocolList * getCurrentProtocolList();
    ProtocolList * getAnalysisCurrentProtocolList();

public slots:
    void onSetClampingModality(ClampingModality_t clampingModality);
    void onSetAnalysisProtocols(bool analysisProtocols);

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

protected slots:
    void onStartProtocol(bool flag);
    void onRestartProtocol(bool flag);

private:
    void setProtocolListVisibility();

    MessageDispatcher * msgDisp = nullptr;
    QPushButton * addProtocolBtn = nullptr;
    QPushButton * removeProtocolBtn = nullptr;
    QPushButton * editProtocolBtn = nullptr;
    QPushButton * copyProtocolBtn = nullptr;
    QPushButton * setProtocolsShortCutsBtn = nullptr;
    QPushButton * importProtocolBtn = nullptr;
    QPushButton * exportProtocolBtn = nullptr;
    ProtocolPropertyDialog * protocolPropertyDialog = nullptr;
    ProtocolList * voltageProtocolList = nullptr;
    ProtocolList * analysisVoltageProtocolList = nullptr;
    ProtocolList * currentProtocolList = nullptr;
    ProtocolList * analysisCurrentProtocolList = nullptr;
    ClampingModality_t clampingModality = e384CommLib::VOLTAGE_CLAMP;
    bool analysisProtocolsFlag = false;
    TimerDisplay * protocolTimer = nullptr;
    QVector <QShortcut *> shortcuts;

signals:
    void startProtocol();
    void restartProtocol();
    void stopProtocol();
};

#endif // PROTOCOLDOCKWIDGET_H
