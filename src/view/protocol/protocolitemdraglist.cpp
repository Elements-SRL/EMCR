#include "protocolitemdraglist.h"

#include <QtWidgets>

ProtocolItemDragList::ProtocolItemDragList() {
    this->setDragDropMode(QAbstractItemView::DragOnly);
    this->setIconSize(QSize(50, 50));
}

QToolButton * ProtocolItemDragList::setSeparator(QString title, QColor color) {
    QToolButton * hideShowBtn = new QToolButton();
    hideShowBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    hideShowBtn->setStyleSheet(QString("QToolButton {border: none;background:rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
    hideShowBtn->setArrowType(Qt::ArrowType::DownArrow);
    hideShowBtn->setText(title);
    hideShowBtn->setCheckable(true);
    hideShowBtn->setChecked(true);

    connect(hideShowBtn, &QToolButton::clicked, this, [=](bool show) {
        if (show) {
            hideShowBtn->setArrowType(Qt::ArrowType::DownArrow);

        } else {
            hideShowBtn->setArrowType(Qt::ArrowType::RightArrow);
        }
    });

    this->setItemWidget(this->item(this->count()-1), hideShowBtn);

    return hideShowBtn;
}

void ProtocolItemDragList::setSeparatorItems(QToolButton * btn, QVector <int> itemIdxs) {
    for (int idx = 0; idx < itemIdxs.size(); idx++) {
        connect(btn, &QToolButton::clicked, this, [=](bool show) {
            this->item(itemIdxs[idx])->setHidden(!show);
        });
    }
}

void ProtocolItemDragList::mousePressEvent(QMouseEvent * event) {
    /*! Parent method called to induce the selection of the pressed item */
    QListWidget::mousePressEvent(event);

    ProtocolDragItem * item = static_cast <ProtocolDragItem *> (this->currentItem());

    if (!item) {
        return;
    }

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << item->type();

    QMimeData * mimeData = new QMimeData;

    mimeData->setData(PROT_DRAG_LIST_WIDGET_ITEM_MIME_FORMAT, itemData);

    switch (item->type()) {
    case PROT_DRAG_LIST_VSTEP_TSTEP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VSTEP_TSTEP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VSTEP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VSTEP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_ISTEP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_ISTEP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VTSTEP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VTSTEP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_ITSTEP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_ITSTEP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VCONST_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VCONST_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_ICONST_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_ICONST_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VHOLD_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VHOLD_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IHOLD_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IHOLD_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VREST_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VREST_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IREST_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IREST_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VRAMP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VRAMP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IRAMP_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IRAMP_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VSIN_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VSIN_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_ISIN_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_ISIN_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VREP_SEQ_SCALED_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IREP_SEQ_SCALED_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IREP_SEQ_SCALED_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VREP_SEQ_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VREP_SEQ_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IREP_SEQ_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IREP_SEQ_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VREP_SEQ_WITH_STEPS_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IREP_SEQ_WITH_STEPS_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VINF_REP_SEQ_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VINF_REP_SEQ_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IINF_REP_SEQ_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IINF_REP_SEQ_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_CONTROL_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_CONTROL_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VOLTAGE_CONTROL_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VOLTAGE_CONTROL_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_CURRENT_CONTROL_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_CURRENT_CONTROL_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_TIME_CONTROL_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_TIME_CONTROL_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_FREQUENCY_CONTROL_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_FREQUENCY_CONTROL_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_NATURAL_NUM_CONTROL_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_ANALYSIS_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_ANALYSIS_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_NOISE_REPORT_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_NOISE_REPORT_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_HISTOGRAM_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_HISTOGRAM_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_SPECTRUM_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_SPECTRUM_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_RESISTANCE_ESTIMATION_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_RESISTANCE_ESTIMATION_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_MEMBRANE_TEST_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_MEMBRANE_TEST_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_IV_GRAPH_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_IV_GRAPH_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_VOLTAGE_TRACKING_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_VOLTAGE_TRACKING_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_AP_THRESHOLD_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_AP_THRESHOLD_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_AP_STATISTICS_ITEM_TYPE:
        mimeData->setData(PROT_DRAG_LIST_AP_STATISTICS_ITEM_MIME_FORMAT, itemData);
        break;

    case PROT_DRAG_LIST_SEPARATOR_TYPE:
        mimeData->setData(PROT_DRAG_LIST_SEPARATOR_MIME_FORMAT, itemData);
        break;
    }

    QDrag * drag = new QDrag(this);
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction);
}
