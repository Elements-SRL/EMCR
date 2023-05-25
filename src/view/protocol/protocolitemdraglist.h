#ifndef PROTOCOLITEMDRAGLIST_H
#define PROTOCOLITEMDRAGLIST_H

#include <QListWidget>

#include "protocoldropitem.h"
#include "protocoldragitem.h"
#include "globaldefines.h"

class ProtocolItemDragList : public QListWidget {
public:
    ProtocolItemDragList();

    QToolButton * setSeparator(QString title, QColor color = Qt::white);
    void setSeparatorItems(QToolButton * btn, QVector <int> itemIdxs);

protected:
    void mousePressEvent(QMouseEvent * event) override;
};

#endif // PROTOCOLITEMDRAGLIST_H
