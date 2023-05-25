#include "protocoldropitemparam.h"

#include "protocoldropitem.h"

ProtocolDropItemParam::ProtocolDropItemParam(
        ProtocolItemCtrlManager * ctrlManager, ProtocolItemCtrlTypes_t ctrlType,
        QString name, QString unit) :
    ctrlManager(ctrlManager),
    ctrlType(ctrlType) {

    nameWidget = new QLabel(name);
    unitWidget = new QLabel(unit);
    ctrlWidget = new QComboBox();
    ctrlWidget->addItem("<No controls>");
}

ProtocolDropItemParam::~ProtocolDropItemParam() {

}

void ProtocolDropItemParam::setName(QString n) {
    nameWidget->setText(n);
}

void ProtocolDropItemParam::setUnit(QString u) {
    unitWidget->setText(u);
}

QLabel * ProtocolDropItemParam::getNameWidget() {
    return nameWidget;
}

QLabel * ProtocolDropItemParam::getUnitWidget() {
    return unitWidget;
}

QComboBox * ProtocolDropItemParam::getCtrlWidget() {
    return ctrlWidget;
}

bool ProtocolDropItemParam::isEnabled() {
    return enabled;
}

ProtocolDropItemIntParam::ProtocolDropItemIntParam(
        ProtocolItemCtrlManager * ctrlManager, ProtocolItemCtrlTypes_t ctrlType, int value,
        QString name, QSpinBox * editWidget, QString unit) :
    ProtocolDropItemParam(ctrlManager, ctrlType, name, unit),
    value(value),
    editWidget(editWidget) {

    editWidget->setValue(value);

    connect(ctrlWidget, QOverload <int> ::of(&QComboBox::activated), this, &ProtocolDropItemParam::onCtrlWidgetActivated);
}

void ProtocolDropItemIntParam::setValue(int v) {
    value = v;
    editWidget->setValue(value);
}

void ProtocolDropItemIntParam::setCtrlSign(int cs) {
    ctrlSign = cs;
}

void ProtocolDropItemIntParam::setVisible(bool visible) {
    enabled = visible;
    nameWidget->setVisible(visible);
    editWidget->setVisible(visible);
    unitWidget->setVisible(visible);
    ctrlWidget->setVisible(visible);
}

int ProtocolDropItemIntParam::getValue() {
    return value;
}

QSpinBox * ProtocolDropItemIntParam::getEditWidget() {
    return editWidget;
}

int ProtocolDropItemIntParam::getCtrlSign() {
    return ctrlSign;
}

void ProtocolDropItemIntParam::setValueToEditWidget() {
    value = editWidget->value();
    emit setWidgetString();
}

void ProtocolDropItemIntParam::acceptEditWidget() {
    if (editWidget->isEnabled()) {
        value = editWidget->value();
    }

    int cbxIdx = ctrlWidget->currentIndex();
    switch (ctrlType) {
    case ProtocolItemCtrlNaturalNum:
        ctrlDelegateItem = ctrlManager->manageNaturalNumCtrlConnections(cbxIdx, this);
        break;

    case ProtocolItemCtrlVoltage:
    case ProtocolItemCtrlCurrent:
    case ProtocolItemCtrlTime:
    case ProtocolItemCtrlFrequency:
    case ProtocolItemCtrlNone:
        ctrlDelegateItem = nullptr;
        break;
    }
}

void ProtocolDropItemIntParam::rejectEditWidget() {
    if (ctrlDelegateItem == nullptr) {
        editWidget->setEnabled(true);

    } else {
        editWidget->setEnabled(false);
    }

    if (editWidget->isEnabled()) {
        editWidget->setValue(value);
    }
}

void ProtocolDropItemIntParam::updateCtrlWidget() {
    switch (ctrlType) {
    case ProtocolItemCtrlNaturalNum:
        ctrlItems = ctrlManager->getNaturalNumCtrlItems();
        break;

    case ProtocolItemCtrlVoltage:
    case ProtocolItemCtrlCurrent:
    case ProtocolItemCtrlTime:
    case ProtocolItemCtrlFrequency:
    case ProtocolItemCtrlNone:
        return;
    }

    int cbxCount = ctrlWidget->count()-1; /*!< Do not consider the -No controls- option */
    for (int cbxIdx = 0; cbxIdx < cbxCount; cbxIdx++) {
        ctrlWidget->removeItem(1);
    }

    bool ctrlDelegateFound = false;
    for (int ctrlIdx = 0; ctrlIdx < ctrlItems->size(); ctrlIdx++) {
        QString name = ctrlItems->at(ctrlIdx)->getName();
        ctrlWidget->addItem(name);
        ctrlWidget->addItem("- " + name);

        /*! Disable option with minus sign if the widget can't accept negative values */
        if (editWidget->minimum() >= 0) {
            QStandardItemModel * model = qobject_cast <QStandardItemModel *> (ctrlWidget->model());
            QStandardItem * item = model->item(ctrlWidget->count()-1);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }

        if (ctrlDelegateItem == ctrlItems->at(ctrlIdx)) {
            ctrlDelegateFound = true;
            if (ctrlSign > 0) {
                ctrlWidget->setCurrentIndex(ctrlIdx*2+1);

            } else {
                ctrlWidget->setCurrentIndex(ctrlIdx*2+2);
            }
        }
    }

    if (ctrlDelegateFound) {
        editWidget->setEnabled(false);

    } else {
        ctrlDelegateItem = nullptr;
        editWidget->setEnabled(true);
    }
}

