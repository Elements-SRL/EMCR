#include "protocoldropitem.h"
#include "protocol/protocolutils.h"

static int voltageControlItemIdx = 0;
static int currentControlItemIdx = 0;
static int timeControlItemIdx = 0;
static int frequencyControlItemIdx = 0;
static int naturalNumControlItemIdx = 0;

ProtocolDropItem::ProtocolDropItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    QObject(), QListWidgetItem(QString("Drop"), nullptr, type),
    mDev(mDev),
    ctrlManager(ctrlManager),
    hold(hold0) {

    /*! Creates a grid that contains the item properties */
    propertyDialog = new QDialog();

    propertyDialog->setWindowFlags(Qt::WindowTitleHint);
    propertyDialog->setWindowTitle("Property panel");

    QVBoxLayout * mainVl = new QVBoxLayout;
    mainVl->setSizeConstraint(QLayout::SetFixedSize);
    propertyDialog->setLayout(mainVl);

    propertyLo = new QGridLayout();
    mainVl->addLayout(propertyLo);

    descriptionName = new QLabel("Add protocol item");

    propertyLo->addWidget(descriptionName, PDI_DESCRIPTION_ROW, 0, 1, -1);
    descriptionName->setVisible(false);

    visible = false;
    visibleEdit = new QCheckBox("Receive Data");
    visibleEdit->setChecked(visible);

    propertyLo->addWidget(visibleEdit, PDI_VISIBLE_CHECK_ROW, 0, 1, -1);

    QHBoxLayout * okCancHl = new QHBoxLayout;
    mainVl->addLayout(okCancHl);

    QPushButton * okButton = new QPushButton("OK");
    okButton->setCheckable(false);
    okCancHl->addWidget(okButton);
    connect(okButton, &QPushButton::clicked, this, &ProtocolDropItem::onAcceptPropertyDialog);
    okButton->setDefault(true);

    QPushButton * cancButton = new QPushButton("Cancel");
    cancButton->setCheckable(false);
    okCancHl->addWidget(cancButton);
    connect(cancButton, &QPushButton::clicked, this, &ProtocolDropItem::onRejectPropertyDialog);

    QPushButton * delButton = new QPushButton("DELETE");
    delButton->setCheckable(false);
    okCancHl->addWidget(delButton);
    connect(delButton, &QPushButton::clicked, this, [=] () {
        this->onAcceptPropertyDialog();
        emit protocolDropItemDeleteRequest(this);
    });

    propertyDialog->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        stimulusAbbrName = "V";
        stimulusName = "Voltage";
        stimulusCtrlType = ProtocolItemCtrlVoltage;

        /*! Collect protocol information from mDev */
        mDev->getMessageDispatcher()->getVoltageProtocolRangeFeature(0, stimulusRange);

    } else {
        stimulusAbbrName = "I";
        stimulusName = "Current";
        stimulusCtrlType = ProtocolItemCtrlCurrent;

        mDev->getMessageDispatcher()->getCurrentProtocolRangeFeature(0, stimulusRange);
    }

    mDev->getMessageDispatcher()->getTimeProtocolRangeFeature(timeRange);
    timeRange.convertValues(UnitPfxMilli);
    timeDecimals = timeRange.decimals();

    mDev->getMessageDispatcher()->getFrequencyProtocolRangeFeature(frequencyRange);
    frequencyRange.convertValues(UnitPfxNone);
    frequencyDecimals = frequencyRange.decimals();

    editWidgets.clear();
    dropItemParams.clear();
}

ProtocolDropItem::~ProtocolDropItem() {
    if (propertyDialog != nullptr) {
        delete propertyDialog;
        propertyDialog = nullptr;
    }

//    for (int idx = 0; idx < editWidgets.size(); idx++) {
//        QWidget * w = editWidgets[idx];
//        if (w != nullptr) {
//            delete w;
//            w = nullptr;
//        }
//    }
//    editWidgets.clear();

    for (int idx = 0; idx < dropItemParams.size(); idx++) {
        ProtocolDropItemParam * dip = dropItemParams[idx];
        if (dip != nullptr) {
            delete dip;
            dip = nullptr;
        }
    }
    dropItemParams.clear();
}

bool ProtocolDropItem::hasVisibleData() {
    return visible;
}

ProtocolItemCtrlTypes_t ProtocolDropItem::getItemCtrlType() {
    return protocolItemCtrlType;
}

ProtocolConsumerType_t ProtocolDropItem::getConsumerType() {
    return protocolConsumerType;
}

void ProtocolDropItem::onAcceptPropertyDialog() {
    this->onSetString();
    emit updateItem();

    if (propertyDialog->isVisible()) {
        propertyDialog->accept();
    }
}

void ProtocolDropItem::onRejectPropertyDialog() {
    if (propertyDialog->isVisible()) {
        propertyDialog->reject();
    }
}

ProtocolDropStimulusItem::ProtocolDropStimulusItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropItem(mDev, ctrlManager, hold0, clampingModality, type) {
    this->setBackground(PROT_EDITOR_STIMULUS_ITEM_COLOR);

    propertyLo->setColumnStretch(0, 2);
    propertyLo->setColumnStretch(1, 3);
    propertyLo->setColumnStretch(2, 1);
    propertyLo->setColumnStretch(3, 2);
    propertyLo->setColumnStretch(4, 2);
}

QString ProtocolDropStimulusItem::getName() {
    return "";
}

