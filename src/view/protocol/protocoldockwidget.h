#ifndef PROTOCOLDOCKWIDGET_H
#define PROTOCOLDOCKWIDGET_H

#include <QDockWidget>
#include <QPushButton>

#include "protocollist.h"
#include "protocolpropertydialog.h"
#include "addtagdialog.h"
#include "lcddisplay.h"

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
    void onNewRecordFile(QString fileName);
    void onPlotting(bool flag, ProtocolType_t type);
    void onNullProtocol();
    void onRecording(bool flag);
    void onProtocolEnded();

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

private:
    void setProtocolListVisibility();
    void setRecordFile(QString path, QString name);

    ModelDevice *  mDev;
    ProtocolPropertyDialog * protocolPropertyDialog = nullptr;
    ProtocolList * voltageProtocolList = nullptr;
    ProtocolList * currentProtocolList = nullptr;
    ClampingModality_t clampingModality;
    QString recordPath;
    QPushButton * recordFileBtn;
    QPushButton * recordProtocolBtn;
    QPushButton * saveLastProtocolBtn;
    AddTagDialog * tagDlg = nullptr;
    TimerDisplay * protocolTimer = nullptr;

signals:
    void plotting(bool);
    void sweep(int, int);
    void stimulusApplied(double value, e384CommLib::RangedMeasurement_t range);
    void holdApplied(double value, e384CommLib::RangedMeasurement_t range);
    void enableTags(bool);
    void markTagTime();
    void saveTagString(QString);
};

#endif // PROTOCOLDOCKWIDGET_H