void ProtocolDropItemIntParam::onSetCtrlValue(int cv) {
    value = cv*ctrlSign;
    emit setWidgetString();
}

void ProtocolDropItemIntParam::onCtrlWidgetActivated(int cbxIdx) {
    if (cbxIdx == 0) {
        editWidget->setEnabled(true);

    } else {
        editWidget->setDisabled(true);

        /*! Set the value from the control into the greyed box */
        switch (ctrlType) {
        case ProtocolItemCtrlNaturalNum:
            editWidget->setValue(ctrlManager->getNaturalNumCtrlValue(cbxIdx));
            break;

        case ProtocolItemCtrlVoltage:
        case ProtocolItemCtrlCurrent:
        case ProtocolItemCtrlTime:
        case ProtocolItemCtrlFrequency:
        case ProtocolItemCtrlNone:
            break;
        }
    }
}

ProtocolDropItemDoubleParam::ProtocolDropItemDoubleParam(
        ProtocolItemCtrlManager * ctrlManager, ProtocolItemCtrlTypes_t ctrlType, double value,
        QString name, QDoubleSpinBox * editWidget, QString unit) :
    ProtocolDropItemParam(ctrlManager, ctrlType, name, unit),
    value(value),
    editWidget(editWidget) {

    editWidget->setValue(value);

    connect(ctrlWidget, QOverload <int> ::of(&QComboBox::activated), this, &ProtocolDropItemParam::onCtrlWidgetActivated);
}

void ProtocolDropItemDoubleParam::setValue(double v) {
    value = v;
    editWidget->setValue(value);
}

void ProtocolDropItemDoubleParam::setCtrlSign(double cs) {
    ctrlSign = cs;
}

void ProtocolDropItemDoubleParam::setVisible(bool visible) {
    enabled = visible;
    nameWidget->setVisible(visible);
    editWidget->setVisible(visible);
    unitWidget->setVisible(visible);
    ctrlWidget->setVisible(visible);
}

double ProtocolDropItemDoubleParam::getValue() {
    return value;
}

QDoubleSpinBox * ProtocolDropItemDoubleParam::getEditWidget() {
    return editWidget;
}

double ProtocolDropItemDoubleParam::getCtrlSign() {
    return ctrlSign;
}

void ProtocolDropItemDoubleParam::setValueToEditWidget() {
    value = editWidget->value();
    emit setWidgetString();
}

void ProtocolDropItemDoubleParam::acceptEditWidget() {
    if (editWidget->isEnabled()) {
        value = editWidget->value();
    }

    int cbxIdx = ctrlWidget->currentIndex();
    switch (ctrlType) {
    case ProtocolItemCtrlVoltage:
        ctrlDelegateItem = ctrlManager->manageVoltageCtrlConnections(cbxIdx, this);
        break;

    case ProtocolItemCtrlCurrent:
        ctrlDelegateItem = ctrlManager->manageCurrentCtrlConnections(cbxIdx, this);
        break;

    case ProtocolItemCtrlTime:
        ctrlDelegateItem = ctrlManager->manageTimeCtrlConnections(cbxIdx, this);
        break;

    case ProtocolItemCtrlFrequency:
        ctrlDelegateItem = ctrlManager->manageFrequencyCtrlConnections(cbxIdx, this);
        break;

    case ProtocolItemCtrlNaturalNum:
    case ProtocolItemCtrlNone:
        ctrlDelegateItem = nullptr;
        break;
    }
}

void ProtocolDropItemDoubleParam::rejectEditWidget() {
    if (ctrlDelegateItem == nullptr) {
        editWidget->setEnabled(true);

    } else {
        editWidget->setEnabled(false);
    }

    if (editWidget->isEnabled()) {
        editWidget->setValue(value);
    }
}

