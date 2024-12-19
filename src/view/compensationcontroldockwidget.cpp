#include "compensationcontroldockwidget.h"
#include "messagedispatcher.h"

#include <vector>
#include <QScrollBar>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>

CompensationControlDockWidget::CompensationControlDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    this->setVisible(false);

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);

    QWidget * bigMainWg = new QWidget();
    QVBoxLayout * vLayout = new QVBoxLayout(bigMainWg);
    QHBoxLayout * intermedHorLayout = new QHBoxLayout();
    QWidget * vcMainWg = new QWidget();
    QWidget * ccMainWg = new QWidget();
    QVBoxLayout * vcVerLayout = new QVBoxLayout;
    QVBoxLayout * ccVerLayout = new QVBoxLayout;
    QWidget * vcCompControlsWg = new QWidget();
    QWidget * ccCompControlsWg = new QWidget();
    QGridLayout * mainVcGridLayout = this->getLayoutWithScrollBar(vcCompControlsWg);
    QGridLayout * mainCcGridLayout = this->getLayoutWithScrollBar(ccCompControlsWg);

    bigMainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Compensation controls");
    this->setWidget(bigMainWg);
    setObjectName("compensationControlsDw");

    vLayout->setContentsMargins(0, 0, 0, 2);
    vLayout->setSpacing(1);
    vLayout->addLayout(intermedHorLayout);

    intermedHorLayout -> addWidget(vcMainWg);
    intermedHorLayout -> addWidget(ccMainWg);

    vcMainWg -> setLayout(vcVerLayout);
    ccMainWg -> setLayout(ccVerLayout);

    vcVerLayout -> addWidget(vcCompControlsWg);

    ccVerLayout -> addWidget(ccCompControlsWg);

    std::vector<RangedMeasurement_t> compensationFeatures;
    std::vector<RangedMeasurement_t> compensationFeaturesBis;
    compensationFeatures.resize(currentChannelsNum);
    compensationFeaturesBis.resize(currentChannelsNum);
    double defaultParamValue;
    double defaultParamValueBis;

    /*! --------------------------------- VC compensation controls --------------------------------- */
    for(int idx = 0; idx < currentChannelsNum+1; idx++){
        this->hbsCfast.push_back(new QHBoxLayout());
        this->hbsCslowRs.push_back(new QHBoxLayout());
        this->hbsRsCp.push_back(new QHBoxLayout());
        this->hbsRsPg.push_back(new QHBoxLayout());
        hbsCfast[idx]->setContentsMargins(0,0,0,0);
        hbsCfast[idx]->setSpacing(1);
        hbsCslowRs[idx]->setContentsMargins(0,0,0,0);
        hbsCslowRs[idx]->setSpacing(1);
        hbsRsCp[idx]->setContentsMargins(0,0,0,0);
        hbsRsCp[idx]->setSpacing(1);
        hbsRsPg[idx]->setContentsMargins(0,0,0,0);
        hbsRsPg[idx]->setSpacing(1);
    }

    /*! Cfast checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_CpVc, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* cfastEnableCb;

        cfastEnableCb = new QCheckBox();
        cfastEnableCb->setCheckable(true);
        cfastEnableCb->setChecked(false);
        hbsCfast[0]->addWidget(cfastEnableCb);

        connect(cfastEnableCb, &QPushButton::clicked, this, [=] (bool flag) {
            for (auto c : cfastCheckBoxes) {
                c->setChecked(flag);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            cfastEnableCb = new QCheckBox();
            cfastEnableCb->setCheckable(true);
            cfastEnableCb->setChecked(false);
            cfastCheckBoxes.push_back(cfastEnableCb);
            hbsCfast[idx+1]->addWidget(cfastEnableCb);
        }

        NoWheelSpinBox* cfastSpinBox;
        QString cfastUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());

        cfastSpinBox = new NoWheelSpinBox();
        cfastSpinBox->setSuffix(QString(" ") + cfastUnit);
        cfastSpinBox->setRange(compensationFeatures[0].min, compensationFeatures[0].max);
        cfastSpinBox->setValue(defaultParamValue);
        cfastSpinBox->setDecimals(compensationFeatures[0].decimals());
        hbsCfast[0]->addWidget(cfastSpinBox);

        connect(cfastSpinBox, &QDoubleSpinBox::editingFinished, this, [=] () {
            auto value = cfastSpinBox->value();
            for (auto c : cfastSpinBoxes) {
                c->setValue(value);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            cfastSpinBox = new NoWheelSpinBox();
            cfastSpinBox->setSuffix(QString(" ") + cfastUnit);
            cfastSpinBox->setRange(compensationFeatures[idx].min, compensationFeatures[idx].max);
            cfastSpinBox->setValue(defaultParamValue);
            cfastSpinBox->setDecimals(compensationFeatures[idx].decimals());
            cfastSpinBoxes.push_back(cfastSpinBox);
            hbsCfast[idx+1]->addWidget(cfastSpinBox);
        }
    }

    /*! Cslow and Rs checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_Cm, compensationFeatures, defaultParamValue) == Success && msgDisp->getCompFeatures(MessageDispatcher::U_Rs, compensationFeaturesBis, defaultParamValueBis) == Success) {
        QCheckBox* cslowRsEnableCb;

        cslowRsEnableCb = new QCheckBox();
        cslowRsEnableCb->setCheckable(true);
        cslowRsEnableCb->setChecked(false);
        hbsCslowRs[0]->addWidget(cslowRsEnableCb);

        connect(cslowRsEnableCb, &QPushButton::clicked, this, [=] (bool flag) {
            for (auto c : cslowRsCheckBoxes) {
                c->setChecked(flag);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            cslowRsEnableCb = new QCheckBox();
            cslowRsEnableCb->setCheckable(true);
            cslowRsEnableCb->setChecked(false);
            cslowRsCheckBoxes.push_back(cslowRsEnableCb);
            hbsCslowRs[idx+1]->addWidget(cslowRsEnableCb);
        }

        NoWheelSpinBox* cslowSpinBox;
        QString cslowUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());

        cslowSpinBox = new NoWheelSpinBox();
        cslowSpinBox->setSuffix(QString(" ") + cslowUnit);
        cslowSpinBox->setRange(compensationFeatures[0].min, compensationFeatures[0].max);
        cslowSpinBox->setValue(defaultParamValue);
        cslowSpinBox->setDecimals(compensationFeatures[0].decimals());
        hbsCslowRs[0]->addWidget(cslowSpinBox);

        connect(cslowSpinBox, &QDoubleSpinBox::editingFinished, this, [=] () {
            auto value = cslowSpinBox->value();
            for (auto c : cslowSpinBoxes) {
                c->setValue(value);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            cslowSpinBox = new NoWheelSpinBox();
            cslowSpinBox->setSuffix(QString(" ") + cslowUnit);
            cslowSpinBox->setRange(compensationFeatures[idx].min, compensationFeatures[idx].max);
            cslowSpinBox->setValue(defaultParamValue);
            cslowSpinBox->setDecimals(compensationFeatures[idx].decimals());
            cslowSpinBoxes.push_back(cslowSpinBox);
            hbsCslowRs[idx+1]->addWidget(cslowSpinBox);
        }

        NoWheelSpinBox* rsSpinBox;
        QString rsUnit = QString().fromStdString(compensationFeaturesBis[0].getFullUnit());

        rsSpinBox = new NoWheelSpinBox();
        rsSpinBox->setSuffix(QString(" ") + rsUnit);
        rsSpinBox->setRange(compensationFeaturesBis[0].min, compensationFeaturesBis[0].max);
        rsSpinBox->setValue(defaultParamValueBis);
        rsSpinBox->setDecimals(compensationFeaturesBis[0].decimals());
        hbsCslowRs[0]->addWidget(rsSpinBox);

        connect(rsSpinBox, &QDoubleSpinBox::editingFinished, this, [=] () {
            auto value = rsSpinBox->value();
            for (auto c : rsSpinBoxes) {
                c->setValue(value);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            rsSpinBox = new NoWheelSpinBox();
            rsSpinBox->setSuffix(QString(" ") + rsUnit);
            rsSpinBox->setRange(compensationFeaturesBis[idx].min, compensationFeaturesBis[idx].max);
            rsSpinBox->setValue(defaultParamValueBis);
            rsSpinBox->setDecimals(compensationFeaturesBis[idx].decimals());
            rsSpinBoxes.push_back(rsSpinBox);
            hbsCslowRs[idx+1]->addWidget(rsSpinBox);
        }
    }

    /*! RsCp checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_RsCp, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* rsCpEnableCb;

        rsCpEnableCb = new QCheckBox();
        rsCpEnableCb->setCheckable(true);
        rsCpEnableCb->setChecked(false);
        hbsRsCp[0]->addWidget(rsCpEnableCb);

        connect(rsCpEnableCb, &QPushButton::clicked, this, [=] (bool flag) {
            for (auto c : rsCpCheckBoxes) {
                c->setChecked(flag);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            rsCpEnableCb = new QCheckBox();
            rsCpEnableCb->setCheckable(true);
            rsCpEnableCb->setChecked(false);
            rsCpCheckBoxes.push_back(rsCpEnableCb);
            hbsRsCp[idx+1]->addWidget(rsCpEnableCb);
        }

        NoWheelSpinBox* rsCpSpinBox;
        QString rsCpUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());

        rsCpSpinBox = new NoWheelSpinBox();
        rsCpSpinBox->setSuffix(QString(" ") + rsCpUnit);
        rsCpSpinBox->setRange(compensationFeatures[0].min, compensationFeatures[0].max);
        rsCpSpinBox->setValue(defaultParamValue);
        rsCpSpinBox->setDecimals(compensationFeatures[0].decimals());
        hbsRsCp[0]->addWidget(rsCpSpinBox);

        connect(rsCpSpinBox, &QDoubleSpinBox::editingFinished, this, [=] () {
            auto value = rsCpSpinBox->value();
            for (auto c : rsCpSpinBoxes) {
                c->setValue(value);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            rsCpSpinBox = new NoWheelSpinBox();
            rsCpSpinBox->setSuffix(QString(" ") + rsCpUnit);
            rsCpSpinBox->setRange(compensationFeatures[idx].min, compensationFeatures[idx].max);
            rsCpSpinBox->setValue(defaultParamValue);
            rsCpSpinBox->setDecimals(compensationFeatures[idx].decimals());
            rsCpSpinBoxes.push_back(rsCpSpinBox);
            hbsRsCp[idx+1]->addWidget(rsCpSpinBox);
        }
    }

    /*! RsPg checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_RsPg, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* rsPgEnableCb;

        rsPgEnableCb = new QCheckBox();
        rsPgEnableCb->setCheckable(true);
        rsPgEnableCb->setChecked(false);
        hbsRsPg[0]->addWidget(rsPgEnableCb);

        connect(rsPgEnableCb, &QPushButton::clicked, this, [=] (bool flag) {
            for (auto c : rsPgCheckBoxes) {
                c->setChecked(flag);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            rsPgEnableCb = new QCheckBox();
            rsPgEnableCb->setCheckable(true);
            rsPgEnableCb->setChecked(false);
            rsPgCheckBoxes.push_back(rsPgEnableCb);
            hbsRsPg[idx+1]->addWidget(rsPgEnableCb);
        }

        NoWheelSpinBox* rsPgSpinBox;
        QString rsPgUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());

        rsPgSpinBox = new NoWheelSpinBox();
        rsPgSpinBox->setSuffix(QString(" ") + rsPgUnit);
        rsPgSpinBox->setRange(compensationFeatures[0].min, compensationFeatures[0].max);
        rsPgSpinBox->setValue(defaultParamValue);
        rsPgSpinBox->setDecimals(compensationFeatures[0].decimals());
        hbsRsPg[0]->addWidget(rsPgSpinBox);

        connect(rsPgSpinBox, &QDoubleSpinBox::editingFinished, this, [=] () {
            auto value = rsPgSpinBox->value();
            for (auto c : rsPgSpinBoxes) {
                c->setValue(value);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            rsPgSpinBox = new NoWheelSpinBox();
            rsPgSpinBox->setSuffix(QString(" ") + rsPgUnit);
            rsPgSpinBox->setRange(compensationFeatures[idx].min, compensationFeatures[idx].max);
            rsPgSpinBox->setValue(defaultParamValue);
            rsPgSpinBox->setDecimals(compensationFeatures[idx].decimals());
            rsPgSpinBoxes.push_back(rsPgSpinBox);
            hbsRsPg[idx+1]->addWidget(rsPgSpinBox);
        }
    }

    // Column captions
    mainVcGridLayout->addWidget(new QLabel("Voltage Clamp"), 0, 0, 1, 2, Qt::AlignLeft);

    mainVcGridLayout->addWidget(new QLabel("Ch"), 1, 0, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("Cfast"), 1, 1, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("Cslow - Rs"), 1, 2, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("RsCorr"), 1, 3, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("RsPred"), 1, 4, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("RsBW"), 1, 5, Qt::AlignHCenter);

    mainVcGridLayout->addWidget(new QLabel("All"), 2, 0);
    mainVcGridLayout->addLayout(hbsCfast[0], 2, 1);
    mainVcGridLayout->addLayout(hbsCslowRs[0], 2, 2);
    mainVcGridLayout->addLayout(hbsRsCp[0], 2, 3);
    mainVcGridLayout->addLayout(hbsRsPg[0], 2, 4);

    for(int idx = 0; idx < currentChannelsNum; idx++){
        QString channelLabel = QString("%1").arg(idx+1);
        mainVcGridLayout->addWidget(new QLabel(channelLabel), idx+3, 0);
        mainVcGridLayout->addLayout(hbsCfast[idx+1], idx+3, 1);
        mainVcGridLayout->addLayout(hbsCslowRs[idx+1], idx+3, 2);
        mainVcGridLayout->addLayout(hbsRsCp[idx+1], idx+3, 3);
        mainVcGridLayout->addLayout(hbsRsPg[idx+1], idx+3, 4);
    }
    {
        QWidget * spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        mainVcGridLayout->addWidget(spacer, currentChannelsNum+3, 0);
    }

    /*! RsBW comboboxes*/
    std::vector <std::string> rsBwStringArray;
    if (msgDisp->getCompOptionsFeatures(MessageDispatcher::CompRsCorr, rsBwStringArray) == Success) {
        QComboBox* rsBwCb;

        rsBwCb = new QComboBox();
        for(int j = 0; j < rsBwStringArray.size(); j++){
            rsBwCb->addItem(QString::fromStdString(rsBwStringArray[j]));
        }
        mainVcGridLayout->addWidget(rsBwCb, 2, 5);

        connect(rsBwCb, &QComboBox::currentIndexChanged, this, [=] (int idx) {
            for (auto c : rsBwComboBoxes) {
                c->setCurrentIndex(idx);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            rsBwCb = new QComboBox();
            for(int j = 0; j < rsBwStringArray.size(); j++){
                rsBwCb->addItem(QString::fromStdString(rsBwStringArray[j]));
            }
            rsBwComboBoxes.push_back(rsBwCb);
            mainVcGridLayout->addWidget(rsBwCb, idx+3, 5);
        }
    }

    /*! --------------------------------- CC compensation controls --------------------------------- */
    /*! Cfast checkboxes and spinboxes*/
    for(int idx = 0; idx < currentChannelsNum+1; idx++){
        this->hbsCcCfast.push_back(new QHBoxLayout());
        hbsCcCfast[idx]->setContentsMargins(0,0,0,0);
        hbsCcCfast[idx]->setSpacing(1);
    }

    if (msgDisp->getCompFeatures(MessageDispatcher::U_CpCc, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* ccCfastEnableCb;

        ccCfastEnableCb = new QCheckBox();
        ccCfastEnableCb->setCheckable(true);
        ccCfastEnableCb->setChecked(false);
        hbsCcCfast[0]->addWidget(ccCfastEnableCb);

        connect(ccCfastEnableCb, &QPushButton::clicked, this, [=] (bool flag) {
            for (auto c : ccCfastCheckBoxes) {
                c->setChecked(flag);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            ccCfastEnableCb = new QCheckBox();
            ccCfastEnableCb->setCheckable(true);
            ccCfastEnableCb->setChecked(false);
            ccCfastCheckBoxes.push_back(ccCfastEnableCb);
            hbsCcCfast[idx+1]->addWidget(ccCfastEnableCb);
        }

        NoWheelSpinBox* ccCfastSpinBox;
        QString ccCfastUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());

        ccCfastSpinBox = new NoWheelSpinBox();
        ccCfastSpinBox->setSuffix(QString(" ") + ccCfastUnit);
        ccCfastSpinBox->setRange(compensationFeatures[0].min, compensationFeatures[0].max);
        ccCfastSpinBox->setValue(defaultParamValue);
        ccCfastSpinBox->setDecimals(compensationFeatures[0].decimals());
        hbsCcCfast[0]->addWidget(ccCfastSpinBox);

        connect(ccCfastSpinBox, &QDoubleSpinBox::editingFinished, this, [=] () {
            auto value = ccCfastSpinBox->value();
            for (auto c : ccCfastSpinBoxes) {
                c->setValue(value);
            }
        });

        for(int idx = 0; idx < currentChannelsNum; idx++){
            ccCfastSpinBox = new NoWheelSpinBox();
            ccCfastSpinBox->setSuffix(QString(" ") + ccCfastUnit);
            ccCfastSpinBox->setRange(compensationFeatures[idx].min, compensationFeatures[idx].max);
            ccCfastSpinBox->setValue(defaultParamValue);
            ccCfastSpinBox->setDecimals(compensationFeatures[idx].decimals());
            ccCfastSpinBoxes.push_back(ccCfastSpinBox);
            hbsCcCfast[idx+1]->addWidget(ccCfastSpinBox);
        }
    }

    // Column captions
    mainCcGridLayout->addWidget(new QLabel("Current Clamp"), 0, 0, 1, 2, Qt::AlignLeft);

    mainCcGridLayout->addWidget(new QLabel("Ch"), 1, 0, Qt::AlignHCenter);
    mainCcGridLayout->addWidget(new QLabel("Cfast"), 1, 1, Qt::AlignHCenter);

    mainCcGridLayout->addWidget(new QLabel("All"), 2, 0);
    mainCcGridLayout->addLayout(hbsCcCfast[0], 2, 1);

    for(int idx = 0; idx < currentChannelsNum; idx++){
        QString channelLabel= QString("%1").arg(idx+1);
        mainCcGridLayout->addWidget(new QLabel(channelLabel), idx+3, 0);
        mainCcGridLayout->addLayout(hbsCcCfast[idx+1], idx+3, 1);
    }
    {
        QWidget * spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        mainCcGridLayout->addWidget(spacer, currentChannelsNum+3, 0);
    }

    {
        QWidget * spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mainCcGridLayout->addWidget(spacer, 0, 2);
    }

    /*! here we restart with the main vertical VLayout*/
    QPushButton* applyButton = new QPushButton("Apply");
    vLayout->addWidget(applyButton);
    connect(applyButton, &QPushButton::clicked, this, &CompensationControlDockWidget::onApplyButtonClicked);
}

QGridLayout * CompensationControlDockWidget::getLayoutWithScrollBar(QWidget * widget) {
    QVBoxLayout * vl = new QVBoxLayout;
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(1);
    widget->setLayout(vl);

    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    vl->addWidget(scrollArea);

    QWidget * scrollWg = new QWidget;
    scrollArea->setWidget(scrollWg);

    QGridLayout * scrollHl = new QGridLayout;
    scrollHl->setContentsMargins(0, 0, 20, 0);
    scrollHl->setSpacing(1);
    scrollWg->setLayout(scrollHl);

    return scrollHl;
}

void CompensationControlDockWidget::onApplyButtonClicked(){
    std::vector<uint16_t> channelIndexes;
    std::vector<bool> cfastEn;
    std::vector<bool> cslowRsEn;
    std::vector<bool> rsCpEn;
    std::vector<bool> rsPgEn;
    std::vector<bool> ccCfastEn;
    std::vector<double> cfastValues;
    std::vector<double> cslowValues;
    std::vector<double> rsValues;
    std::vector<double> rsCpValues;
    std::vector<double> rsPgValues;
    std::vector<uint16_t> rsBWValueIdxs;
    std::vector<double> ccCfastValues;

    for(int i = 0; i < currentChannelsNum; i++){
        channelIndexes.push_back(i);
        if (!cfastCheckBoxes.empty()) {
            cfastEn.push_back(cfastCheckBoxes[i]->isChecked());
        }
        if (!cslowRsCheckBoxes.empty()) {
            cslowRsEn.push_back(cslowRsCheckBoxes[i]->isChecked());
        }
        if (!rsCpCheckBoxes.empty()) {
            rsCpEn.push_back(rsCpCheckBoxes[i]->isChecked());
        }
        if (!rsPgCheckBoxes.empty()) {
            rsPgEn.push_back(rsPgCheckBoxes[i]->isChecked());
        }
        if (!ccCfastCheckBoxes.empty()) {
            ccCfastEn.push_back(ccCfastCheckBoxes[i]->isChecked());
        }
        if (!cfastSpinBoxes.empty()) {
            cfastValues.push_back(cfastSpinBoxes[i]->value());
        }
        if (!cslowSpinBoxes.empty()) {
            cslowValues.push_back(cslowSpinBoxes[i]->value());
        }
        if (!rsSpinBoxes.empty()) {
            rsValues.push_back(rsSpinBoxes[i]->value());
        }
        if (!rsCpSpinBoxes.empty()) {
            rsCpValues.push_back(rsCpSpinBoxes[i]->value());
        }
        if (!rsPgSpinBoxes.empty()) {
            rsPgValues.push_back(rsPgSpinBoxes[i]->value());
        }
        if (!rsBwComboBoxes.empty()) {
            rsBWValueIdxs.push_back(rsBwComboBoxes[i]->currentIndex());
        }
        if (!ccCfastSpinBoxes.empty()) {
            ccCfastValues.push_back(ccCfastSpinBoxes[i]->value());
        }
    }

    emit sigCompensationsApplied(channelIndexes, cfastEn, cslowRsEn, rsCpEn, rsPgEn, cfastValues, cslowValues, rsValues, rsCpValues, rsPgValues, rsBWValueIdxs, ccCfastEn, ccCfastValues);
}

/*! It updates in GUI both the param values and ranges based on potenial clipings and conversions done at the asic domain by the messageDispatcher*/
void CompensationControlDockWidget::onCompValuesDispatched(std::vector<std::vector<double>> compValueMatrix, std::vector<RangedMeasurement> cfastFeatures, std::vector<RangedMeasurement> cslowFeatures, std::vector<RangedMeasurement> rsFeatures, std::vector<RangedMeasurement> rsCpFeatures, std::vector<RangedMeasurement> rsPgFeatures, std::vector<RangedMeasurement> ccCfastFeatures){
    ClampingModality_t mode;
    msgDisp->getClampingModality(mode);
    for (int i = 0; i < currentChannelsNum; i++){
        if (!cfastSpinBoxes.empty()) {
            cfastSpinBoxes[i]->setRange(cfastFeatures[i].min, cfastFeatures[i].max);
            cfastSpinBoxes[i]->setDecimals(cfastFeatures[i].decimals());
        }

        if (!ccCfastSpinBoxes.empty()) {
            ccCfastSpinBoxes[i]->setRange(ccCfastFeatures[i].min, ccCfastFeatures[i].max);
            ccCfastSpinBoxes[i]->setDecimals(ccCfastFeatures[i].decimals());
        }

        if(mode == ClampingModality_t::VOLTAGE_CLAMP){
            if (!cfastSpinBoxes.empty()) {
                cfastSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_CpVc]);
            }
        } else if(mode == ClampingModality_t::ZERO_CURRENT_CLAMP || mode == ClampingModality_t::CURRENT_CLAMP) {
            if (!ccCfastSpinBoxes.empty()) {
                ccCfastSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_CpCc]);
            }
        } else {
            /*! \todo MPAC ancora da fare*/
        }

        if (!cslowSpinBoxes.empty()) {
            cslowSpinBoxes[i]->setRange(cslowFeatures[i].min, cslowFeatures[i].max);
            cslowSpinBoxes[i]->setDecimals(cslowFeatures[i].decimals());
            cslowSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_Cm]);
        }

        if (!rsSpinBoxes.empty()) {
            rsSpinBoxes[i]->setRange(rsFeatures[i].min, rsFeatures[i].max);
            rsSpinBoxes[i]->setDecimals(rsFeatures[i].decimals());
            rsSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_Rs]);
        }

        if (!rsCpSpinBoxes.empty()) {
            rsCpSpinBoxes[i]->setRange(rsCpFeatures[i].min, rsCpFeatures[i].max);
            rsCpSpinBoxes[i]->setDecimals(rsCpFeatures[i].decimals());
            rsCpSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_RsCp]);
        }

        if (!rsPgSpinBoxes.empty()) {
            rsPgSpinBoxes[i]->setRange(rsPgFeatures[i].min, rsPgFeatures[i].max);
            rsPgSpinBoxes[i]->setDecimals(rsPgFeatures[i].decimals());
            rsPgSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_RsPg]);
        }
    }
}
