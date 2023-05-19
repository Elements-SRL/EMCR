#ifndef PROTOCOLDOCKWIDGET_H
#define PROTOCOLDOCKWIDGET_H

#include <QDockWidget>
#include <QPushButton>

#include "protocollist.h"
#include "protocolpropertydialog.h"
#include "addtagdialog.h"
#include "lcddisplay.h"
#include "globaldefines.h"

class ProtocolDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ProtocolDockWidget(ModelDevice * mDev, ClampingModality_t clampingModality, QWidget * parent = nullptr);
    ~ProtocolDockWidget();

    ProtocolList * getProtocolList();
    ProtocolList * getVoltageProtocolList();
    ProtocolList * getCurrentProtocolList();

public slots:
    void onSetClampingModality(ClampingModality_t clampingModality);
    void onPlotting(bool flag, ProtocolType_t type);
    void onNullProtocol();
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    void onNewRecordFile(QString fileName);
    void onRecording(bool flag);
#endif

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

private:
    void setProtocolListVisibility();
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    void setRecordFile(QString path, QString name);
#endif

    ModelDevice *  mDev;
    ProtocolPropertyDialog * protocolPropertyDialog = nullptr;
    ProtocolList * voltageProtocolList = nullptr;
    ProtocolList * currentProtocolList = nullptr;
    ClampingModality_t clampingModality;
    TimerDisplay * protocolTimer = nullptr;
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    QString recordPath;
    QPushButton * recordFileBtn;
    QPushButton * recordProtocolBtn;
    QPushButton * saveLastProtocolBtn;
    AddTagDialog * tagDlg = nullptr;
#endif

signals:
    void plotting(bool);
    void sweep(int, int);
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    void enableTags(bool);
    void markTagTime();
    void saveTagString(QString);
#endif
};

#endif // PROTOCOLDOCKWIDGET_H
