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

    int localNumOfVoltChans;
    int localNumOfCurrChans;
    msgDisp->getChannelNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);

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
    QGridLayout* checkAllLayout = new QGridLayout();
    QGridLayout* ccCheckAllLayout = new QGridLayout();

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
    vcMainWg -> setWindowTitle("Voltage Clamp Compensations");
    ccMainWg -> setLayout(ccVerLayout);
    ccMainWg -> setWindowTitle("Current Clamp Compensations");

//    ccMainWg -> setVisible(false);

    vcVerLayout -> addWidget(vcCompControlsWg);
    vcVerLayout -> addLayout(checkAllLayout);

    ccVerLayout -> addWidget(ccCompControlsWg);
    ccVerLayout -> addLayout(ccCheckAllLayout);

    std::vector<RangedMeasurement_t> compensationFeatures;
    std::vector<RangedMeasurement_t> compensationFeaturesBis;
    compensationFeatures.resize(localNumOfCurrChans);
    compensationFeaturesBis.resize(localNumOfCurrChans);
    double defaultParamValue;
    double defaultParamValueBis;

    /*! --------------------------------- VC compensation controls --------------------------------- */
    for(int i = 1; i <= localNumOfCurrChans; i++){
        this->hbsCfast.push_back(new QHBoxLayout());
        this->hbsCslowRs.push_back(new QHBoxLayout());
        this->hbsRsCp.push_back(new QHBoxLayout());
        this->hbsRsPg.push_back(new QHBoxLayout());
        hbsCfast[i-1]->setContentsMargins(0,0,0,0);
        hbsCfast[i-1]->setSpacing(1);
        hbsCslowRs[i-1]->setContentsMargins(0,0,0,0);
        hbsCslowRs[i-1]->setSpacing(1);
        hbsRsCp[i-1]->setContentsMargins(0,0,0,0);
        hbsRsCp[i-1]->setSpacing(1);
        hbsRsPg[i-1]->setContentsMargins(0,0,0,0);
        hbsRsPg[i-1]->setSpacing(1);
    }

    /*! Cfast checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_CpVc, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* cfastEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cfastEnableCb = new QCheckBox();
            cfastEnableCb->setCheckable(true);
            cfastEnableCb->setChecked(false);
            this->cfastCheckBoxes.push_back(cfastEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCfast[i-1]->addWidget(spazietto);
            hbsCfast[i-1]->addWidget(cfastEnableCb);
        }

        MySpinBox* cfastSpinBox;
        QString cfastUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cfastSpinBox = new MySpinBox();
            cfastSpinBox->setSuffix(QString(" ") + cfastUnit);
            cfastSpinBox->setRange(compensationFeatures[i-1].min, compensationFeatures[i-1].max);
            cfastSpinBox->setValue(defaultParamValue);
            cfastSpinBox->setDecimals(compensationFeatures[i-1].decimals());
            this->cfastSpinBoxes.push_back(cfastSpinBox);
            hbsCfast[i-1]->addWidget(cfastSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCfast[i-1]->addWidget(spazietto);
        }
    }

    /*! Cslow and Rs checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_Cm, compensationFeatures, defaultParamValue) == Success && msgDisp->getCompFeatures(MessageDispatcher::U_Rs, compensationFeaturesBis, defaultParamValueBis) == Success) {
        QCheckBox* cslowRsEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cslowRsEnableCb = new QCheckBox();
            cslowRsEnableCb->setCheckable(true);
            cslowRsEnableCb->setChecked(false);
            this->cslowRsCheckBoxes.push_back(cslowRsEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCslowRs[i-1]->addWidget(spazietto);
            hbsCslowRs[i-1]->addWidget(cslowRsEnableCb);
        }

        MySpinBox* cslowSpinBox;
        QString cslowUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cslowSpinBox = new MySpinBox();
            cslowSpinBox->setSuffix(QString(" ") + cslowUnit);
            cslowSpinBox->setRange(compensationFeatures[i-1].min, compensationFeatures[i-1].max);
            cslowSpinBox->setValue(defaultParamValue);
            cslowSpinBox->setDecimals(compensationFeatures[i-1].decimals());
            this->cslowSpinBoxes.push_back(cslowSpinBox);
            hbsCslowRs[i-1]->addWidget(cslowSpinBox);
        }

        MySpinBox* rsSpinBox;
        QString rsUnit = QString().fromStdString(compensationFeaturesBis[0].getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsSpinBox = new MySpinBox();
            rsSpinBox->setSuffix(QString(" ") + rsUnit);
            rsSpinBox->setRange(compensationFeaturesBis[i-1].min, compensationFeaturesBis[i-1].max);
            rsSpinBox->setValue(defaultParamValueBis);
            rsSpinBox->setDecimals(compensationFeaturesBis[i-1].decimals());
            this->rsSpinBoxes.push_back(rsSpinBox);
            hbsCslowRs[i-1]->addWidget(rsSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCslowRs[i-1]->addWidget(spazietto);
        }
    }

    /*! RsCp checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_RsCp, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* rsCpEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsCpEnableCb = new QCheckBox();
            rsCpEnableCb->setCheckable(true);
            rsCpEnableCb->setChecked(false);
            this->rsCpCheckBoxes.push_back(rsCpEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsCp[i-1]->addWidget(spazietto);
            hbsRsCp[i-1]->addWidget(rsCpEnableCb);
        }

        MySpinBox* rsCpSpinBox;
        QString rsCpUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsCpSpinBox = new MySpinBox();
            rsCpSpinBox->setSuffix(QString(" ") + rsCpUnit);
            rsCpSpinBox->setRange(compensationFeatures[i-1].min, compensationFeatures[i-1].max);
            rsCpSpinBox->setValue(defaultParamValue);
            rsCpSpinBox->setDecimals(compensationFeatures[i-1].decimals());
            this->rsCpSpinBoxes.push_back(rsCpSpinBox);
            hbsRsCp[i-1]->addWidget(rsCpSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsCp[i-1]->addWidget(spazietto);
        }
    }

    /*! RsPg checkboxes and spinboxes*/
    if (msgDisp->getCompFeatures(MessageDispatcher::U_RsPg, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* rsPgEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsPgEnableCb = new QCheckBox();
            rsPgEnableCb->setCheckable(true);
            rsPgEnableCb->setChecked(false);
            this->rsPgCheckBoxes.push_back(rsPgEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsPg[i-1]->addWidget(spazietto);
            hbsRsPg[i-1]->addWidget(rsPgEnableCb);
        }

        MySpinBox* rsPgSpinBox;
        QString rsPgUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsPgSpinBox = new MySpinBox();
            rsPgSpinBox->setSuffix(QString(" ") + rsPgUnit);
            rsPgSpinBox->setRange(compensationFeatures[i-1].min, compensationFeatures[i-1].max);
            rsPgSpinBox->setValue(defaultParamValue);
            rsPgSpinBox->setDecimals(compensationFeatures[i-1].decimals());
            this->rsPgSpinBoxes.push_back(rsPgSpinBox);
            hbsRsPg[i-1]->addWidget(rsPgSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsPg[i-1]->addWidget(spazietto);
        }
    }

    // Column captions
    mainVcGridLayout->addWidget(new QLabel("Ch"), 0, 0, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("Cfast"), 0, 1, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("Cslow - Rs"), 0, 2, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("RsCorr"), 0, 3, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("RsPred"), 0, 4, Qt::AlignHCenter);
    mainVcGridLayout->addWidget(new QLabel("RsBW"), 0, 5, Qt::AlignHCenter);

    for(int i = 1; i <= localNumOfCurrChans; i++){
        QString channelLabel= QString("    %1").arg(i);
        mainVcGridLayout->addWidget(new QLabel(channelLabel), i, 0);
        mainVcGridLayout->addLayout(hbsCfast[i-1], i, 1);
        mainVcGridLayout->addLayout(hbsCslowRs[i-1], i, 2);
        mainVcGridLayout->addLayout(hbsRsCp[i-1], i, 3);
        mainVcGridLayout->addLayout(hbsRsPg[i-1], i, 4);
    }

    /*! RsBW comboboxes*/
    std::vector <std::string> rsBwStringArray;
    if (msgDisp->getCompOptionsFeatures(MessageDispatcher::CompRsCorr, rsBwStringArray) == Success) {
        QComboBox* rsBwCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsBwCb = new QComboBox();
            for(int j = 0; j < rsBwStringArray.size(); j++){
                rsBwCb->addItem(QString::fromStdString(rsBwStringArray[j]));
            }
            this->rsBwComboBoxes.push_back(rsBwCb);
            mainVcGridLayout->addWidget(rsBwCb,i, 5);
        }
    }


    // Column captions
    checkAllLayout->addWidget(new QLabel("Cfast"), 0, 0, Qt::AlignHCenter);
    checkAllLayout->addWidget(new QLabel("Cslow - Rs"), 0, 1, Qt::AlignHCenter);
    checkAllLayout->addWidget(new QLabel("RsCorr"), 0, 2, Qt::AlignHCenter);
    checkAllLayout->addWidget(new QLabel("RsPred"), 0, 3, Qt::AlignHCenter);

    QPushButton* checkAllCfast = new QPushButton("Check All");
    QPushButton* checkAllCslowRs = new QPushButton("Check All");
    QPushButton* checkAllRsCorr = new QPushButton("Check All");
    QPushButton* checkAllRsPred = new QPushButton("Check All");
    QPushButton* uncheckAllCfast = new QPushButton("Uncheck All");
    QPushButton* uncheckAllCslowRs = new QPushButton("Uncheck All");
    QPushButton* uncheckAllRsCorr = new QPushButton("Uncheck All");
    QPushButton* uncheckAllRsPred = new QPushButton("Uncheck All");

    checkAllLayout->addWidget(checkAllCfast, 1, 0, Qt::AlignHCenter);
    checkAllLayout->addWidget(uncheckAllCfast, 2, 0, Qt::AlignHCenter);

    checkAllLayout->addWidget(checkAllCslowRs, 1, 1, Qt::AlignHCenter);
    checkAllLayout->addWidget(uncheckAllCslowRs, 2, 1, Qt::AlignHCenter);

    checkAllLayout->addWidget(checkAllRsCorr, 1, 2, Qt::AlignHCenter);
    checkAllLayout->addWidget(uncheckAllRsCorr, 2, 2, Qt::AlignHCenter);

    checkAllLayout->addWidget(checkAllRsPred, 1, 3, Qt::AlignHCenter);
    checkAllLayout->addWidget(uncheckAllRsPred, 2, 3, Qt::AlignHCenter);

    connect(checkAllCfast, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < cfastCheckBoxes.size(); i++){
            cfastCheckBoxes[i]->setChecked(true);
        }
    });
    connect(uncheckAllCfast, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < cfastCheckBoxes.size(); i++){
            cfastCheckBoxes[i]->setChecked(false);
        }
    });

    connect(checkAllCslowRs, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < cslowRsCheckBoxes.size(); i++){
            cslowRsCheckBoxes[i]->setChecked(true);
        }
    });
    connect(uncheckAllCslowRs, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < cslowRsCheckBoxes.size(); i++){
            cslowRsCheckBoxes[i]->setChecked(false);
        }
    });

    connect(checkAllRsCorr, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < rsCpCheckBoxes.size(); i++){
            rsCpCheckBoxes[i]->setChecked(true);
        }
    });
    connect(uncheckAllRsCorr, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < rsCpCheckBoxes.size(); i++){
            rsCpCheckBoxes[i]->setChecked(false);
        }
    });

    connect(checkAllRsPred, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < rsPgCheckBoxes.size(); i++){
            rsPgCheckBoxes[i]->setChecked(true);
        }
    });
    connect(uncheckAllRsPred, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < rsPgCheckBoxes.size(); i++){
            rsPgCheckBoxes[i]->setChecked(false);
        }
    });

    /*! --------------------------------- CC compensation controls --------------------------------- */
    /*! Cfast checkboxes and spinboxes*/
    for(int i = 1; i <= localNumOfCurrChans; i++){
        this->hbsCcCfast.push_back(new QHBoxLayout());
        hbsCcCfast[i-1]->setContentsMargins(0,0,0,0);
        hbsCcCfast[i-1]->setSpacing(1);
    }

    if (msgDisp->getCompFeatures(MessageDispatcher::U_CpCc, compensationFeatures, defaultParamValue) == Success) {
        QCheckBox* ccCfastEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            ccCfastEnableCb = new QCheckBox();
            ccCfastEnableCb->setCheckable(true);
            ccCfastEnableCb->setChecked(false);
            this->ccCfastCheckBoxes.push_back(ccCfastEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCcCfast[i-1]->addWidget(spazietto);
            hbsCcCfast[i-1]->addWidget(ccCfastEnableCb);
        }

        MySpinBox* ccCfastSpinBox;
        QString ccCfastUnit = QString().fromStdString(compensationFeatures[0].getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            ccCfastSpinBox = new MySpinBox();
            ccCfastSpinBox->setSuffix(QString(" ") + ccCfastUnit);
            ccCfastSpinBox->setRange(compensationFeatures[i-1].min, compensationFeatures[i-1].max);
            ccCfastSpinBox->setValue(defaultParamValue);
            ccCfastSpinBox->setDecimals(compensationFeatures[i-1].decimals());
            this->ccCfastSpinBoxes.push_back(ccCfastSpinBox);
            hbsCcCfast[i-1]->addWidget(ccCfastSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCcCfast[i-1]->addWidget(spazietto);
        }
    }

    // Column captions
    mainCcGridLayout->addWidget(new QLabel("Ch"), 0, 0, Qt::AlignHCenter);
    mainCcGridLayout->addWidget(new QLabel("Cfast"), 0, 1, Qt::AlignHCenter);

    for(int i = 1; i <= localNumOfCurrChans; i++){
        QString channelLabel= QString("    %1").arg(i);
        mainCcGridLayout->addWidget(new QLabel(channelLabel), i, 0);
        mainCcGridLayout->addLayout(hbsCcCfast[i-1], i, 1);
    }


    // Column captions
    ccCheckAllLayout->addWidget(new QLabel("Cfast"), 0, 0, Qt::AlignHCenter);


    QPushButton* checkAllCcCfast = new QPushButton("Check All");
    QPushButton* uncheckAllCcCfast = new QPushButton("Uncheck All");


    ccCheckAllLayout->addWidget(checkAllCcCfast, 1, 0, Qt::AlignHCenter);
    ccCheckAllLayout->addWidget(uncheckAllCcCfast, 2, 0, Qt::AlignHCenter);

    connect(checkAllCcCfast, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < ccCfastCheckBoxes.size(); i++){
            ccCfastCheckBoxes[i]->setChecked(true);
        }
    });
    connect(uncheckAllCcCfast, &QPushButton::clicked, this, [=] () {
        for(int i = 0; i < ccCfastCheckBoxes.size(); i++){
            ccCfastCheckBoxes[i]->setChecked(false);
        }
    });

    /*! here we restart with the main verical VLayout*/
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

    int localNumOfVoltChans;
    int localNumOfCurrChans;
    msgDisp->getChannelNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);
    for(int i = 0; i < localNumOfCurrChans; i++){
        channelIndexes.push_back(i);
        cfastEn.push_back(cfastCheckBoxes[i]->isChecked());
        cslowRsEn.push_back(cslowRsCheckBoxes[i]->isChecked());
        rsCpEn.push_back(rsCpCheckBoxes[i]->isChecked());
        rsPgEn.push_back(rsPgCheckBoxes[i]->isChecked());
        ccCfastEn.push_back(ccCfastCheckBoxes[i]->isChecked());
        cfastValues.push_back(cfastSpinBoxes[i]->value());
        cslowValues.push_back(cslowSpinBoxes[i]->value());
        rsValues.push_back(rsSpinBoxes[i]->value());
        rsCpValues.push_back(rsCpSpinBoxes[i]->value());
        rsPgValues.push_back(rsPgSpinBoxes[i]->value());
        rsBWValueIdxs.push_back(rsBwComboBoxes[i]->currentIndex());
        ccCfastValues.push_back(ccCfastSpinBoxes[i]->value());
    }

    emit sigCompensationsApplied(channelIndexes, cfastEn, cslowRsEn, rsCpEn, rsPgEn, cfastValues, cslowValues, rsValues, rsCpValues, rsPgValues, rsBWValueIdxs, ccCfastEn, ccCfastValues);
}