void ProtocolDropItemDoubleParam::updateCtrlWidget() {
    switch (ctrlType) {
    case ProtocolItemCtrlVoltage:
        ctrlItems = ctrlManager->getVoltageCtrlItems();
        break;

    case ProtocolItemCtrlCurrent:
        ctrlItems = ctrlManager->getCurrentCtrlItems();
        break;

    case ProtocolItemCtrlTime:
        ctrlItems = ctrlManager->getTimeCtrlItems();
        break;

    case ProtocolItemCtrlFrequency:
        ctrlItems = ctrlManager->getFrequencyCtrlItems();
        break;

    case ProtocolItemCtrlNaturalNum:
    case ProtocolItemCtrlNone:
        return;
    }

    int cbxCount = ctrlWidget->count()-1; /*!< Do not consider the -No controls- option */
    for (int cbxIdx = 0; cbxIdx < cbxCount; cbxIdx++) {
        ctrlWidget->removeItem(1);
    }

    bool ctrlDelegateFound = false;
    for (int ctrlIdx = 0; ctrlIdx < ctrlItems->size(); ctrlIdx++) {
        QString name = ctrlItems->at(ctrlIdx)->getName();
        ctrlWidget->addItem(name);
        ctrlWidget->addItem("- " + name);

        /*! Disable option with minus sign if the widget can't accept negative values */
        if (editWidget->minimum() >= 0.0) {
            QStandardItemModel * model = qobject_cast <QStandardItemModel *> (ctrlWidget->model());
            QStandardItem * item = model->item(ctrlWidget->count()-1);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }

        if (ctrlDelegateItem == ctrlItems->at(ctrlIdx)) {
            ctrlDelegateFound = true;
            if (ctrlSign > 0.0) {
                ctrlWidget->setCurrentIndex(ctrlIdx*2+1);

            } else {
                ctrlWidget->setCurrentIndex(ctrlIdx*2+2);
            }
        }
    }

    if (ctrlDelegateFound) {
        editWidget->setEnabled(false);

    } else {
        ctrlDelegateItem = nullptr;
        editWidget->setEnabled(true);
    }
}

void ProtocolDropItemDoubleParam::onSetCtrlValue(double cv) {
    value = cv*ctrlSign;
    emit setWidgetString();
}

void ProtocolDropItemDoubleParam::onCtrlWidgetActivated(int cbxIdx) {
    if (cbxIdx == 0) {
        editWidget->setEnabled(true);

    } else {
        editWidget->setDisabled(true);

        /*! Set the value from the control into the greyed box */
        switch (ctrlType) {
        case ProtocolItemCtrlVoltage:
            editWidget->setValue(ctrlManager->getVoltageCtrlValue(cbxIdx));
            break;

        case ProtocolItemCtrlCurrent:
            editWidget->setValue(ctrlManager->getCurrentCtrlValue(cbxIdx));
            break;

        case ProtocolItemCtrlTime:
            editWidget->setValue(ctrlManager->getTimeCtrlValue(cbxIdx));
            break;

        case ProtocolItemCtrlFrequency:
            editWidget->setValue(ctrlManager->getFrequencyCtrlValue(cbxIdx));
            break;

        case ProtocolItemCtrlNaturalNum:
        case ProtocolItemCtrlNone:
            break;
        }
    }
}

ProtocolDropAnalysisItemParam::ProtocolDropAnalysisItemParam(QString n) {
    nameWidget = new QLabel(n);
    timePointWidget = new QComboBox();
    timePointWidget->addItem("<No cursor selected>");
}

void ProtocolDropAnalysisItemParam::setName(QString n) {
    nameWidget->setText(n);
}

QLabel * ProtocolDropAnalysisItemParam::getNameWidget() {
    return nameWidget;
}

QComboBox * ProtocolDropAnalysisItemParam::getTimePointWidget() {
    return timePointWidget;
}

int ProtocolDropAnalysisItemParam::getCursorIdx() {
    return timePointWidget->currentIndex();
}

void ProtocolDropAnalysisItemParam::acceptEditWidget() {

}

void ProtocolDropAnalysisItemParam::rejectEditWidget() {

}

void ProtocolDropAnalysisItemParam::addCursors(int cursorsNum) {
    for (int cbxIdx = timePointWidget->count(); cbxIdx < cursorsNum+1; cbxIdx++) {
        timePointWidget->addItem(QString("Cursor %1").arg(cbxIdx));
    }
}

void ProtocolDropAnalysisItemParam::removeCursors(QVector <int> cursorsMap) {
    int currentSelection = timePointWidget->currentIndex();
    int cbxCount = timePointWidget->count(); /*!< Do not consider the -No cursors- option */
    int prevIdx = cursorsMap.size()-1;
    for (int cbxIdx = cbxCount-1; cbxIdx > 0; cbxIdx--, prevIdx--) {
        if (cursorsMap[prevIdx] < 0) {
            if (currentSelection == cbxIdx) {
                timePointWidget->setCurrentIndex(0);
            }
            timePointWidget->removeItem(cbxIdx);

        } else {
            timePointWidget->setItemText(cbxIdx, QString("Cursor %1").arg(cursorsMap[prevIdx]+1));
        }
    }
}
