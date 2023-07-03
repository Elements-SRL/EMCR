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
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    void onNewRecordFile(QString fileName);
    void onRecording(bool flag);
#endif

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

protected slots:
    void onStartProtocol(bool flag);

private:
    void setProtocolListVisibility();
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    void setRecordFile(QString path, QString name);
#endif

    MessageDispatcher * msgDisp = nullptr;
    ProtocolPropertyDialog * protocolPropertyDialog = nullptr;
    ProtocolList * voltageProtocolList = nullptr;
    ProtocolList * currentProtocolList = nullptr;
    ClampingModality_t clampingModality = e384CommLib::VOLTAGE_CLAMP;
    TimerDisplay * protocolTimer = nullptr;
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    QString recordPath;
    QPushButton * recordFileBtn;
    QPushButton * recordProtocolBtn;
    QPushButton * saveLastProtocolBtn;
    AddTagDialog * tagDlg = nullptr;
#endif

signals:
    void startProtocol();
    void stopProtocol();
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    void enableTags(bool);
    void markTagTime();
    void saveTagString(QString);
#endif
};

#endif // PROTOCOLDOCKWIDGET_H