ProtocolDropXStepTStepItem::ProtocolDropXStepTStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropStimulusItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/stimulus step time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    descriptionName->setVisible(false);

    propertyDialog->setWindowTitle(stimulusAbbrName + " step t step property panel");

    int rowIdx = PDI_FIRST_PARAM_ROW;

    /*! Param x0 */ {
        double value = hold+100.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        x0Param = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                  "First " + stimulusName.toLower(), editWidget,
                                                  QString::fromStdString(stimulusRange.getFullUnit()));

        connect(x0Param, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(x0Param->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(x0Param->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(x0Param->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(x0Param->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(x0Param);
        x0EditWidget = editWidget;
    }

    /*! Param xStep */ {
        double value = 0.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        xStepParam = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                     stimulusName + "step", editWidget,
                                                     QString::fromStdString(stimulusRange.getFullUnit()));

        connect(xStepParam, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(xStepParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(xStepParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(xStepParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(xStepParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(xStepParam);
        xStepEditWidget = editWidget;
    }

    /*! Param t0 */ {
        double value = 100.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, timeRange, RangedQDoubleSpinBox_t::MIN_MAX);

        t0Param = new ProtocolDropItemDoubleParam(ctrlManager, ProtocolItemCtrlTime, value,
                                                  "First duration", editWidget,
                                                  QString::fromStdString(timeRange.getFullUnit()));

        connect(t0Param, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(t0Param->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(t0Param->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(t0Param->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(t0Param->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(t0Param);
    }

    /*! Param tStep */ {
        double value = 0.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, timeRange, RangedQDoubleSpinBox_t::DELTA);

        tStepParam = new ProtocolDropItemDoubleParam(ctrlManager, ProtocolItemCtrlTime, value,
                                                     "Duration step", editWidget,
                                                     QString::fromStdString(timeRange.getFullUnit()));

        connect(tStepParam, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(tStepParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(tStepParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(tStepParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(tStepParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(tStepParam);
    }

    this->onSetString();
}

void ProtocolDropXStepTStepItem::openPropertyDialog() {
    x0Param->updateCtrlWidget();
    xStepParam->updateCtrlWidget();
    t0Param->updateCtrlWidget();
    tStepParam->updateCtrlWidget();
    propertyDialog->exec();
}

void ProtocolDropXStepTStepItem::setStimulusRange(RangedMeasurement_t &range) {
    stimulusRange = range;
    initQdoubleSpinBox(x0EditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    x0EditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    x0Param->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));

    initQdoubleSpinBox(x0EditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    xStepEditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    xStepParam->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));
    this->onAcceptPropertyDialog();
}

double ProtocolDropXStepTStepItem::getX0() {
    return x0Param->getValue();
}

double ProtocolDropXStepTStepItem::getXStep() {
    return xStepParam->getValue();
}

double ProtocolDropXStepTStepItem::getT0() {
    return t0Param->getValue();
}

double ProtocolDropXStepTStepItem::getTStep() {
    return tStepParam->getValue();
}

ProtocolDropItemDoubleParam * ProtocolDropXStepTStepItem::getX0Param() {
    return x0Param;
}

ProtocolDropItemDoubleParam * ProtocolDropXStepTStepItem::getXStepParam() {
    return xStepParam;
}

ProtocolDropItemDoubleParam * ProtocolDropXStepTStepItem::getT0Param() {
    return t0Param;
}

ProtocolDropItemDoubleParam * ProtocolDropXStepTStepItem::getTStepParam() {
    return tStepParam;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlVStepTStep() {
    YAML::VStepTStep yamlPhase;

    yamlPhase.v0 = x0Param->getValue();
    yamlPhase.vstep = xStepParam->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.tstep = tStepParam->getValue();
    yamlPhase.v0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.vstepctrl = xStepParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.tstepctrl = tStepParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlIStepTStep() {
    YAML::IStepTStep yamlPhase;

    yamlPhase.i0 = x0Param->getValue();
    yamlPhase.istep = xStepParam->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.tstep = tStepParam->getValue();
    yamlPhase.i0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.istepctrl = xStepParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.tstepctrl = tStepParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlVConst() {
    YAML::VConst yamlPhase;

    yamlPhase.v0 = x0Param->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.v0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlIConst() {
    YAML::IConst yamlPhase;

    yamlPhase.i0 = x0Param->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.i0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlVHold() {
    YAML::VHold yamlPhase;

    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlIHold() {
    YAML::IHold yamlPhase;

    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlVRest() {
    YAML::VRest yamlPhase;

    yamlPhase.v0 = x0Param->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.v0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXStepTStepItem::getYamlIRest() {
    YAML::IRest yamlPhase;

    yamlPhase.i0 = x0Param->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.i0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::VStepTStep_t &yamlPhase) {
    x0Param->setValue(yamlPhase.v0);
    x0Param->updateCtrlWidget();
    xStepParam->setValue(yamlPhase.vstep);
    xStepParam->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    tStepParam->setValue(yamlPhase.tstep);
    tStepParam->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.v0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    xStepParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.vstepctrl));
    xStepParam->onCtrlWidgetActivated(xStepParam->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    tStepParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.tstepctrl));
    tStepParam->onCtrlWidgetActivated(tStepParam->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::IStepTStep_t &yamlPhase) {
    x0Param->setValue(yamlPhase.i0);
    x0Param->updateCtrlWidget();
    xStepParam->setValue(yamlPhase.istep);
    xStepParam->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    tStepParam->setValue(yamlPhase.tstep);
    tStepParam->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.i0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    xStepParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.istepctrl));
    xStepParam->onCtrlWidgetActivated(xStepParam->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    tStepParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.tstepctrl));
    tStepParam->onCtrlWidgetActivated(tStepParam->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::VConst_t &yamlPhase) {
    x0Param->setValue(yamlPhase.v0);
    x0Param->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.v0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::IConst_t &yamlPhase) {
    x0Param->setValue(yamlPhase.i0);
    x0Param->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.i0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::VHold_t &yamlPhase) {
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::IHold_t &yamlPhase) {
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::VRest_t &yamlPhase) {
    x0Param->setValue(yamlPhase.v0);
    x0Param->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.v0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::setPhaseFromYaml(const YAML::IRest_t &yamlPhase) {
    x0Param->setValue(yamlPhase.i0);
    x0Param->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.i0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::onSetString() {
    this->setText(QString(stimulusAbbrName + "0: %1, " + stimulusAbbrName + " step: %2, t0: %3, t step: %4")
                  .arg(x0Param->getValue()).arg(xStepParam->getValue())
                  .arg(t0Param->getValue()).arg(tStepParam->getValue()));
}

void ProtocolDropXStepTStepItem::onAcceptPropertyDialog() {
    x0Param->acceptEditWidget();
    xStepParam->acceptEditWidget();
    t0Param->acceptEditWidget();
    tStepParam->acceptEditWidget();
    visible = visibleEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXStepTStepItem::onRejectPropertyDialog() {
    x0Param->rejectEditWidget();
    xStepParam->rejectEditWidget();
    t0Param->rejectEditWidget();
    tStepParam->rejectEditWidget();
    visibleEdit->setChecked(visible);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXStepTStepItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVStepTStepItem::ProtocolDropVStepTStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXStepTStepItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIStepTStepItem::ProtocolDropIStepTStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXStepTStepItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    x0Param->setValue(hold+1.0);
    this->onSetString();
}

ProtocolDropXStepItem::ProtocolDropXStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXStepTStepItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/stimulus step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle(stimulusAbbrName + " step property panel");

    t0Param->setName("Duration");

    tStepParam->setValue(0.0);
    tStepParam->setVisible(false);

    this->onSetString();
}

void ProtocolDropXStepItem::onSetString() {
    this->setText(QString(stimulusAbbrName + "0: %1, " + stimulusAbbrName + " step: %2, t: %3")
                  .arg(x0Param->getValue()).arg(xStepParam->getValue()).arg(t0Param->getValue()));
}

void ProtocolDropXStepItem::onAcceptPropertyDialog() {
    x0Param->acceptEditWidget();
    xStepParam->acceptEditWidget();
    t0Param->acceptEditWidget();
    visible = visibleEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXStepItem::onRejectPropertyDialog() {
    x0Param->rejectEditWidget();
    xStepParam->rejectEditWidget();
    t0Param->rejectEditWidget();
    visibleEdit->setChecked(visible);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXStepItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVStepItem::ProtocolDropVStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXStepItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIStepItem::ProtocolDropIStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXStepItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    x0Param->setValue(hold+1.0);
    this->onSetString();
}

ProtocolDropXTStepItem::ProtocolDropXTStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXStepTStepItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("t step property panel");

    x0Param->setName(stimulusName);

    xStepParam->setValue(0.0);
    xStepParam->setVisible(false);

    this->onSetString();
}

void ProtocolDropXTStepItem::onSetString() {
    this->setText(QString(stimulusAbbrName + ": %1, t0: %2, t step: %3")
                  .arg(x0Param->getValue()).arg(t0Param->getValue()).arg(tStepParam->getValue()));
}

void ProtocolDropXTStepItem::onAcceptPropertyDialog() {
    x0Param->acceptEditWidget();
    t0Param->acceptEditWidget();
    tStepParam->acceptEditWidget();
    visible = visibleEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXTStepItem::onRejectPropertyDialog() {
    x0Param->rejectEditWidget();
    t0Param->rejectEditWidget();
    tStepParam->rejectEditWidget();
    visibleEdit->setChecked(visible);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXTStepItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVTStepItem::ProtocolDropVTStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXTStepItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropITStepItem::ProtocolDropITStepItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXTStepItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    x0Param->setValue(hold+1.0);
    this->onSetString();
}

ProtocolDropXConstItem::ProtocolDropXConstItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXStepTStepItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/constant stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle(stimulusAbbrName + " const panel");

    x0Param->setName(stimulusName);

    xStepParam->setValue(0.0);
    xStepParam->setVisible(false);

    t0Param->setName("Duration");

    tStepParam->setValue(0.0);
    tStepParam->setVisible(false);

    this->onSetString();
}

void ProtocolDropXConstItem::onSetString() {
    this->setText(QString(stimulusAbbrName + ": %1, t: %2")
                  .arg(x0Param->getValue()).arg(t0Param->getValue()));
}

void ProtocolDropXConstItem::onAcceptPropertyDialog() {
    x0Param->acceptEditWidget();
    t0Param->acceptEditWidget();
    visible = visibleEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXConstItem::onRejectPropertyDialog() {
    x0Param->rejectEditWidget();
    t0Param->rejectEditWidget();
    visibleEdit->setChecked(visible);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXConstItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVConstItem::ProtocolDropVConstItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXConstItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIConstItem::ProtocolDropIConstItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXConstItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    x0Param->setValue(hold+1.0);
    this->onSetString();
}

ProtocolDropXHoldItem::ProtocolDropXHoldItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXStepTStepItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/holding stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle(stimulusAbbrName + " hold property panel");

    x0Param->setValue(hold);
    x0Param->setVisible(false);

    xStepParam->setValue(0.0);
    xStepParam->setVisible(false);

    t0Param->setName("Duration");

    tStepParam->setValue(0.0);
    tStepParam->setVisible(false);

    this->onSetString();
}

void ProtocolDropXHoldItem::onSetString() {
    this->setText(QString("t: %1")
                  .arg(t0Param->getValue()));
}

void ProtocolDropXHoldItem::onAcceptPropertyDialog() {
    t0Param->acceptEditWidget();
    visible = visibleEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXHoldItem::onRejectPropertyDialog() {
    t0Param->rejectEditWidget();
    visibleEdit->setChecked(visible);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXHoldItem::onUpdateHold(double value) {
    hold = value;
    x0Param->setValue(hold);
}

ProtocolDropVHoldItem::ProtocolDropVHoldItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXHoldItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIHoldItem::ProtocolDropIHoldItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXHoldItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDropXRestItem::ProtocolDropXRestItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXStepTStepItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/rest.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Rest property panel");

    descriptionName->setVisible(true);

    descriptionName->setText("Add a rest time at a fixed " + stimulusName.toLower() + " while not receiving data");

    x0Param->setName(stimulusName);

    xStepParam->setValue(0.0);
    xStepParam->setVisible(false);

    t0Param->setName("Duration");

    tStepParam->setValue(0.0);
    tStepParam->setVisible(false);

    visible = false;
    visibleEdit->setVisible(false);
    visibleEdit->setEnabled(false);

    this->onSetString();
}

void ProtocolDropXRestItem::onSetString() {
    this->setText(QString(stimulusAbbrName + ": %1, t: %2")
                  .arg(x0Param->getValue()).arg(t0Param->getValue()));
}

void ProtocolDropXRestItem::onAcceptPropertyDialog() {
    x0Param->acceptEditWidget();
    t0Param->acceptEditWidget();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXRestItem::onRejectPropertyDialog() {
    x0Param->rejectEditWidget();
    t0Param->rejectEditWidget();

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXRestItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVRestItem::ProtocolDropVRestItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRestItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIRestItem::ProtocolDropIRestItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRestItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    x0Param->setValue(hold+1.0);
    this->onSetString();
}

ProtocolDropXRampItem::ProtocolDropXRampItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropStimulusItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/stimulus ramp.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle(stimulusAbbrName + " ramp property panel");

    descriptionName->setVisible(false);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    /*! Param x0 */ {
        double value = hold;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        x0Param = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                  "Initial " + stimulusName.toLower(), editWidget,
                                                  QString::fromStdString(stimulusRange.getFullUnit()));

        connect(x0Param, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(x0Param->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(x0Param->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(x0Param->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(x0Param->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(x0Param);
        x0EditWidget = editWidget;
    }

    /*! Param xFinal */ {
        double value = hold+100.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        xFinalParam = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                      "Final " + stimulusName.toLower(), editWidget,
                                                      QString::fromStdString(stimulusRange.getFullUnit()));

        connect(xFinalParam, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(xFinalParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(xFinalParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(xFinalParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(xFinalParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(xFinalParam);
        xFinalEditWidget = editWidget;
    }

    /*! Param t0 */ {
        double value = 100.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, timeRange, RangedQDoubleSpinBox_t::MIN_MAX);
//        editWidget->setRangedMeasurement(timeRange, QDoubleSpinBox::MinMaxRange);

        t0Param = new ProtocolDropItemDoubleParam(ctrlManager, ProtocolItemCtrlTime, value,
                                                  "Ramp duration", editWidget,
                                                  QString::fromStdString(timeRange.getFullUnit()));

        connect(t0Param, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(t0Param->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(t0Param->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(t0Param->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(t0Param->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(t0Param);
    }

    this->onSetString();
}

void ProtocolDropXRampItem::openPropertyDialog() {
    x0Param->updateCtrlWidget();
    xFinalParam->updateCtrlWidget();
    t0Param->updateCtrlWidget();
    propertyDialog->exec();
}

double ProtocolDropXRampItem::getX0() {
    return x0Param->getValue();
}

double ProtocolDropXRampItem::getXFinal() {
    return xFinalParam->getValue();
}

double ProtocolDropXRampItem::getT0() {
    return t0Param->getValue();
}

ProtocolDropItemDoubleParam * ProtocolDropXRampItem::getX0Param() {
    return x0Param;
}

ProtocolDropItemDoubleParam * ProtocolDropXRampItem::getXFinalParam() {
    return xFinalParam;
}

ProtocolDropItemDoubleParam * ProtocolDropXRampItem::getT0Param() {
    return t0Param;
}

YAML::Phase_t ProtocolDropXRampItem::getYamlVRamp() {
    YAML::VRamp yamlPhase;

    yamlPhase.v0 = x0Param->getValue();
    yamlPhase.vfinal = xFinalParam->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.v0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.vfinalctrl = xFinalParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXRampItem::getYamlIRamp() {
    YAML::IRamp yamlPhase;

    yamlPhase.i0 = x0Param->getValue();
    yamlPhase.ifinal = xFinalParam->getValue();
    yamlPhase.t0 = t0Param->getValue();
    yamlPhase.i0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.ifinalctrl = xFinalParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.t0ctrl = t0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

void ProtocolDropXRampItem::setPhaseFromYaml(const YAML::VRamp_t &yamlPhase) {
    x0Param->setValue(yamlPhase.v0);
    x0Param->updateCtrlWidget();
    xFinalParam->setValue(yamlPhase.vfinal);
    xFinalParam->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.v0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    xFinalParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.vfinalctrl));
    xFinalParam->onCtrlWidgetActivated(xFinalParam->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXRampItem::setPhaseFromYaml(const YAML::IRamp_t &yamlPhase) {
    x0Param->setValue(yamlPhase.i0);
    x0Param->updateCtrlWidget();
    xFinalParam->setValue(yamlPhase.ifinal);
    xFinalParam->updateCtrlWidget();
    t0Param->setValue(yamlPhase.t0);
    t0Param->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.i0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    xFinalParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.ifinalctrl));
    xFinalParam->onCtrlWidgetActivated(xFinalParam->getCtrlWidget()->currentIndex());
    t0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.t0ctrl));
    t0Param->onCtrlWidgetActivated(t0Param->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXRampItem::onSetString() {
    this->setText(QString(stimulusAbbrName + "0: %1, " + stimulusAbbrName + " final: %2, t: %3")
                  .arg(x0Param->getValue()).arg(xFinalParam->getValue()).arg(t0Param->getValue()));
}

void ProtocolDropXRampItem::onAcceptPropertyDialog() {
    x0Param->acceptEditWidget();
    xFinalParam->acceptEditWidget();
    t0Param->acceptEditWidget();
    visible = visibleEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXRampItem::setStimulusRange(RangedMeasurement_t &range) {
    stimulusRange = range;

    initQdoubleSpinBox(x0EditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    x0EditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    x0Param->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));

    initQdoubleSpinBox(xFinalEditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    xFinalEditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    xFinalParam->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));
    this->onAcceptPropertyDialog();
}

void ProtocolDropXRampItem::onRejectPropertyDialog() {
    x0Param->rejectEditWidget();
    xFinalParam->rejectEditWidget();
    t0Param->rejectEditWidget();
    visibleEdit->setChecked(visible);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXRampItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVRampItem::ProtocolDropVRampItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRampItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIRampItem::ProtocolDropIRampItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRampItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    xFinalParam->setValue(hold+1.0);
    this->onSetString();
}

ProtocolDropXSinItem::ProtocolDropXSinItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropStimulusItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/stimulus sin.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle(stimulusAbbrName + " sinusoid property panel");

    descriptionName->setVisible(false);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    /*! Param x0 */ {
        double value = hold;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        x0Param = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                  "Offset " + stimulusName.toLower(), editWidget,
                                                  QString::fromStdString(stimulusRange.getFullUnit()));

        connect(x0Param, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(x0Param->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(x0Param->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(x0Param->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(x0Param->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(x0Param);
        x0EditWidget = editWidget;
    }

    /*! Param xAmp */ {
        double value = hold+100.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        xAmpParam = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                    "Oscillation amplitude", editWidget,
                                                    QString::fromStdString(stimulusRange.getFullUnit()));

        connect(xAmpParam, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(xAmpParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(xAmpParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(xAmpParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(xAmpParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(xAmpParam);
        xAmpEditWidget = editWidget;
    }

    /*! Param freq */ {
        double value = 10.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, frequencyRange, RangedQDoubleSpinBox_t::MIN_MAX);
//        editWidget->setRangedMeasurement(frequencyRange, QDoubleSpinBox::MinMaxRange);

        freqParam = new ProtocolDropItemDoubleParam(ctrlManager, ProtocolItemCtrlFrequency, value,
                                                    "Oscillation frequency", editWidget,
                                                    QString::fromStdString(frequencyRange.getFullUnit()));

        connect(freqParam, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(freqParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(freqParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(freqParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(freqParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(freqParam);
    }

    this->onSetString();
}

void ProtocolDropXSinItem::openPropertyDialog() {
    x0Param->updateCtrlWidget();
    xAmpParam->updateCtrlWidget();
    freqParam->updateCtrlWidget();
    propertyDialog->exec();
}

double ProtocolDropXSinItem::getX0() {
    return x0Param->getValue();
}

double ProtocolDropXSinItem::getXAmp() {
    return xAmpParam->getValue();
}

double ProtocolDropXSinItem::getFreq() {
    return freqParam->getValue();
}

ProtocolDropItemDoubleParam * ProtocolDropXSinItem::getX0Param() {
    return x0Param;
}

ProtocolDropItemDoubleParam * ProtocolDropXSinItem::getXAmpParam() {
    return xAmpParam;
}

ProtocolDropItemDoubleParam * ProtocolDropXSinItem::getFreqParam() {
    return freqParam;
}

YAML::Phase_t ProtocolDropXSinItem::getYamlVSin() {
    YAML::VSin yamlPhase;

    yamlPhase.v0 = x0Param->getValue();
    yamlPhase.vamp = xAmpParam->getValue();
    yamlPhase.freq = freqParam->getValue();
    yamlPhase.v0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.vampctrl = xAmpParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.freqctrl = freqParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXSinItem::getYamlISin() {
    YAML::ISin yamlPhase;

    yamlPhase.i0 = x0Param->getValue();
    yamlPhase.iamp = xAmpParam->getValue();
    yamlPhase.freq = freqParam->getValue();
    yamlPhase.i0ctrl = x0Param->getCtrlWidget()->currentText().toStdString();
    yamlPhase.iampctrl = xAmpParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.freqctrl = freqParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.visible = visibleEdit->isChecked();

    return yamlPhase;
}

void ProtocolDropXSinItem::setPhaseFromYaml(const YAML::VSin_t &yamlPhase) {
    x0Param->setValue(yamlPhase.v0);
    x0Param->updateCtrlWidget();
    xAmpParam->setValue(yamlPhase.vamp);
    xAmpParam->updateCtrlWidget();
    freqParam->setValue(yamlPhase.freq);
    freqParam->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.v0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    xAmpParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.vampctrl));
    xAmpParam->onCtrlWidgetActivated(xAmpParam->getCtrlWidget()->currentIndex());
    freqParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.freqctrl));
    freqParam->onCtrlWidgetActivated(freqParam->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXSinItem::setPhaseFromYaml(const YAML::ISin_t &yamlPhase) {
    x0Param->setValue(yamlPhase.i0);
    x0Param->updateCtrlWidget();
    xAmpParam->setValue(yamlPhase.iamp);
    xAmpParam->updateCtrlWidget();
    freqParam->setValue(yamlPhase.freq);
    freqParam->updateCtrlWidget();
    x0Param->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.i0ctrl));
    x0Param->onCtrlWidgetActivated(x0Param->getCtrlWidget()->currentIndex());
    xAmpParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.iampctrl));
    xAmpParam->onCtrlWidgetActivated(xAmpParam->getCtrlWidget()->currentIndex());
    freqParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.freqctrl));
    freqParam->onCtrlWidgetActivated(freqParam->getCtrlWidget()->currentIndex());
    visibleEdit->setChecked(yamlPhase.visible);

    this->onAcceptPropertyDialog();
}

void ProtocolDropXSinItem::onSetString() {
    this->setText(QString(stimulusAbbrName + "0: %1, " + stimulusAbbrName + " amp: %2, freq: %3")
                  .arg(x0Param->getValue()).arg(xAmpParam->getValue()).arg(freqParam->getValue()));
}

void ProtocolDropXSinItem::onAcceptPropertyDialog() {
    x0Param->acceptEditWidget();
    xAmpParam->acceptEditWidget();
    freqParam->acceptEditWidget();
    visible = visibleEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXSinItem::setStimulusRange(RangedMeasurement_t &range) {
    stimulusRange = range;

    initQdoubleSpinBox(x0EditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    x0EditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    x0Param->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));

    initQdoubleSpinBox(xAmpEditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    xAmpEditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    xAmpParam->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));
    this->onAcceptPropertyDialog();
}

void ProtocolDropXSinItem::onRejectPropertyDialog() {
    x0Param->rejectEditWidget();
    xAmpParam->rejectEditWidget();
    freqParam->rejectEditWidget();
    visibleEdit->setChecked(visible);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXSinItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVSinItem::ProtocolDropVSinItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXSinItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropISinItem::ProtocolDropISinItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXSinItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    xAmpParam->setValue(hold+1.0);
    this->onSetString();
}

ProtocolDropLoopsItem::ProtocolDropLoopsItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropItem(mDev, ctrlManager, hold0, clampingModality, type) {

    this->setBackground(PROT_EDITOR_LOOPS_ITEM_COLOR);

    propertyLo->setColumnStretch(0, 2);
    propertyLo->setColumnStretch(1, 3);
    propertyLo->setColumnStretch(2, 1);
    propertyLo->setColumnStretch(3, 2);
    propertyLo->setColumnStretch(4, 2);
}

QString ProtocolDropLoopsItem::getName() {
    return "";
}

ProtocolDropXRepSeqScaledItem::ProtocolDropXRepSeqScaledItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropLoopsItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/P over N.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("P/N property panel");

    descriptionName->setText("Repeat shifted and scaled versions of the preceding items");
    descriptionName->setVisible(true);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    /*! Param holdLeak */ {
        double value = hold-100.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        holdLeakParam = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                         "Leak holding " + stimulusName.toLower(), editWidget,
                                                         QString::fromStdString(stimulusRange.getFullUnit()));

        connect(holdLeakParam, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(holdLeakParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(holdLeakParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(holdLeakParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(holdLeakParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(holdLeakParam);
        holdLeakEditWidget = editWidget;
    }

    /*! Param scaleFactor */ {
        int value = 5;

        QSpinBox * editWidget = new QSpinBox();
        editWidget->setRange(1, 20);

        scaleFactorParam = new ProtocolDropItemIntParam(ctrlManager, ProtocolItemCtrlNaturalNum, value,
                                                        "Down scale factor", editWidget, "");

        connect(scaleFactorParam, &ProtocolDropItemIntParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(scaleFactorParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(scaleFactorParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(scaleFactorParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(scaleFactorParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        dropItemParams.push_back(scaleFactorParam);
    }

    /*! Param repsNum */ {
        int value = 4;

        QSpinBox * editWidget = new QSpinBox();
        editWidget->setRange(1, 100);

        repNumParam = new ProtocolDropItemIntParam(ctrlManager, ProtocolItemCtrlNaturalNum, value,
                                                   "Repetitions", editWidget, "");

        connect(repNumParam, &ProtocolDropItemIntParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(repNumParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(repNumParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(repNumParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(repNumParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        dropItemParams.push_back(repNumParam);
    }

    /*! Param itemNum */ {
        int value = 3;

        QSpinBox * editWidget = new QSpinBox();
        editWidget->setRange(1, 100);

        itemNumParam = new ProtocolDropItemIntParam(ctrlManager, ProtocolItemCtrlNaturalNum, value,
                                                    "Affected items", editWidget, "");

        connect(itemNumParam, &ProtocolDropItemIntParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(itemNumParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(itemNumParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(itemNumParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(itemNumParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        dropItemParams.push_back(itemNumParam);
    }

    /*! Param restStimulus */ {
        double value = hold;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//        editWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);

        restStimulusParam = new ProtocolDropItemDoubleParam(ctrlManager, stimulusCtrlType, value,
                                                           "Preceding rest " + stimulusName.toLower(), editWidget,
                                                           QString::fromStdString(stimulusRange.getFullUnit()));

        connect(restStimulusParam, &ProtocolDropItemDoubleParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(restStimulusParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(restStimulusParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(restStimulusParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(restStimulusParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(restStimulusParam);
        restStimulusEditWidget = editWidget;
    }

    /*! Param restTime */ {
        double value = 0.0;

        QDoubleSpinBox * editWidget = new QDoubleSpinBox();
        initQdoubleSpinBox(editWidget, timeRange, RangedQDoubleSpinBox_t::ZERO_MAX);
//        editWidget->setRangedMeasurement(timeRange, QDoubleSpinBox::ZeroMaxRange);

        restTimeParam = new ProtocolDropItemDoubleParam(ctrlManager, ProtocolItemCtrlTime, value,
                                                        "Preceding rest time", editWidget,
                                                        QString::fromStdString(timeRange.getFullUnit()));

        connect(restTimeParam, &ProtocolDropItemIntParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(restTimeParam->getNameWidget(), rowIdx, 0, Qt::AlignRight);
        propertyLo->addWidget(restTimeParam->getEditWidget(), rowIdx, 1);
        propertyLo->addWidget(restTimeParam->getUnitWidget(), rowIdx, 2);
        propertyLo->addWidget(restTimeParam->getCtrlWidget(), rowIdx++, 3, 1, 2);

        editWidgets.push_back(editWidget);
        dropItemParams.push_back(restTimeParam);
    }

    preceding = false;
    precedingEdit = new QCheckBox("Precede main pulse");
    precedingEdit->setChecked(preceding);

    propertyLo->addWidget(precedingEdit, rowIdx++, 0, 1, -1);

    reversed = false;
    reversedEdit = new QCheckBox("Reversed P/N pulse");
    reversedEdit->setChecked(reversed);

    propertyLo->addWidget(reversedEdit, rowIdx++, 0, 1, -1);

    alternating = false;
    alternatingEdit = new QCheckBox("Alternate P/N pulses polarization");
    alternatingEdit->setChecked(preceding);

    propertyLo->addWidget(alternatingEdit, rowIdx++, 0, 1, -1);

    visible = false;
    visibleEdit->setVisible(false);
    visibleEdit->setEnabled(false);

    applySteps = false;

    this->onSetString();
}

void ProtocolDropXRepSeqScaledItem::openPropertyDialog() {
    holdLeakParam->updateCtrlWidget();
    scaleFactorParam->updateCtrlWidget();
    repNumParam->updateCtrlWidget();
    itemNumParam->updateCtrlWidget();
    restStimulusParam->updateCtrlWidget();
    restTimeParam->updateCtrlWidget();
    propertyDialog->exec();
}

void ProtocolDropXRepSeqScaledItem::setStimulusRange(RangedMeasurement_t &range) {
    stimulusRange = range;

    initQdoubleSpinBox(holdLeakEditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    holdLeakEditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    holdLeakParam->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));

    initQdoubleSpinBox(restStimulusEditWidget, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    restStimulusEditWidget->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    restStimulusParam->setUnit(QString::fromStdString(stimulusRange.getFullUnit()));
    this->onAcceptPropertyDialog();
}

double ProtocolDropXRepSeqScaledItem::getHoldLeak() {
    return holdLeakParam->getValue();
}

int ProtocolDropXRepSeqScaledItem::getScaleFactor() {
    return scaleFactorParam->getValue();
}

int ProtocolDropXRepSeqScaledItem::getRepsNum() {
    return repNumParam->getValue();
}

int ProtocolDropXRepSeqScaledItem::getItemNum() {
    return itemNumParam->getValue();
}

double ProtocolDropXRepSeqScaledItem::getRestStimulus() {
    return restStimulusParam->getValue();
}

double ProtocolDropXRepSeqScaledItem::getRestTime() {
    return restTimeParam->getValue();
}

bool ProtocolDropXRepSeqScaledItem::getPreceding() {
    return preceding;
}

bool ProtocolDropXRepSeqScaledItem::getReversed() {
    return reversed;
}

bool ProtocolDropXRepSeqScaledItem::getAlternating() {
    return alternating;
}

bool ProtocolDropXRepSeqScaledItem::getApplySteps() {
    return applySteps;
}

ProtocolDropItemDoubleParam * ProtocolDropXRepSeqScaledItem::getHoldLeakParam() {
    return holdLeakParam;
}

ProtocolDropItemIntParam * ProtocolDropXRepSeqScaledItem::getScaleFactorParam() {
    return scaleFactorParam;
}

ProtocolDropItemIntParam * ProtocolDropXRepSeqScaledItem::getRepNumParam() {
    return repNumParam;
}

ProtocolDropItemIntParam * ProtocolDropXRepSeqScaledItem::getItemNumParam() {
    return itemNumParam;
}

ProtocolDropItemDoubleParam * ProtocolDropXRepSeqScaledItem::getRestStimulusParam() {
    return restStimulusParam;
}

ProtocolDropItemDoubleParam * ProtocolDropXRepSeqScaledItem::getRestTimeParam() {
    return restTimeParam;
}

YAML::Phase_t ProtocolDropXRepSeqScaledItem::getYamlRepSeqScaled() {
    YAML::RepSeqScaled yamlPhase;

    yamlPhase.vholdleak = holdLeakParam->getValue();
    yamlPhase.scalefactor = scaleFactorParam->getValue();
    yamlPhase.repnum = repNumParam->getValue();
    yamlPhase.itemnum = itemNumParam->getValue();
    yamlPhase.reststimulus = restStimulusParam->getValue();
    yamlPhase.resttime = restTimeParam->getValue();
    yamlPhase.preceding = precedingEdit->isChecked();
    yamlPhase.reversed = reversedEdit->isChecked();
    yamlPhase.alternating = alternatingEdit->isChecked();
    yamlPhase.vholdleakctrl = holdLeakParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.scalefactorctrl = scaleFactorParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.repnumctrl = repNumParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.itemnumctrl = itemNumParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.reststimulusctrl = restStimulusParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.resttimectrl = restTimeParam->getCtrlWidget()->currentText().toStdString();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXRepSeqScaledItem::getYamlRepSeq() {
    YAML::RepSeq yamlPhase;

    yamlPhase.repnum = repNumParam->getValue();
    yamlPhase.itemnum = itemNumParam->getValue();
    yamlPhase.reststimulus = restStimulusParam->getValue();
    yamlPhase.resttime = restTimeParam->getValue();
    yamlPhase.repnumctrl = repNumParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.itemnumctrl = itemNumParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.reststimulusctrl = restStimulusParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.resttimectrl = restTimeParam->getCtrlWidget()->currentText().toStdString();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXRepSeqScaledItem::getYamlRepSeqWithSteps() {
    YAML::RepSeqWithSteps yamlPhase;

    yamlPhase.repnum = repNumParam->getValue();
    yamlPhase.itemnum = itemNumParam->getValue();
    yamlPhase.reststimulus = restStimulusParam->getValue();
    yamlPhase.resttime = restTimeParam->getValue();
    yamlPhase.repnumctrl = repNumParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.itemnumctrl = itemNumParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.reststimulusctrl = restStimulusParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.resttimectrl = restTimeParam->getCtrlWidget()->currentText().toStdString();

    return yamlPhase;
}

YAML::Phase_t ProtocolDropXRepSeqScaledItem::getYamlInfRepSeq() {
    YAML::InfRepSeq yamlPhase;

    yamlPhase.itemnum = itemNumParam->getValue();
    yamlPhase.reststimulus = restStimulusParam->getValue();
    yamlPhase.resttime = restTimeParam->getValue();
    yamlPhase.itemnumctrl = itemNumParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.reststimulusctrl = restStimulusParam->getCtrlWidget()->currentText().toStdString();
    yamlPhase.resttimectrl = restTimeParam->getCtrlWidget()->currentText().toStdString();

    return yamlPhase;
}

void ProtocolDropXRepSeqScaledItem::setPhaseFromYaml(const YAML::RepSeqScaled_t &yamlPhase) {
    holdLeakParam->setValue(yamlPhase.vholdleak);
    holdLeakParam->updateCtrlWidget();
    scaleFactorParam->setValue(yamlPhase.scalefactor);
    scaleFactorParam->updateCtrlWidget();
    repNumParam->setValue(yamlPhase.repnum);
    repNumParam->updateCtrlWidget();
    itemNumParam->setValue(yamlPhase.itemnum);
    itemNumParam->updateCtrlWidget();
    restStimulusParam->setValue(yamlPhase.reststimulus);
    restStimulusParam->updateCtrlWidget();
    restTimeParam->setValue(yamlPhase.resttime);
    restTimeParam->updateCtrlWidget();
    precedingEdit->setChecked(yamlPhase.preceding);
    reversedEdit->setChecked(yamlPhase.reversed);
    alternatingEdit->setChecked(yamlPhase.alternating);
    holdLeakParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.vholdleakctrl));
    holdLeakParam->onCtrlWidgetActivated(holdLeakParam->getCtrlWidget()->currentIndex());
    scaleFactorParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.scalefactorctrl));
    scaleFactorParam->onCtrlWidgetActivated(scaleFactorParam->getCtrlWidget()->currentIndex());
    repNumParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.repnumctrl));
    repNumParam->onCtrlWidgetActivated(repNumParam->getCtrlWidget()->currentIndex());
    itemNumParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.itemnumctrl));
    itemNumParam->onCtrlWidgetActivated(itemNumParam->getCtrlWidget()->currentIndex());
    restStimulusParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.reststimulusctrl));
    restStimulusParam->onCtrlWidgetActivated(restStimulusParam->getCtrlWidget()->currentIndex());
    restTimeParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.resttimectrl));
    restTimeParam->onCtrlWidgetActivated(restTimeParam->getCtrlWidget()->currentIndex());

    this->onAcceptPropertyDialog();
}

void ProtocolDropXRepSeqScaledItem::setPhaseFromYaml(const YAML::RepSeq_t &yamlPhase) {
    repNumParam->setValue(yamlPhase.repnum);
    repNumParam->updateCtrlWidget();
    itemNumParam->setValue(yamlPhase.itemnum);
    itemNumParam->updateCtrlWidget();
    restStimulusParam->setValue(yamlPhase.reststimulus);
    restStimulusParam->updateCtrlWidget();
    restTimeParam->setValue(yamlPhase.resttime);
    restTimeParam->updateCtrlWidget();
    repNumParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.repnumctrl));
    repNumParam->onCtrlWidgetActivated(repNumParam->getCtrlWidget()->currentIndex());
    itemNumParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.itemnumctrl));
    itemNumParam->onCtrlWidgetActivated(itemNumParam->getCtrlWidget()->currentIndex());
    restStimulusParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.reststimulusctrl));
    restStimulusParam->onCtrlWidgetActivated(restStimulusParam->getCtrlWidget()->currentIndex());
    restTimeParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.resttimectrl));
    restTimeParam->onCtrlWidgetActivated(restTimeParam->getCtrlWidget()->currentIndex());

    this->onAcceptPropertyDialog();
}

void ProtocolDropXRepSeqScaledItem::setPhaseFromYaml(const YAML::RepSeqWithSteps_t &yamlPhase) {
    repNumParam->setValue(yamlPhase.repnum);
    repNumParam->updateCtrlWidget();
    itemNumParam->setValue(yamlPhase.itemnum);
    itemNumParam->updateCtrlWidget();
    restStimulusParam->setValue(yamlPhase.reststimulus);
    restStimulusParam->updateCtrlWidget();
    restTimeParam->setValue(yamlPhase.resttime);
    restTimeParam->updateCtrlWidget();
    repNumParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.repnumctrl));
    repNumParam->onCtrlWidgetActivated(repNumParam->getCtrlWidget()->currentIndex());
    itemNumParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.itemnumctrl));
    itemNumParam->onCtrlWidgetActivated(itemNumParam->getCtrlWidget()->currentIndex());
    restStimulusParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.reststimulusctrl));
    restStimulusParam->onCtrlWidgetActivated(restStimulusParam->getCtrlWidget()->currentIndex());
    restTimeParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.resttimectrl));
    restTimeParam->onCtrlWidgetActivated(restTimeParam->getCtrlWidget()->currentIndex());

    this->onAcceptPropertyDialog();
}

void ProtocolDropXRepSeqScaledItem::setPhaseFromYaml(const YAML::InfRepSeq_t &yamlPhase) {
    itemNumParam->setValue(yamlPhase.itemnum);
    itemNumParam->updateCtrlWidget();
    restStimulusParam->setValue(yamlPhase.reststimulus);
    restStimulusParam->updateCtrlWidget();
    restTimeParam->setValue(yamlPhase.resttime);
    restTimeParam->updateCtrlWidget();
    itemNumParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.itemnumctrl));
    itemNumParam->onCtrlWidgetActivated(itemNumParam->getCtrlWidget()->currentIndex());
    restStimulusParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.reststimulusctrl));
    restStimulusParam->onCtrlWidgetActivated(restStimulusParam->getCtrlWidget()->currentIndex());
    restTimeParam->getCtrlWidget()->setCurrentText(QString::fromStdString(yamlPhase.resttimectrl));
    restTimeParam->onCtrlWidgetActivated(restTimeParam->getCtrlWidget()->currentIndex());

    this->onAcceptPropertyDialog();
}

void ProtocolDropXRepSeqScaledItem::onSetString() {
    this->setText(QString(stimulusAbbrName + " hold leak: %1, scale: %2, rep: %3, \nitems: %4, " + stimulusAbbrName + " rest: %5, t rest: %6")
                  .arg(holdLeakParam->getValue()).arg(scaleFactorParam->getValue()).arg(repNumParam->getValue())
                  .arg(itemNumParam->getValue()).arg(restStimulusParam->getValue()).arg(restTimeParam->getValue()));
}

void ProtocolDropXRepSeqScaledItem::onAcceptPropertyDialog() {
    holdLeakParam->acceptEditWidget();
    scaleFactorParam->acceptEditWidget();
    repNumParam->acceptEditWidget();
    itemNumParam->acceptEditWidget();
    restStimulusParam->acceptEditWidget();
    restTimeParam->acceptEditWidget();
    preceding = precedingEdit->isChecked();
    reversed = reversedEdit->isChecked();
    alternating = alternatingEdit->isChecked();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXRepSeqScaledItem::onRejectPropertyDialog() {
    holdLeakParam->rejectEditWidget();
    scaleFactorParam->rejectEditWidget();
    repNumParam->rejectEditWidget();
    itemNumParam->rejectEditWidget();
    restStimulusParam->rejectEditWidget();
    restTimeParam->rejectEditWidget();
    precedingEdit->setChecked(preceding);
    reversedEdit->setChecked(reversed);
    alternatingEdit->setChecked(alternating);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXRepSeqScaledItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropVRepSeqScaledItem::ProtocolDropVRepSeqScaledItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRepSeqScaledItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIRepSeqScaledItem::ProtocolDropIRepSeqScaledItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRepSeqScaledItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    holdLeakParam->setValue(hold-1.0);
    this->onSetString();
}

ProtocolDropXRepSeqItem::ProtocolDropXRepSeqItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXRepSeqScaledItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Repeat sequence property panel");

    descriptionName->setText("Repeat preceding items for 'Repetitions' times");
    descriptionName->setVisible(true);

    holdLeakParam->setValue(hold);
    holdLeakParam->setVisible(false);

    scaleFactorParam->setValue(1);
    scaleFactorParam->setVisible(false);

    preceding = false;
    precedingEdit->setVisible(false);
    precedingEdit->setEnabled(false);

    reversed = false;
    reversedEdit->setVisible(false);
    reversedEdit->setEnabled(false);

    alternating = false;
    alternatingEdit->setVisible(false);
    alternatingEdit->setEnabled(false);

    applySteps = false;

    this->onSetString();
}

int ProtocolDropXRepSeqItem::getRepsNum() {
    return repNumParam->getValue()-1;
}

void ProtocolDropXRepSeqItem::onSetString() {
    this->setText(QString("rep: %1, items: %2, " + stimulusAbbrName + " rest: %3, t rest: %4")
                  .arg(repNumParam->getValue()).arg(itemNumParam->getValue())
                  .arg(restStimulusParam->getValue()).arg(restTimeParam->getValue()));
}

void ProtocolDropXRepSeqItem::onAcceptPropertyDialog() {
    repNumParam->acceptEditWidget();
    itemNumParam->acceptEditWidget();
    restStimulusParam->acceptEditWidget();
    restTimeParam->acceptEditWidget();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXRepSeqItem::onRejectPropertyDialog() {
    repNumParam->rejectEditWidget();
    itemNumParam->rejectEditWidget();
    restStimulusParam->rejectEditWidget();
    restTimeParam->rejectEditWidget();

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXRepSeqItem::onUpdateHold(double value) {
    hold = value;
    holdLeakParam->setValue(hold);
}

ProtocolDropVRepSeqItem::ProtocolDropVRepSeqItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRepSeqItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIRepSeqItem::ProtocolDropIRepSeqItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRepSeqItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDropXRepSeqWithStepsItem::ProtocolDropXRepSeqWithStepsItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXRepSeqScaledItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/repeat with steps.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Repeat sequence with steps property panel");

    descriptionName->setText("Repeat preceding items for 'Repetitions' times. Each repetition increases stepped parameters");
    descriptionName->setVisible(true);

    holdLeakParam->setValue(hold);
    holdLeakParam->setVisible(false);

    scaleFactorParam->setValue(1);
    scaleFactorParam->setVisible(false);

    preceding = false;
    precedingEdit->setVisible(false);
    precedingEdit->setEnabled(false);

    reversed = false;
    reversedEdit->setVisible(false);
    reversedEdit->setEnabled(false);

    alternating = false;
    alternatingEdit->setVisible(false);
    alternatingEdit->setEnabled(false);

    applySteps = true;

    this->onSetString();
}

int ProtocolDropXRepSeqWithStepsItem::getRepsNum() {
    return repNumParam->getValue()-1;
}

void ProtocolDropXRepSeqWithStepsItem::onSetString() {
    this->setText(QString("rep: %1, items: %2, " + stimulusAbbrName + " rest: %3, t rest: %4")
                  .arg(repNumParam->getValue()).arg(itemNumParam->getValue())
                  .arg(restStimulusParam->getValue()).arg(restTimeParam->getValue()));
}

void ProtocolDropXRepSeqWithStepsItem::onAcceptPropertyDialog() {
    repNumParam->acceptEditWidget();
    itemNumParam->acceptEditWidget();
    restStimulusParam->acceptEditWidget();
    restTimeParam->acceptEditWidget();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXRepSeqWithStepsItem::onRejectPropertyDialog() {
    repNumParam->rejectEditWidget();
    itemNumParam->rejectEditWidget();
    restStimulusParam->rejectEditWidget();
    restTimeParam->rejectEditWidget();

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXRepSeqWithStepsItem::onUpdateHold(double value) {
    hold = value;
    holdLeakParam->setValue(hold);
}

ProtocolDropVRepSeqWithStepsItem::ProtocolDropVRepSeqWithStepsItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRepSeqWithStepsItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIRepSeqWithStepsItem::ProtocolDropIRepSeqWithStepsItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXRepSeqWithStepsItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDropXInfRepSeqItem::ProtocolDropXInfRepSeqItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropXRepSeqScaledItem(mDev, ctrlManager, hold0, clampingModality, type) {

    QString iconString = ":imgs/infinite repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Infinite repeat sequence property panel");

    descriptionName->setText("Repeat preceding items undefinetely");
    descriptionName->setVisible(true);

    holdLeakParam->setValue(hold);
    holdLeakParam->setVisible(false);

    repNumParam->setValue(-1);
    repNumParam->setVisible(false);

    scaleFactorParam->setValue(1);
    scaleFactorParam->setVisible(false);

    preceding = false;
    precedingEdit->setVisible(false);
    precedingEdit->setEnabled(false);

    reversed = false;
    reversedEdit->setVisible(false);
    reversedEdit->setEnabled(false);

    alternating = false;
    alternatingEdit->setVisible(false);
    alternatingEdit->setEnabled(false);

    applySteps = false;

    this->onSetString();
}

int ProtocolDropXInfRepSeqItem::getRepsNum() {
    return -1;
}

void ProtocolDropXInfRepSeqItem::onSetString() {
    this->setText(QString("items: %1, " + stimulusAbbrName + " rest: %2, t rest: %3")
                  .arg(itemNumParam->getValue()).arg(restStimulusParam->getValue()).arg(restTimeParam->getValue()));
}

void ProtocolDropXInfRepSeqItem::onAcceptPropertyDialog() {
    itemNumParam->acceptEditWidget();
    restStimulusParam->acceptEditWidget();
    restTimeParam->acceptEditWidget();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropXInfRepSeqItem::onRejectPropertyDialog() {
    itemNumParam->rejectEditWidget();
    restStimulusParam->rejectEditWidget();
    restTimeParam->rejectEditWidget();

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropXInfRepSeqItem::onUpdateHold(double value) {
    hold = value;
    holdLeakParam->setValue(hold);
}

ProtocolDropVInfRepSeqItem::ProtocolDropVInfRepSeqItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXInfRepSeqItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDropIInfRepSeqItem::ProtocolDropIInfRepSeqItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropXInfRepSeqItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDropControlItem::ProtocolDropControlItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropItem(mDev, ctrlManager, hold0, clampingModality, type) {

    visible = false;
    visibleEdit->setVisible(false);
    visibleEdit->setEnabled(false);
    this->setBackground(PROT_EDITOR_CONTROLS_ITEM_COLOR);

    propertyLo->setColumnStretch(0, 2);
    propertyLo->setColumnStretch(1, 2);
    propertyLo->setColumnStretch(2, 2);
    propertyLo->setColumnStretch(3, 3);
    propertyLo->setColumnStretch(4, 1);
}

void ProtocolDropControlItem::openPropertyDialog() {
    propertyDialog->exec();
}

QString ProtocolDropControlItem::getName() {
    return name;
}

int ProtocolDropControlItem::getIntValue() {
    return intValue;
}

double ProtocolDropControlItem::getDoubleValue() {
    return doubleValue;
}

QSpinBox * ProtocolDropControlItem::getIntEdit() {
    return intEdit;
}

QDoubleSpinBox * ProtocolDropControlItem::getDoubleEdit() {
    return doubleEdit;
}

QString ProtocolDropControlItem::getUnit() {
    return valueUnit->text();
}

YAML::Control_t ProtocolDropControlItem::getYamlVoltageCtrl() {
    YAML::VoltageCtrl_t yamlCtrl;
    yamlCtrl.name = name.toStdString();
    yamlCtrl.value = doubleEdit->value();
    return yamlCtrl;
}

YAML::Control_t ProtocolDropControlItem::getYamlCurrentCtrl() {
    YAML::CurrentCtrl_t yamlCtrl;
    yamlCtrl.name = name.toStdString();
    yamlCtrl.value = doubleEdit->value();
    return yamlCtrl;
}

YAML::Control_t ProtocolDropControlItem::getYamlTimeCtrl() {
    YAML::TimeCtrl_t yamlCtrl;
    yamlCtrl.name = name.toStdString();
    yamlCtrl.value = doubleEdit->value();
    return yamlCtrl;
}

YAML::Control_t ProtocolDropControlItem::getYamlFrequencyCtrl() {
    YAML::FrequencyCtrl_t yamlCtrl;
    yamlCtrl.name = name.toStdString();
    yamlCtrl.value = doubleEdit->value();
    return yamlCtrl;
}

YAML::Control_t ProtocolDropControlItem::getYamlNaturalNumCtrl() {
    YAML::NaturalNumCtrl_t yamlCtrl;
    yamlCtrl.name = name.toStdString();
    yamlCtrl.value = intEdit->value();
    return yamlCtrl;
}

void ProtocolDropControlItem::setCtrlFromYaml(const YAML::VoltageCtrl &yamlCtrl) {
    name = QString::fromStdString(yamlCtrl.name);
    nameEdit->setText(name);
    doubleValue = yamlCtrl.value;
    doubleEdit->setValue(doubleValue);

    this->onAcceptPropertyDialog();
}

void ProtocolDropControlItem::setCtrlFromYaml(const YAML::CurrentCtrl &yamlCtrl) {
    name = QString::fromStdString(yamlCtrl.name);
    nameEdit->setText(name);
    doubleValue = yamlCtrl.value;
    doubleEdit->setValue(doubleValue);

    this->onAcceptPropertyDialog();
}

void ProtocolDropControlItem::setCtrlFromYaml(const YAML::TimeCtrl &yamlCtrl) {
    name = QString::fromStdString(yamlCtrl.name);
    nameEdit->setText(name);
    doubleValue = yamlCtrl.value;
    doubleEdit->setValue(doubleValue);

    this->onAcceptPropertyDialog();
}

void ProtocolDropControlItem::setCtrlFromYaml(const YAML::FrequencyCtrl &yamlCtrl) {
    name = QString::fromStdString(yamlCtrl.name);
    nameEdit->setText(name);
    doubleValue = yamlCtrl.value;
    doubleEdit->setValue(doubleValue);

    this->onAcceptPropertyDialog();
}

void ProtocolDropControlItem::setCtrlFromYaml(const YAML::NaturalNumCtrl &yamlCtrl) {
    name = QString::fromStdString(yamlCtrl.name);
    nameEdit->setText(name);
    intValue = yamlCtrl.value;
    intEdit->setValue(intValue);

    this->onAcceptPropertyDialog();
}

ProtocolDropVoltageControlItem::ProtocolDropVoltageControlItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropControlItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/voltage control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Voltage control property panel");

    protocolItemCtrlType = ProtocolItemCtrlVoltage;

    descriptionName->setText("Adds a control for voltage values in main protocol view");
    descriptionName->setVisible(true);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    name = QString("voltage ctrl%1").arg(voltageControlItemIdx++);
    nameEdit = new QLineEdit(name);
    doubleValue = 100.0;
    doubleEdit = new QDoubleSpinBox();
    initQdoubleSpinBox(doubleEdit, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    doubleEdit->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    doubleEdit->setValue(doubleValue);
    valueUnit = new QLabel(QString::fromStdString(stimulusRange.getFullUnit()));

    connect(doubleEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), this, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged));

    propertyLo->addWidget(new QLabel("Name"), rowIdx, 0, Qt::AlignRight);
    propertyLo->addWidget(nameEdit, rowIdx, 1, 1, 2);
    propertyLo->addWidget(doubleEdit, rowIdx, 3);
    propertyLo->addWidget(valueUnit, rowIdx++, 4);

    editWidgets.push_back(doubleEdit);

    this->onSetString();
}

void ProtocolDropVoltageControlItem::setStimulusRange(RangedMeasurement_t &range) {
    stimulusRange = range;

    initQdoubleSpinBox(doubleEdit, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    doubleEdit->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    valueUnit->setText(QString::fromStdString(stimulusRange.getFullUnit()));
    this->onAcceptPropertyDialog();
}

void ProtocolDropVoltageControlItem::onSetString() {
    this->setText(QString("%1: %2").arg(name).arg(doubleValue));
}

void ProtocolDropVoltageControlItem::onAcceptPropertyDialog() {
    name = nameEdit->text();
    doubleValue = doubleEdit->value();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropVoltageControlItem::onRejectPropertyDialog() {
    nameEdit->setText(name);
    doubleEdit->setValue(doubleValue);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropVoltageControlItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropCurrentControlItem::ProtocolDropCurrentControlItem(ModelDevice * mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropControlItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    QString iconString = ":imgs/current control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Current control property panel");

    protocolItemCtrlType = ProtocolItemCtrlCurrent;

    descriptionName->setText("Adds a control for current values in main protocol view");
    descriptionName->setVisible(true);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    name = QString("current ctrl%1").arg(currentControlItemIdx++);
    nameEdit = new QLineEdit(name);
    doubleValue = 1.0;
    doubleEdit = new QDoubleSpinBox();
    initQdoubleSpinBox(doubleEdit, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    doubleEdit->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    doubleEdit->setValue(doubleValue);
    valueUnit = new QLabel(QString::fromStdString(stimulusRange.getFullUnit()));

    connect(doubleEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), this, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged));

    propertyLo->addWidget(new QLabel("Name"), rowIdx, 0, Qt::AlignRight);
    propertyLo->addWidget(nameEdit, rowIdx, 1, 1, 2);
    propertyLo->addWidget(doubleEdit, rowIdx, 3);
    propertyLo->addWidget(valueUnit, rowIdx++, 4);

    editWidgets.push_back(doubleEdit);

    this->onSetString();
}

void ProtocolDropCurrentControlItem::setStimulusRange(RangedMeasurement_t &range) {
    stimulusRange = range;
    initQdoubleSpinBox(doubleEdit, stimulusRange, RangedQDoubleSpinBox_t::DELTA);
//    doubleEdit->setRangedMeasurement(stimulusRange, QDoubleSpinBox::DeltaRange);
    valueUnit->setText(QString::fromStdString(stimulusRange.getFullUnit()));
    this->onAcceptPropertyDialog();
}

void ProtocolDropCurrentControlItem::onSetString() {
    this->setText(QString("%1: %2").arg(name).arg(doubleValue));
}

void ProtocolDropCurrentControlItem::onAcceptPropertyDialog() {
    name = nameEdit->text();
    doubleValue = doubleEdit->value();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropCurrentControlItem::onRejectPropertyDialog() {
    nameEdit->setText(name);
    doubleEdit->setValue(doubleValue);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropCurrentControlItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropTimeControlItem::ProtocolDropTimeControlItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropControlItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/time control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Time control property panel");

    protocolItemCtrlType = ProtocolItemCtrlTime;

    descriptionName->setText("Adds a control for time values in main protocol view");
    descriptionName->setVisible(true);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    name = QString("time ctrl%1").arg(timeControlItemIdx++);
    nameEdit = new QLineEdit(name);
    doubleValue = 100.0;
    doubleEdit = new QDoubleSpinBox();
    initQdoubleSpinBox(doubleEdit, timeRange, RangedQDoubleSpinBox_t::DELTA);
//    doubleEdit->setRangedMeasurement(timeRange, QDoubleSpinBox::DeltaRange);
    doubleEdit->setValue(doubleValue);
    valueUnit = new QLabel(QString::fromStdString(timeRange.getFullUnit()));

    connect(doubleEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), this, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged));

    propertyLo->addWidget(new QLabel("Name"), rowIdx, 0, Qt::AlignRight);
    propertyLo->addWidget(nameEdit, rowIdx, 1, 1, 2);
    propertyLo->addWidget(doubleEdit, rowIdx, 3);
    propertyLo->addWidget(valueUnit, rowIdx++, 4);

    editWidgets.push_back(doubleEdit);

    this->onSetString();
}

void ProtocolDropTimeControlItem::setStimulusRange(RangedMeasurement_t &) {
    /*! nothing to do */
}

void ProtocolDropTimeControlItem::onSetString() {
    this->setText(QString("%1: %2").arg(name).arg(doubleValue));
}

void ProtocolDropTimeControlItem::onAcceptPropertyDialog() {
    name = nameEdit->text();
    doubleValue = doubleEdit->value();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropTimeControlItem::onRejectPropertyDialog() {
    nameEdit->setText(name);
    doubleEdit->setValue(doubleValue);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropTimeControlItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropFrequencyControlItem::ProtocolDropFrequencyControlItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropControlItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/frequency control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Frequency control property panel");

    protocolItemCtrlType = ProtocolItemCtrlFrequency;

    descriptionName->setText("Adds a control for frequency values in main protocol view");
    descriptionName->setVisible(true);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    name = QString("frequency ctrl%1").arg(frequencyControlItemIdx++);
    nameEdit = new QLineEdit(name);
    doubleValue = 10.0;
    doubleEdit = new QDoubleSpinBox();
    initQdoubleSpinBox(doubleEdit, frequencyRange, RangedQDoubleSpinBox_t::MIN_MAX);
//    doubleEdit->setRangedMeasurement(frequencyRange, QDoubleSpinBox::MinMaxRange);
    doubleEdit->setValue(doubleValue);
    valueUnit = new QLabel(QString::fromStdString(frequencyRange.getFullUnit()));

    connect(doubleEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), this, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged));

    propertyLo->addWidget(new QLabel("Name"), rowIdx, 0, Qt::AlignRight);
    propertyLo->addWidget(nameEdit, rowIdx, 1, 1, 2);
    propertyLo->addWidget(doubleEdit, rowIdx, 3);
    propertyLo->addWidget(valueUnit, rowIdx++, 4);

    editWidgets.push_back(doubleEdit);

    this->onSetString();
}

void ProtocolDropFrequencyControlItem::setStimulusRange(RangedMeasurement_t &) {
    /*! nothing to do */
}

void ProtocolDropFrequencyControlItem::onSetString() {
    this->setText(QString("%1: %2").arg(name).arg(doubleValue));
}

void ProtocolDropFrequencyControlItem::onAcceptPropertyDialog() {
    name = nameEdit->text();
    doubleValue = doubleEdit->value();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropFrequencyControlItem::onRejectPropertyDialog() {
    nameEdit->setText(name);
    doubleEdit->setValue(doubleValue);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropFrequencyControlItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropNaturalNumControlItem::ProtocolDropNaturalNumControlItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropControlItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/number control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("");

    propertyDialog->setWindowTitle("Natural number control property panel");

    protocolItemCtrlType = ProtocolItemCtrlNaturalNum;

    descriptionName->setText("Adds a control for natural numbers in main protocol view");
    descriptionName->setVisible(true);

    int rowIdx = PDI_FIRST_PARAM_ROW;

    name = QString("num ctrl%1").arg(naturalNumControlItemIdx++);
    nameEdit = new QLineEdit(name);
    intValue = 1;
    intEdit = new QSpinBox();
    intEdit->setRange(1, 100);
    intEdit->setValue(intValue);
    valueUnit = new QLabel("");

    connect(intEdit, QOverload <int> ::of(&QSpinBox::valueChanged), this, QOverload <int> ::of(&ProtocolDropControlItem::valueChanged));

    propertyLo->addWidget(new QLabel("Name"), rowIdx, 0, Qt::AlignRight);
    propertyLo->addWidget(nameEdit, rowIdx, 1, 1, 2);
    propertyLo->addWidget(intEdit, rowIdx, 3);
    propertyLo->addWidget(valueUnit, rowIdx++, 4);

    this->onSetString();
}

void ProtocolDropNaturalNumControlItem::setStimulusRange(RangedMeasurement_t &) {
    /*! nothing to do */
}

void ProtocolDropNaturalNumControlItem::onSetString() {
    this->setText(QString("%1: %2").arg(name).arg(intValue));
}

void ProtocolDropNaturalNumControlItem::onAcceptPropertyDialog() {
    name = nameEdit->text();
    intValue = intEdit->value();

    ProtocolDropItem::onAcceptPropertyDialog();
}

void ProtocolDropNaturalNumControlItem::onRejectPropertyDialog() {
    nameEdit->setText(name);
    intEdit->setValue(intValue);

    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropNaturalNumControlItem::onUpdateHold(double value) {
    hold = value;
}

ProtocolDropAnalysisItem::ProtocolDropAnalysisItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type) :
    ProtocolDropItem(mDev, ctrlManager, hold0, clampingModality, type) {

    visible = false;
    visibleEdit->setVisible(false);
    visibleEdit->setEnabled(false);
    this->setBackground(PROT_EDITOR_ANALYSIS_ITEM_COLOR);

    /*! \todo FCON qui forse conviene fare un metodo che setta una stringa d'errore e modifica il widget in modo da farlo sembrare non valido */
    this->setText("Cursors not set");

    propertyLo->setColumnStretch(0, 2);
    propertyLo->setColumnStretch(1, 2);
    propertyLo->setColumnStretch(2, 2);
    propertyLo->setColumnStretch(3, 2);
    propertyLo->setColumnStretch(4, 2);
}

ProtocolDropAnalysisItem::~ProtocolDropAnalysisItem() {
    for (int idx = 0; idx < timePointParams.size(); idx++) {
        ProtocolDropAnalysisItemParam * daip = timePointParams[idx];
        if (daip != nullptr) {
            delete daip;
            daip = nullptr;
        }
    }
    timePointParams.clear();

    if (analysisCursors != nullptr) {
        analysisCursors->clear();
        delete analysisCursors;
        analysisCursors = nullptr;
    }
}

void ProtocolDropAnalysisItem::openPropertyDialog() {
    propertyDialog->exec();
}

QString ProtocolDropAnalysisItem::getName() {
    return "";
}

void ProtocolDropAnalysisItem::setStimulusRange(RangedMeasurement_t &) {
    /*! nothing to do */
}

void ProtocolDropAnalysisItem::addCursors(QVector <ProtocolCursor *> * cursors) {
    protocolCursors = cursors;
    for (int timePointIdx = 0; timePointIdx < timePointsRequired; timePointIdx++) {
        timePointParams[timePointIdx]->addCursors(cursors->size());
    }
}

void ProtocolDropAnalysisItem::removeCursors(QVector <ProtocolCursor *> * cursors, QVector <int> cursorsMap) {
    protocolCursors = cursors;
    for (int timePointIdx = 0; timePointIdx < timePointsRequired; timePointIdx++) {
        timePointParams[timePointIdx]->removeCursors(cursorsMap);
    }

    this->onAcceptPropertyDialog();
}

QVector <int> ProtocolDropAnalysisItem::getCursorMapping() {
    QVector <int> cursorMap;
    cursorMap.resize(timePointsRequired);
    for (int timePointIdx = 0; timePointIdx < timePointsRequired; timePointIdx++) {
        cursorMap[timePointIdx] = timePointParams[timePointIdx]->getCursorIdx()-1;
    }
    return cursorMap;
}

YAML::Analysis_t ProtocolDropAnalysisItem::getYamlAnalysis() {
    YAML::Analysis_t yamlAnalysis;

    yamlAnalysis.type = analysisType;
    for (int timePointIdx = 0; timePointIdx < timePointsRequired; timePointIdx++) {
        yamlAnalysis.timepoints.push_back(timePointParams[timePointIdx]->getCursorIdx());
    }

    return yamlAnalysis;
}

void ProtocolDropAnalysisItem::setAnalysisFromYaml(const YAML::Analysis_t &yamlAnalysis) {
    this->updateTimePointsNum(yamlAnalysis.timepoints.size());

    int intValue;
    for (int timePointIdx = 0; timePointIdx < timePointsRequired; timePointIdx++) {
        intValue = qMin(yamlAnalysis.timepoints[timePointIdx], timePointParams[timePointIdx]->getTimePointWidget()->count()-1);
        timePointParams[timePointIdx]->getTimePointWidget()->setCurrentIndex(intValue);
    }

    this->onAcceptPropertyDialog();
}

void ProtocolDropAnalysisItem::onAcceptPropertyDialog() {
    this->onSetString();

    if (propertyDialog->isVisible()) {
        propertyDialog->accept();
    }
    emit analysisChanged();
}

void ProtocolDropAnalysisItem::setEnabled(bool enabled) {
    analysisEnabled = enabled;
    this->onAcceptPropertyDialog();
}

QString ProtocolDropAnalysisItem::onCheckCursorsValidity() {
    if (!analysisEnabled) {
        this->setInvalidProtocolToolTip();
        return "Invalid protocol";
    }

    if (!(this->allTimePointsSet())) {
        this->setMissingCursorsToolTip();
        return "Cursors not set";
    }

    for (int idx = 0; idx < timePointsRequired; idx++) {
        /*! getCursorIdx has an offset of 1, cause 0 means no cursor set.
            For this reason we need to subtract 1 and always check that all time points are set
            before moving on */
        analysisCursors->replace(idx, protocolCursors->at(timePointParams[idx]->getCursorIdx()-1));
    }

    return "OK";
}

void ProtocolDropAnalysisItem::initializeWidgets() {
    int rowIdx = PDI_FIRST_PARAM_ROW;

    for (int timePointIdx = 0; timePointIdx < timePointsRequired; timePointIdx++) {
        timePointParams.append(new ProtocolDropAnalysisItemParam(QString("Time point %1").arg(timePointIdx+1)));

        connect(timePointParams.back(), &ProtocolDropAnalysisItemParam::setWidgetString, this, &ProtocolDropItem::onSetString);

        propertyLo->addWidget(timePointParams.back()->getNameWidget(), rowIdx, 0, 1, 2, Qt::AlignRight);
        propertyLo->addWidget(timePointParams.back()->getTimePointWidget(), rowIdx++, 2, 1, 3);
    }

    analysisCursors = new QVector <ProtocolCursor *> (timePointsRequired);
}

void ProtocolDropAnalysisItem::updateTimePointsNum(int) {
    /*! Does nothing by default, overload where needed */
}

bool ProtocolDropAnalysisItem::allTimePointsSet() {
    if (protocolCursors->size() < timePointsRequired) {
        return false;
    }

    for (int timePointIdx = 0; timePointIdx < timePointsRequired; timePointIdx++) {
        if (timePointParams[timePointIdx]->getCursorIdx() == 0) {
            return false;
        }
    }
    return true;
}

void ProtocolDropAnalysisItem::setValidToolTip() {
    this->setToolTip("");
}

void ProtocolDropAnalysisItem::setInvalidProtocolToolTip() {
    this->setToolTip("A valid protocol is mandatory to use analysis");
}

void ProtocolDropAnalysisItem::setMissingCursorsToolTip() {
    this->setToolTip("One or more of the required cursors\n"
                     "haven't been set for this analysis");
}

void ProtocolDropAnalysisItem::setInvalidPrecedenceToolTip() {
    this->setToolTip("This analysis can execute correctly\n"
                     "only if cursors are in a given order.\n"
                     "Note that wrong cursors order may be due\n"
                     "to cursors repetition or sweep properties");
}

void ProtocolDropAnalysisItem::setInvalidLoopsToolTip() {
    this->setToolTip("This analysis can execute correctly\n"
                     "only if some cursors are activated\n"
                     "within the same loops. This means, for\n"
                     "example, the if one cursor is active\n"
                     "on all repetitions, a coupled cursor\n"
                     "can't be active only on 1 repetition.");
}

ProtocolDropNoiseReportItem::ProtocolDropNoiseReportItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    analysisType = YAML::NoiseReport;
    QString iconString = ":imgs/analysis noise report.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Noise report property panel");

    descriptionName->setText("Allows estimation of the signals mean values and\n"
                             "standard deviations while applying a constant\n"
                             "holding stimulus H0.\n"
                             "\nTime points 1 and 2 must enclose an interval\n"
                             "in which H0 is applied.");
    descriptionName->setVisible(true);

    timePointsRequired = 2;

    protocolConsumerType = ProtocolConsumerNoiseReport;

    this->initializeWidgets();
}

void ProtocolDropNoiseReportItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("H0: cursors %1-%2")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropNoiseReportItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropNoiseReportItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropNoiseReportItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropNoiseReportItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}

ProtocolDropHistogramItem::ProtocolDropHistogramItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    analysisType = YAML::Histogram;
    QString iconString = ":imgs/analysis histogram.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Histogram property panel");

    descriptionName->setText("Allows estimation of the histogram of a signal\n"
                             "in a given interval I0.\n"
                             "\nTime points 1 and 2 must enclose the interval\n"
                             "I0.");
    descriptionName->setVisible(true);

    timePointsRequired = 2;

    protocolConsumerType = ProtocolConsumerHistogram;

    this->initializeWidgets();
}

void ProtocolDropHistogramItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("I0: cursors %1-%2")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropHistogramItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropHistogramItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropHistogramItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropHistogramItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}

ProtocolDropSpectrumItem::ProtocolDropSpectrumItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    analysisType = YAML::Spectrum;
    QString iconString = ":imgs/analysis spectrum.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Spectrum property panel");

    descriptionName->setText("Allows estimation of the spectrum of a signal\n"
                             "in a given interval I0.\n"
                             "\nTime points 1 and 2 must enclose the interval\n"
                             "I0.");
    descriptionName->setVisible(true);

    timePointsRequired = 2;

    protocolConsumerType = ProtocolConsumerSpectrum;

    this->initializeWidgets();
}

void ProtocolDropSpectrumItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("I0: cursors %1-%2")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropSpectrumItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropSpectrumItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropSpectrumItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropSpectrumItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}

ProtocolDropResistanceEstimationItem::ProtocolDropResistanceEstimationItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    analysisType = YAML::ResistanceEstimation;
    QString iconString = ":imgs/analysis resistance estimation.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Resistance estimation property panel");

    descriptionName->setText("Allows estimation total DUT resistance.\n"
                             "Estimations are based on current response\n"
                             "following a voltage step from V0 to V1.\n"
                             "\nTime points 1 and 2 must enclose an interval\n"
                             "in which V0 is reasonably at regime value.\n"
                             "\nTime points 3 and 4 must enclose the interval\n"
                             "in which V1 is reasonably at regime value.");
    descriptionName->setVisible(true);

    timePointsRequired = 4;

    protocolConsumerType = ProtocolConsumerResistanceEstimation;

    this->initializeWidgets();
}

void ProtocolDropResistanceEstimationItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("V0: cursors %1-%2\n"
                              "V1: cursors %3-%4")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx())
                .arg(timePointParams[2]->getCursorIdx())
                .arg(timePointParams[3]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropResistanceEstimationItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropResistanceEstimationItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropResistanceEstimationItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropResistanceEstimationItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(1)->precedes(analysisCursors->at(2)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[1]->getCursorIdx()).arg(timePointParams[2]->getCursorIdx());
    }

    if (!(analysisCursors->at(2)->precedes(analysisCursors->at(3)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[2]->getCursorIdx()).arg(timePointParams[3]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(1)->sameRepetitions(analysisCursors->at(2)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[1]->getCursorIdx()).arg(timePointParams[2]->getCursorIdx());
    }

    if (!(analysisCursors->at(2)->sameRepetitions(analysisCursors->at(3)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[2]->getCursorIdx()).arg(timePointParams[3]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}

ProtocolDropMembraneTestItem::ProtocolDropMembraneTestItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    analysisType = YAML::MembraneTest;
    QString iconString = ":imgs/analysis membrane test.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Membrane test property panel");

    descriptionName->setText("Allows estimation of seal resistance, pipette\n"
                             "capacitance, access resistance, membrane\n"
                             "capacitance and membrane resistance.\n"
                             "Estimations are based on current response\n"
                             "following a voltage step from V0 to V1.\n"
                             "\nTime points 1 and 2 must enclose an interval\n"
                             "in which V0 is reasonably at regime value.\n"
                             "\nTime points 3 and 4 must enclose the interval\n"
                             "where V1 is applied, starting from the step,\n"
                             "so that the current peak is completely included.");
    descriptionName->setVisible(true);

    timePointsRequired = 4;

    protocolConsumerType = ProtocolConsumerMembraneTest;

    this->initializeWidgets();
}

void ProtocolDropMembraneTestItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("V0: cursors %1-%2\n"
                              "V1: cursors %3-%4")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx())
                .arg(timePointParams[2]->getCursorIdx())
                .arg(timePointParams[3]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropMembraneTestItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropMembraneTestItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropMembraneTestItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropMembraneTestItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(1)->precedes(analysisCursors->at(2)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[1]->getCursorIdx()).arg(timePointParams[2]->getCursorIdx());
    }

    if (!(analysisCursors->at(2)->precedes(analysisCursors->at(3)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[2]->getCursorIdx()).arg(timePointParams[3]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(1)->sameRepetitions(analysisCursors->at(2)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[1]->getCursorIdx()).arg(timePointParams[2]->getCursorIdx());
    }

    if (!(analysisCursors->at(2)->sameRepetitions(analysisCursors->at(3)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[2]->getCursorIdx()).arg(timePointParams[3]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}

ProtocolDropIvGraphItem::ProtocolDropIvGraphItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::VOLTAGE_CLAMP, type) {

    analysisType = YAML::IVGraph;
    QString iconString = ":imgs/analysis iv graph.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("I/V graph property panel");

    descriptionName->setText("Plots an I/V graph of the current and voltage\n"
                             "values sampled within specific intervals.\n"
                             "\nThe number of intervals can be modified, and\n"
                             "each pair of consecutive time point must enclose\n"
                             "one of the intervals in which I/V samples are collected:\n"
                             "\ne.g.Time points 1 and 2 must enclose an interval I0.");
    descriptionName->setVisible(true);

    timePointsRequired = 2; /*! \todo FCON vorrei rendere variabile il numero delle coppie di punti */

    protocolConsumerType = ProtocolConsumerIvGraph;

    this->initializeWidgets();
}

void ProtocolDropIvGraphItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        QString text = "";
        int intervalsNum = timePointsRequired/2;
        for (int intervalIdx = 0; intervalIdx < intervalsNum; intervalIdx++) {
            if (intervalIdx > 0) {
                text += "\n";
            }
            text += QString("I%1: cursors %2-%3")
                    .arg(intervalIdx)
                    .arg(timePointParams[intervalIdx*2]->getCursorIdx())
                    .arg(timePointParams[intervalIdx*2+1]->getCursorIdx());
        }
        this->setText(text);

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropIvGraphItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropIvGraphItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropIvGraphItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropIvGraphItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    int intervalsNum = timePointsRequired/2;
    for (int intervalIdx = 0; intervalIdx < intervalsNum; intervalIdx++) {
        if (intervalIdx > 0) {
            if (!(analysisCursors->at(intervalIdx*2-1)->precedes(analysisCursors->at(intervalIdx*2)))) {
                this->setInvalidPrecedenceToolTip();
                return QString("Cursor %1 must\nprecede cursor %2")
                        .arg(timePointParams[intervalIdx*2-1]->getCursorIdx()).arg(timePointParams[intervalIdx*2]->getCursorIdx());
            }

            if (!(analysisCursors->at(intervalIdx*2-1)->sameRepetitions(analysisCursors->at(intervalIdx*2)))) {
                this->setInvalidLoopsToolTip();
                return QString("Cursors %1 and %2\nmust execute the same loops")
                        .arg(timePointParams[intervalIdx*2-1]->getCursorIdx()).arg(timePointParams[intervalIdx*2]->getCursorIdx());
            }
        }

        if (!(analysisCursors->at(intervalIdx*2)->precedes(analysisCursors->at(intervalIdx*2+1)))) {
            this->setInvalidPrecedenceToolTip();
            return QString("Cursor %1 must\nprecede cursor %2")
                    .arg(timePointParams[intervalIdx*2]->getCursorIdx()).arg(timePointParams[intervalIdx*2+1]->getCursorIdx());
        }

        if (!(analysisCursors->at(intervalIdx*2)->sameRepetitions(analysisCursors->at(intervalIdx*2+1)))) {
            this->setInvalidLoopsToolTip();
            return QString("Cursors %1 and %2\nmust execute the same loops")
                    .arg(timePointParams[intervalIdx*2]->getCursorIdx()).arg(timePointParams[intervalIdx*2+1]->getCursorIdx());
        }
    }

    this->setValidToolTip();
    return "OK";
}

void ProtocolDropIvGraphItem::initializeWidgets() {
    int rowIdx = PDI_ANALYSIS_OPTION_ROW;

    QLabel * intervalsNumLbl = new QLabel("Intervals #");
    propertyLo->addWidget(intervalsNumLbl, rowIdx, 0, 1, 2, Qt::AlignRight);

    intervalsNumSb = new QSpinBox();
    intervalsNumSb->setMinimum(1);
    intervalsNumSb->setMaximum(PDI_MAX_PARAMS/2); /*!< Each interval requires 2 time points */
    propertyLo->addWidget(intervalsNumSb, rowIdx++, 2, 1, 3);
    connect(intervalsNumSb, QOverload <int> ::of(&QSpinBox::valueChanged), this, [=] (int intervalsNum) {
        if (timePointsRequired != intervalsNum*2) {
            this->updateTimePointsNum(intervalsNum*2);
        }
    });

    ProtocolDropAnalysisItem::initializeWidgets();
}

void ProtocolDropIvGraphItem::updateTimePointsNum(int timePointsNum) {
    int oldPointsRequired = timePointsRequired;
    timePointsRequired = timePointsNum;

    if (timePointsRequired > oldPointsRequired) {
        int rowIdx = PDI_FIRST_PARAM_ROW+oldPointsRequired;

        for (int timePointIdx = oldPointsRequired; timePointIdx < timePointsRequired; timePointIdx++) {
            timePointParams.append(new ProtocolDropAnalysisItemParam(QString("Time point %1").arg(timePointIdx+1)));

            connect(timePointParams.back(), &ProtocolDropAnalysisItemParam::setWidgetString, this, &ProtocolDropItem::onSetString);

            propertyLo->addWidget(timePointParams.back()->getNameWidget(), rowIdx, 0, 1, 2, Qt::AlignRight);
            propertyLo->addWidget(timePointParams.back()->getTimePointWidget(), rowIdx++, 2, 1, 3);
        }
        this->updateTimePointsNumHouseKeeping();

    } else if (timePointsRequired < oldPointsRequired) {
        QLayoutItem * item;
        for (int timePointIdx = timePointsRequired; timePointIdx < oldPointsRequired; timePointIdx++) {
            for (int colIdx = 0; colIdx < 3; colIdx += 2) {
                item = propertyLo->itemAtPosition(PDI_FIRST_PARAM_ROW+timePointIdx, colIdx);
                if (item != nullptr) {
                    propertyLo->removeItem(item);
                    delete item->widget();
                }
            }
        }
        this->updateTimePointsNumHouseKeeping();
    }
}

void ProtocolDropIvGraphItem::updateTimePointsNumHouseKeeping() {
    timePointParams.resize(timePointsRequired);
    analysisCursors->resize(timePointsRequired);
    if (protocolCursors != nullptr) {
        this->addCursors(protocolCursors);
    }
    intervalsNumSb->setValue(timePointsRequired/2);

    this->onSetString();
}

ProtocolDropVoltageTrackingItem::ProtocolDropVoltageTrackingItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

//    analysisType = YAML::Histogram;
    QString iconString = ":imgs/analysis voltage tracking.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Voltage tracking property panel");

    descriptionName->setText("Allows automatic correction of applied current\n"
                             "to keep a fixed potential.\n"
                             "\nTime points 1 and 2 must enclose the interval\n"
                             "I0 in which the potential is monitored.");
    descriptionName->setVisible(true);

    timePointsRequired = 2;

    protocolConsumerType = ProtocolConsumerVoltageTracking;

    this->initializeWidgets();
}

void ProtocolDropVoltageTrackingItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("I0: cursors %1-%2")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropVoltageTrackingItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropVoltageTrackingItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropVoltageTrackingItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropVoltageTrackingItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}

ProtocolDropApThresholdItem::ProtocolDropApThresholdItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    analysisType = YAML::APThreshold;
    QString iconString = ":imgs/analysis ap threshold.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Action potential threshold property panel");

    descriptionName->setText("Allows estimation of the voltage threshold for action\n"
                             "potentials generation and subthreshold current.\n"
                             "\nTime points 1 and 2 must enclose the interval\n"
                             "I0 in which the potential is monitored.");
    descriptionName->setVisible(true);

    timePointsRequired = 2;

    protocolConsumerType = ProtocolConsumerApThreshold;

    this->initializeWidgets();
}

void ProtocolDropApThresholdItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("I0: cursors %1-%2")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropApThresholdItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropApThresholdItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropApThresholdItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropApThresholdItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}

ProtocolDropApStatisticsItem::ProtocolDropApStatisticsItem(ModelDevice *  mDev, ProtocolItemCtrlManager * ctrlManager, double hold0, int type) :
    ProtocolDropAnalysisItem(mDev, ctrlManager, hold0, ClampingModality_t::CURRENT_CLAMP, type) {

    analysisType = YAML::APStatistics;
    QString iconString = ":imgs/analysis ap statistics.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    propertyDialog->setWindowTitle("Action potential statistics property panel");

    descriptionName->setText("Allows detection of action potentials and the estimation.\n"
                             "of several statistics, such as inter AP time, AP frequency,\n"
                             "AP durations, and AP rise slope.\n"
                             "\nTime points 1 and 2 must enclose the interval\n"
                             "I0 in which the action potentials are monitored.");
    descriptionName->setVisible(true);

    timePointsRequired = 2;

    protocolConsumerType = ProtocolConsumerApStatistics;

    this->initializeWidgets();
}

void ProtocolDropApStatisticsItem::onSetString() {
    QString validTimePoints = this->onCheckCursorsValidity();
    if (validTimePoints == "OK") {
        this->setText(QString("I0: cursors %1-%2")
                      .arg(timePointParams[0]->getCursorIdx())
                .arg(timePointParams[1]->getCursorIdx()));

    } else {
        this->setText(validTimePoints);
    }
}

void ProtocolDropApStatisticsItem::onAcceptPropertyDialog() {
    ProtocolDropAnalysisItem::onAcceptPropertyDialog();
}

void ProtocolDropApStatisticsItem::onRejectPropertyDialog() {
    ProtocolDropItem::onRejectPropertyDialog();
}

void ProtocolDropApStatisticsItem::onUpdateHold(double value) {
    hold = value;
}

QString ProtocolDropApStatisticsItem::onCheckCursorsValidity() {
    QString generalChecks = ProtocolDropAnalysisItem::onCheckCursorsValidity();
    if (generalChecks != "OK") {
        return generalChecks;
    }

    if (!(analysisCursors->at(0)->precedes(analysisCursors->at(1)))) {
        this->setInvalidPrecedenceToolTip();
        return QString("Cursor %1 must\nprecede cursor %2")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    if (!(analysisCursors->at(0)->sameRepetitions(analysisCursors->at(1)))) {
        this->setInvalidLoopsToolTip();
        return QString("Cursors %1 and %2\nmust execute the same loops")
                .arg(timePointParams[0]->getCursorIdx()).arg(timePointParams[1]->getCursorIdx());
    }

    this->setValidToolTip();
    return "OK";
}
