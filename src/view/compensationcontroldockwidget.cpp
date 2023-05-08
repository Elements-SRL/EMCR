#include "compensationcontroldockwidget.h"
#include "messagedispatcher.h"

#include <vector>
#include <QScrollBar>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>

CompensationControlDockWidget::CompensationControlDockWidget(ModelDevice * mDev, QWidget * parent) :
    QDockWidget(parent),
    mDev(mDev) {
    int localNumOfVoltChans;
    int localNumOfCurrChans;
    mDev->getChannelsNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);

    bool anyControlFlag = false;

    QWidget * bigMainWg = new QWidget();
    bigMainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Compensation controls");

    QWidget * mainWg = new QWidget();

    this->setWidget(bigMainWg);
    QVBoxLayout * vLayout = new QVBoxLayout(bigMainWg);
    vLayout->setContentsMargins(0, 0, 0, 2);
    vLayout->setSpacing(1);
    vLayout->addWidget(mainWg);

    QGridLayout * mainGridLayout = this->getLayoutWithScrollBar(mainWg);

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
    vector<RangedMeasurement_t> compensationFeatures;
    vector<RangedMeasurement_t> compensationFeaturesBis;
    compensationFeatures.resize(localNumOfCurrChans);
    compensationFeaturesBis.resize(localNumOfCurrChans);
    double defaultParamValue;
    double defaultParamValueBis;
    if (mDev->getCompFeatures(MessageDispatcher::U_CpVc, compensationFeatures, defaultParamValue) == Success) {
        anyControlFlag = true;
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
    if (mDev->getCompFeatures(MessageDispatcher::U_Cm, compensationFeatures, defaultParamValue) == Success && mDev->getCompFeatures(MessageDispatcher::U_Rs, compensationFeaturesBis, defaultParamValueBis) == Success) {
        anyControlFlag = true;
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
    if (mDev->getCompFeatures(MessageDispatcher::U_RsCp, compensationFeatures, defaultParamValue) == Success) {
        anyControlFlag = true;
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
    if (mDev->getCompFeatures(MessageDispatcher::U_RsPg, compensationFeatures, defaultParamValue) == Success) {
        anyControlFlag = true;

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
    mainGridLayout->addWidget(new QLabel("Ch"), 0, 0, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel(""), 0, 1);
    mainGridLayout->addWidget(new QLabel("Cfast"), 0, 1, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel("En"), 0, 3);
    mainGridLayout->addWidget(new QLabel("Cslow - Rs"), 0, 2, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel("Rs"), 0, 5);
//    mainGridLayout->addWidget(new QLabel("En"), 0, 6);
    mainGridLayout->addWidget(new QLabel("RsCorr"), 0, 3, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel("En"), 0, 8);
    mainGridLayout->addWidget(new QLabel("RsPred"), 0, 4, Qt::AlignHCenter);
    mainGridLayout->addWidget(new QLabel("RsBW"), 0, 5, Qt::AlignHCenter);

    for(int i = 1; i <= localNumOfCurrChans; i++){
        QString channelLabel= QString("    %1").arg(i);
        mainGridLayout->addWidget(new QLabel(channelLabel), i, 0);
        mainGridLayout->addLayout(hbsCfast[i-1], i, 1);
        mainGridLayout->addLayout(hbsCslowRs[i-1], i, 2);
        mainGridLayout->addLayout(hbsRsCp[i-1], i, 3);
        mainGridLayout->addLayout(hbsRsPg[i-1], i, 4);
    }

    /*! RsBW comboboxes*/
    std::vector <std::string> rsBwStringArray;
    if (mDev->getCompOptionsFeatures(MessageDispatcher::CompRsCorr, rsBwStringArray) == Success) {
        anyControlFlag = true;
        QComboBox* rsBwCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsBwCb = new QComboBox();
            for(int j = 0; j < rsBwStringArray.size(); j++){
                rsBwCb->addItem(QString::fromStdString(rsBwStringArray[j]));
            }
            this->rsBwComboBoxes.push_back(rsBwCb);
            mainGridLayout->addWidget(rsBwCb,i, 5);
        }
    }

    QPushButton* applyButton = new QPushButton("Apply");
    vLayout->addWidget(applyButton);
    connect(applyButton, &QPushButton::clicked, this, &CompensationControlDockWidget::onApplyButtonClicked);

    if (!anyControlFlag) {
        this->setEnabled(false);
        this->setVisible(false);
    }
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
    vector<uint16_t> channelIndexes;
    vector<bool> cfastEn;
    vector<bool> cslowRsEn;
    vector<bool> rsCpEn;
    vector<bool> rsPgEn;
    vector<double> cfastValues;
    vector<double> cslowValues;
    vector<double> rsValues;
    vector<double> rsCpValues;
    vector<double> rsPgValues;
    vector<uint16_t> rsBWValueIdxs;

    int localNumOfVoltChans;
    int localNumOfCurrChans;
    mDev->getChannelsNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);
    for(int i = 0; i < localNumOfCurrChans; i++){
        channelIndexes.push_back(i);
        cfastEn.push_back(cfastCheckBoxes[i]->isChecked());
        cslowRsEn.push_back(cslowRsCheckBoxes[i]->isChecked());
        rsCpEn.push_back(rsCpCheckBoxes[i]->isChecked());
        rsPgEn.push_back(rsPgCheckBoxes[i]->isChecked());
        cfastValues.push_back(cfastSpinBoxes[i]->value());
        cslowValues.push_back(cslowSpinBoxes[i]->value());
        rsValues.push_back(rsSpinBoxes[i]->value());
        rsCpValues.push_back(rsCpSpinBoxes[i]->value());
        rsPgValues.push_back(rsPgSpinBoxes[i]->value());
        rsBWValueIdxs.push_back(rsBwComboBoxes[i]->currentIndex());
    }

    emit sigCompensationsApplied(channelIndexes, cfastEn, cslowRsEn, rsCpEn, rsPgEn, cfastValues, cslowValues, rsValues, rsCpValues, rsPgValues, rsBWValueIdxs);

}

/*! It updates in GUI both the param values and ranges based on potenial clipings and covnersions done at the asic domain by the messageDispatcher*/
void CompensationControlDockWidget::onCompValuesDispatched(vector<vector<double>> compValueMatrix, vector<RangedMeasurement> cfastFeatures, vector<RangedMeasurement> cslowFeatures, vector<RangedMeasurement> rsFeatures, vector<RangedMeasurement> rsCpFeatures, vector<RangedMeasurement> rsPgFeatures){
    int localNumOfVoltChans;
    int localNumOfCurrChans;
    int ongoingClampingMode;
    mDev->getChannelsNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);
    ongoingClampingMode = mDev->getOngoingClampingModality();
    for (int i = 0; i < localNumOfCurrChans; i++){
        cfastSpinBoxes[i]->setRange(cfastFeatures[i].min, cfastFeatures[i].max);
        cfastSpinBoxes[i]->setDecimals(cfastFeatures[i].decimals());
        if(ongoingClampingMode == E384CL_VOLTAGE_CLAMP_MODE){
            cfastSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_CpVc]);
        } else if(ongoingClampingMode == E384CL_ZERO_CURRENT_CLAMP_MODE || ongoingClampingMode == E384CL_CURRENT_CLAMP_MODE) {
            cfastSpinBoxes[i]->setValue(compValueMatrix[i][MessageDispatcher::U_CpCc]);
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