/*! It updates in GUI both the param values and ranges based on potenial clipings and covnersions done at the asic domain by the messageDispatcher*/
void CompensationControlDockWidget::onCompValuesDispatched(std::vector<std::vector<double>> compValueMatrix, std::vector<RangedMeasurement> cfastFeatures, std::vector<RangedMeasurement> cslowFeatures, std::vector<RangedMeasurement> rsFeatures, std::vector<RangedMeasurement> rsCpFeatures, std::vector<RangedMeasurement> rsPgFeatures, std::vector<RangedMeasurement> ccCfastFeatures){
    int localNumOfVoltChans;
    int localNumOfCurrChans;
    ClampingModality_t mode;
    msgDisp->getChannelNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);
    msgDisp->getClampingModality(mode);
    for (int i = 0; i < localNumOfCurrChans; i++){
        cfastSpinBoxes[i]->setRange(cfastFeatures[i].min, cfastFeatures[i].max);
        cfastSpinBoxes[i]->setDecimals(cfastFeatures[i].decimals());

        ccCfastSpinBoxes[i]->setRange(ccCfastFeatures[i].min, ccCfastFeatures[i].max);
        ccCfastSpinBoxes[i]->setDecimals(ccCfastFeatures[i].decimals());

        if(mode == ClampingModality_t::VOLTAGE_CLAMP){
            cfastSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_CpVc]);
        } else if(mode == ClampingModality_t::ZERO_CURRENT_CLAMP || mode == ClampingModality_t::CURRENT_CLAMP) {
            ccCfastSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_CpCc]);
        } else {
            /*! \todo MPAC ancora da fare*/
        }

        cslowSpinBoxes[i]->setRange(cslowFeatures[i].min, cslowFeatures[i].max);
        cslowSpinBoxes[i]->setDecimals(cslowFeatures[i].decimals());
        cslowSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_Cm]);

        rsSpinBoxes[i]->setRange(rsFeatures[i].min, rsFeatures[i].max);
        rsSpinBoxes[i]->setDecimals(rsFeatures[i].decimals());
        rsSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_Rs]);

        rsCpSpinBoxes[i]->setRange(rsCpFeatures[i].min, rsCpFeatures[i].max);
        rsCpSpinBoxes[i]->setDecimals(rsCpFeatures[i].decimals());
        rsCpSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_RsCp]);

        rsPgSpinBoxes[i]->setRange(rsPgFeatures[i].min, rsPgFeatures[i].max);
        rsPgSpinBoxes[i]->setDecimals(rsPgFeatures[i].decimals());
        rsPgSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_RsPg]);
    }
}
