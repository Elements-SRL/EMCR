#include "protocolcursor.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>

ProtocolCursor::ProtocolCursor(ModelDevice * mDev, QwtPlot * plot, double x, int cursorIdx) :
    QObject(),
    mDev(mDev),
    cursorIdx(cursorIdx) {

    marker = new QwtPlotMarker();
    marker->setLineStyle(QwtPlotMarker::VLine);
    marker->setLinePen(QColor("darkgray"), 2.0, Qt::DotLine);
    marker->setXValue(x);
    marker->attach(plot);

    QwtText labelText(QString::number(cursorIdx));
    QColor c("#333333");
    labelText.setBorderPen(QPen(Qt::NoPen));
    labelText.setColor(c);
    c.setAlpha(50);
    labelText.setBackgroundBrush(c);
    marker->setLabel(labelText);
    marker->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);

    this->initializePropertyDialog();
}

ProtocolCursor::~ProtocolCursor() {
    marker->detach();
    delete marker;

    if (propertyDialog != nullptr) {
        delete propertyDialog;
        propertyDialog = nullptr;
    }
}

void ProtocolCursor::openPropertyDialog() {
    protocolSectionOrig = protocolSection;
    propertyDialog->show();
}

void ProtocolCursor::setVisible(bool visible) {
    marker->setVisible(visible);
}

void ProtocolCursor::setSection(ProtocolSection * section, double offset, ProtocolType_t protocolType) {
    /*! setSection is called with no pre-exsting protocolSection only during import from .epml files */
    bool importedCursorsFlag = protocolSection == nullptr;
    int repsNumOrig = (importedCursorsFlag ? 0 : protocolSection->repsNum);

    protocolSection = section;
    itemIdx = protocolSection->itemIdx;
    this->protocolType = protocolType;

    sweepGb->setVisible(protocolType == ProtocolTypeEpisodic);

    this->setMaxDuration();
    if (locationCb->currentIndex() == LocationFromStart) {
        locationSb->setValue(offset);

    } else {
        locationSb->setValue(protocolSection->duration()-offset);
    }
    locationDelay = locationSb->value();

    if (protocolSection->repsNum > 0) {
        repetitionSb->setMaximum(protocolSection->repsNum);
        repetitionSb->setValue(protocolSection->repsIdx+1); /*!< Human reference is 1-based, not 0-based */
        repetitionIdx = repetitionSb->value();
        lastRepItem->setFlags(lastRepItem->flags() | Qt::ItemIsEnabled);

    } else {
        repetitionSb->setMaximum(std::numeric_limits <int> ::max());
        /*! Within an infinite repetition loop update the repsIdx only if it changes from 0 or to 0, i.e. product = 0,
            or if we come from an item that is not in an infinite loop.
            In any case do not update it if the protocol was imported. */
        if ((((protocolSection->repsIdx*(repetitionSb->value()-1)) == 0) ||
                (repsNumOrig > 0)) &&
                !importedCursorsFlag) {
            repetitionSb->setValue(protocolSection->repsIdx+1); /*!< Human reference is 1-based, not 0-based */
            repetitionIdx = repetitionSb->value();
        }

        if (repetitionCb->currentIndex() == RepetitionLast) {
            repetitionCb->setCurrentIndex(RepetitionOne);

        }

        lastRepItem->setFlags(lastRepItem->flags() & ~Qt::ItemIsEnabled);
    }

    if ((repetitionCb->currentIndex() == RepetitionLast) && (repetitionSb->value() != protocolSection->repsNum)) {
        repetitionCb->setCurrentIndex(RepetitionOne);
    }

    sweepSb->setMaximum(protocolSection->sweepsNum);
    sweepSb->setValue(protocolSection->sweepIdx+1); /*!< Human reference is 1-based, not 0-based */
    sweepIdx = sweepSb->value();

    if (locationCb->currentIndex() == LocationFromStart) {
        this->setXValue(protocolSection->startingTime+locationDelay);

    } else {
        this->setXValue(protocolSection->endingTime-locationDelay);
    }
}

bool ProtocolCursor::updateSection(ProtocolSection * section, ProtocolType_t protocolType) {
    if (sweepCb->currentIndex() == SweepLast) {
        sweepIdx = section->sweepsNum;
    }

    if (repetitionCb->currentIndex() == RepetitionLast) {
        protocolSection = section->getSectionByItem(itemIdx, 0, sweepIdx-1);
        if (protocolSection != nullptr) {
            repetitionIdx = protocolSection->repsNum;

        } else {
            return false;
        }
    }

    protocolSection = section->getSectionByItem(itemIdx, repetitionIdx-1, sweepIdx-1);

    if (protocolSection != nullptr) {
        double offset;
        if (locationCb->currentIndex() == LocationFromStart) {
            offset = locationSb->value();

        } else {
            offset = protocolSection->duration()-locationSb->value();
        }
        this->setSection(protocolSection, offset, protocolType);
        return true;

    } else {
        return false;
    }
}

void ProtocolCursor::setCursorIdx(int idx) {
    cursorIdx = idx;
    QwtText markerText = marker->label();
    markerText.setText(QString::number(cursorIdx));
    marker->setLabel(markerText);
    propertyDialog->setWindowTitle(QString("Cursor %1 properties").arg(cursorIdx));
}

int ProtocolCursor::getItemIdx() {
    return itemIdx;
}

ProtocolType_t ProtocolCursor::getProtocolType() {
    return protocolType;
}

int ProtocolCursor::getLocationType() {
    return locationType;
}

double ProtocolCursor::getLocationDelay() {
    return locationDelay;
}

int ProtocolCursor::getRepetitionType() {
    return repetitionType;
}

int ProtocolCursor::getRepetitionIdx() {
    return repetitionIdx-1;
}

int ProtocolCursor::getRepetitionsNum() {
    return protocolSection->repsNum;
}

int ProtocolCursor::getLoopIdx() {
    return protocolSection->loopIdx;
}

int ProtocolCursor::getSweepType() {
    return sweepType;
}

int ProtocolCursor::getSweepIdx() {
    return sweepIdx-1;
}

int ProtocolCursor::getSweepsNum() {
    return protocolSection->sweepsNum;
}

int ProtocolCursor::getTriggerType() {
    return triggerType;
}

int ProtocolCursor::getTriggerId() {
    return triggerId-1;
}

void ProtocolCursor::setXValue(double x) {
    marker->setXValue(x);
}

double ProtocolCursor::getXValue() {
    return marker->xValue();
}

double ProtocolCursor::getOffset() {
    if (locationCb->currentIndex() == LocationFromStart) {
        return locationDelay;

    } else {
        return protocolSection->duration()-locationDelay;
    }
}

double ProtocolCursor::getOffset(int sectionRepsIdx, int sectionSweepIdx) {
    if (locationCb->currentIndex() == LocationFromStart) {
        return locationDelay;

    } else {
        return protocolSection->getSectionByItem(itemIdx, sectionRepsIdx, sectionSweepIdx)->duration()-locationDelay;
    }
}

double ProtocolCursor::getAbsoluteOffset() {
    if (locationCb->currentIndex() == LocationFromStart) {
        return protocolSection->startingTime+locationDelay;

    } else {
        return protocolSection->endingTime-locationDelay;
    }
}

double ProtocolCursor::getAbsoluteOffset(int sectionRepsIdx, int sectionSweepIdx) {
    ProtocolSection * sectionRep0 = protocolSection->getSectionByItem(itemIdx, 0, 0);
    /*! 3 different cases for... */
    if (sectionRep0->repsNum == 0) {
        /*! Infinite loops in gap free protocols: in infinite sequences only two protocol sections are created,
         *  so the absolute time at a specific repetition must be calculated based on the sequence period */
        ProtocolSection * sectionRep1 = protocolSection->getSectionByItem(itemIdx, 1, 0);
        double period = sectionRep1->startingTime-sectionRep0->startingTime;
        double repetitionsDelay = period*((double)sectionRepsIdx);
        if (locationCb->currentIndex() == LocationFromStart) {
            return sectionRep0->startingTime+repetitionsDelay+locationDelay;

        } else {
            return sectionRep0->endingTime+repetitionsDelay-locationDelay;
        }

    } else if (sectionRep0->sweepsNum > 1) {
        /*! Episodic protocols: the duration of previous sweeps must be added to each cursor */
        ProtocolSection * sectionRep1 = protocolSection->getSectionByItem(-1, 0, 1);
        ProtocolSection * sectionRep2 = protocolSection->getSectionByItem(-1, 0, 2);

        double sweepsDelay = 0.0;
        if (sectionRep1 != nullptr && sectionRep2 != nullptr) {
            double firstSweepDuration = sectionRep1->endingTime;
            double deltaSweepDuration = sectionRep2->endingTime-sectionRep1->endingTime;
            sweepsDelay = firstSweepDuration*((double)sectionSweepIdx)+deltaSweepDuration*(double)((sectionSweepIdx*(sectionSweepIdx-1))/2);
        }

        if (locationCb->currentIndex() == LocationFromStart) {
            return protocolSection->getSectionByItem(itemIdx, sectionRepsIdx, sectionSweepIdx)->startingTime+sweepsDelay+locationDelay;

        } else {
            return protocolSection->getSectionByItem(itemIdx, sectionRepsIdx, sectionSweepIdx)->endingTime+sweepsDelay-locationDelay;
        }

    } else {
        /*! Gap free non infinite protocols */
        if (locationCb->currentIndex() == LocationFromStart) {
            return protocolSection->getSectionByItem(itemIdx, sectionRepsIdx, sectionSweepIdx)->startingTime+locationDelay;

        } else {
            return protocolSection->getSectionByItem(itemIdx, sectionRepsIdx, sectionSweepIdx)->endingTime-locationDelay;
        }
    }
}

bool ProtocolCursor::precedes(ProtocolCursor * cursor) {
    bool ret = true;

    if (this->getSweepType() == SweepAll) {
        if (cursor->getSweepType() == SweepAll) {
            for (int sweepIter = 0; sweepIter < this->getSweepsNum(); sweepIter++) {
                if (!(this->precedesLoopLevel(cursor, sweepIter))) {
                    ret = false;
                    break;
                }
            }

        } else {
            if (cursor->getSweepIdx() < this->getSweepsNum()-1) {
                ret = false;

            } else {
                ret = this->precedesLoopLevel(cursor, cursor->getSweepIdx());
            }
        }

    } else {
        if (cursor->getSweepType() == SweepAll) {
            if (this->getSweepIdx() > 0) {
                ret = false;

            } else {
                ret = this->precedesLoopLevel(cursor, this->getSweepIdx());
            }

        } else {
            if (this->getSweepIdx() < cursor->getSweepIdx()) {
                ret = true;

            } else if (this->getSweepIdx() > cursor->getSweepIdx()) {
                ret = false;

            } else {
                ret = this->precedesLoopLevel(cursor, cursor->getSweepIdx());
            }
        }
    }

    return ret;
}

bool ProtocolCursor::sameRepetitions(ProtocolCursor * cursor) {
    bool ret = true;
    if (this->getSweepType() == SweepAll) {
        if (cursor->getSweepType() == SweepAll) {
            ret = this->sameRepetitionsLoopLevel(cursor);

        } else {
            if (this->getSweepsNum() > 1) {
                ret = false;

            } else {
                ret = this->sameRepetitionsLoopLevel(cursor);
            }
        }

    } else {
        if (cursor->getSweepType() == SweepAll) {
            if (cursor->getSweepsNum() > 1) {
                ret = false;

            } else {
                ret = this->sameRepetitionsLoopLevel(cursor);
            }

        } else {
            if (this->getSweepIdx() != cursor->getSweepIdx()) {
                ret = false;

            } else {
                ret = this->sameRepetitionsLoopLevel(cursor);
            }
        }
    }
    return ret;
}

YAML::Cursor ProtocolCursor::getYamlCursor() {
    YAML::Cursor yamlCursor;

    yamlCursor.xvalue = marker->xValue();
    yamlCursor.itemidx = itemIdx;
    yamlCursor.locationtype = (YAML::CursorLocationType_t)locationType;
    yamlCursor.locationdelay = locationDelay;
    yamlCursor.repetitiontype = (YAML::CursorRepetitionType_t)repetitionType;
    yamlCursor.repetitionidx = repetitionIdx;
    yamlCursor.sweeptype = (YAML::CursorSweepType_t)sweepType;
    yamlCursor.sweepidx = sweepIdx;
    yamlCursor.triggertype = (YAML::CursorTriggerType_t)triggerType;
    yamlCursor.triggeridx = triggerId;

    return yamlCursor;
}

void ProtocolCursor::setCursorFromYaml(const YAML::Cursor &yamlCursor) {
    locationType = (LocationType_t)yamlCursor.locationtype;
    locationDelay = yamlCursor.locationdelay;
    repetitionType = (RepetitionType_t)yamlCursor.repetitiontype;
    repetitionIdx = yamlCursor.repetitionidx;
    sweepType = (SweepType_t)yamlCursor.sweeptype;
    sweepIdx = yamlCursor.sweepidx;
    triggerType = (TriggerType_t)yamlCursor.triggertype;
    triggerId = yamlCursor.triggeridx;

    locationCb->setCurrentIndex(locationType);
    locationSb->setValue(locationDelay);
    repetitionCb->setCurrentIndex(repetitionType);
    repetitionSb->setValue(repetitionIdx);
    sweepCb->setCurrentIndex(sweepType);
    sweepSb->setValue(sweepIdx);
    triggerCb->setCurrentIndex(triggerType);
    triggerSb->setValue(triggerId);
}

void ProtocolCursor::initializePropertyDialog() {
    propertyDialog = new QDialog();

    propertyDialog->setWindowFlags(Qt::WindowTitleHint);
    propertyDialog->setWindowTitle(QString("Cursor %1 properties").arg(cursorIdx));
    propertyDialog->setWindowModality(Qt::ApplicationModal);

    QVBoxLayout * propertyLo = new QVBoxLayout();

    propertyDialog->setLayout(propertyLo);

    /*! Location settings */
    QGroupBox * locationGb = new QGroupBox("Location");
    propertyLo->addWidget(locationGb);
    QHBoxLayout * locationLo = new QHBoxLayout;
    locationGb->setLayout(locationLo);

    locationCb = new QComboBox();
    locationCb->addItem("from item start");
    locationCb->addItem("to item end");
    locationLo->addWidget(locationCb);

    RangedMeasurement_t timeRange;
//    \todo FCON Ficona potresti gentilmente aggiungere la linea seguente thx
//    mDev->getTimeProtocolRange(timeRange);
    timeRange.convertValues(UnitPfxMilli);

    locationSb = new QDoubleSpinBox();
    locationSb->setRange(0.0, std::numeric_limits <double> ::max());
    locationSb->setValue(locationDelay);
    locationSb->setDecimals(timeRange.decimals());
    locationSb->setSingleStep(timeRange.step);
    locationSb->setMinimumWidth(70);
    locationLo->addWidget(locationSb);

    locationLo->addWidget(new QLabel("ms"));

    locationLo->setStretch(0, 1);
    locationLo->setStretch(1, 1);
    locationLo->setStretch(2, 0);

    connect(locationCb, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int) {
        if (protocolSection != nullptr) {
            locationSb->setValue(protocolSection->duration()-locationSb->value());
        }
    });
    emit locationCb->currentIndexChanged(LocationFromStart);

    /*! Repetitions settings */
    QGroupBox * repetitionGb = new QGroupBox("Repetitions");
    propertyLo->addWidget(repetitionGb);
    QHBoxLayout * repetitionLo = new QHBoxLayout;
    repetitionGb->setLayout(repetitionLo);

    repetitionCb = new QComboBox();
    repetitionCb->addItem("All");
    repetitionCb->addItem("Last");
    this->getLastRepComboItem();
    repetitionCb->addItem("Select one");
    repetitionLo->addWidget(repetitionCb);

    repetitionLo->addWidget(new QLabel("#"));

    repetitionSb = new QSpinBox();
    repetitionSb->setRange(1, std::numeric_limits <int> ::max());
    repetitionSb->setValue(repetitionIdx);
    repetitionSb->setSingleStep(1);
    repetitionSb->setMinimumWidth(70);
    repetitionSb->setEnabled(false);
    repetitionLo->addWidget(repetitionSb);

    repetitionLo->setStretch(0, 1);
    repetitionLo->setStretch(1, 0);
    repetitionLo->setStretch(2, 1);

    connect(repetitionCb, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int idx) {
        repetitionSb->setEnabled(idx == RepetitionOne);
        if (protocolSection != nullptr) {
            if (idx == RepetitionLast) {
                repetitionSb->setValue(protocolSection->repsNum);
            }

            this->setMaxDuration();
        }
    });
    repetitionCb->setCurrentIndex(RepetitionAll);
    repetitionType = RepetitionAll;

    connect(repetitionSb, QOverload <int> ::of(&QSpinBox::valueChanged), this, &ProtocolCursor::onRepetitionIdxChanged);

    /*! Sweeps settings */
    sweepGb = new QGroupBox("Sweeps");
    propertyLo->addWidget(sweepGb);
    QHBoxLayout * sweepLo = new QHBoxLayout;
    sweepGb->setLayout(sweepLo);

    sweepCb = new QComboBox();
    sweepCb->addItem("All");
    sweepCb->addItem("Last");
    sweepCb->addItem("Select one");
    sweepLo->addWidget(sweepCb);

    sweepLo->addWidget(new QLabel("#"));

    sweepSb = new QSpinBox();
    sweepSb->setRange(1, std::numeric_limits <int> ::max());
    sweepSb->setValue(sweepIdx);
    sweepSb->setSingleStep(1);
    sweepSb->setMinimumWidth(70);
    sweepSb->setEnabled(false);
    sweepLo->addWidget(sweepSb);

    sweepLo->setStretch(0, 1);
    sweepLo->setStretch(1, 0);
    sweepLo->setStretch(2, 1);

    connect(sweepCb, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int idx) {
        sweepSb->setEnabled(idx == SweepOne);
        if (protocolSection != nullptr) {
            if (idx == SweepLast) {
                sweepSb->setValue(protocolSection->sweepsNum);
            }
            this->setMaxDuration();
        }
    });
    sweepCb->setCurrentIndex(SweepAll);
    sweepType = SweepAll;

    connect(sweepSb, QOverload <int> ::of(&QSpinBox::valueChanged), this, &ProtocolCursor::onSweepIdxChanged);

    /*! Trigger settings */
    QGroupBox * triggerGb = new QGroupBox("Digital trigger output");
    propertyLo->addWidget(triggerGb);
    QHBoxLayout * triggerLo = new QHBoxLayout;
    triggerGb->setLayout(triggerLo);

    triggerCb = new QComboBox();
    triggerCb->addItem("None");
    triggerCb->addItem("Rising");
    triggerCb->addItem("Falling");
    triggerLo->addWidget(triggerCb);

    triggerLo->addWidget(new QLabel("#"));

//    \todo FCON add getTriggerOutput in ModelDevice?
    unsigned int triggersNum = 10;
//    commLib->getOutputTriggersNum(triggersNum);
    triggerSb = new QSpinBox();
    triggerSb->setRange(1, (int)triggersNum);
    triggerSb->setValue(triggerId);
    triggerSb->setSingleStep(1);
    triggerSb->setMinimumWidth(70);
    triggerLo->addWidget(triggerSb);

    triggerLo->setStretch(0, 1);
    triggerLo->setStretch(1, 0);
    triggerLo->setStretch(2, 1);

    connect(triggerCb, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int idx) {
        triggerSb->setEnabled(idx != 0);
    });
    emit triggerCb->currentIndexChanged(TriggerNone);
    triggerType = TriggerNone;

    /*! Navigation */
    QGroupBox * navigationGb = new QGroupBox("Navigation panel");
    propertyLo->addWidget(navigationGb);
    QHBoxLayout * navigationLo = new QHBoxLayout;
    navigationGb->setLayout(navigationLo);

    QPushButton * goToFirstBtn = new QPushButton("<<");
    goToFirstBtn->setMinimumWidth(40);
    navigationLo->addWidget(goToFirstBtn);
    connect(goToFirstBtn, &QPushButton::clicked, this, [=] () {
        this->onAcceptPropertyDialog();
        emit cursorOpenPropertiesRequest(0);
    });

    QPushButton * goToPreviousBtn = new QPushButton("<");
    goToPreviousBtn->setMinimumWidth(40);
    navigationLo->addWidget(goToPreviousBtn);
    connect(goToPreviousBtn, &QPushButton::clicked, this, [=] () {
        this->onAcceptPropertyDialog();
        emit cursorOpenPropertiesRequest(cursorIdx-2);
    });

    QPushButton * goToNextBtn = new QPushButton(">");
    goToNextBtn->setMinimumWidth(40);
    navigationLo->addWidget(goToNextBtn);
    connect(goToNextBtn, &QPushButton::clicked, this, [=] () {
        this->onAcceptPropertyDialog();
        emit cursorOpenPropertiesRequest(cursorIdx);
    });

    QPushButton * goToLastBtn = new QPushButton(">>");
    goToLastBtn->setMinimumWidth(40);
    navigationLo->addWidget(goToLastBtn);
    connect(goToLastBtn, &QPushButton::clicked, this, [=] () {
        this->onAcceptPropertyDialog();
        emit cursorOpenPropertiesRequest(-1);
    });

    /*! OK / Cancel buttons */
    QHBoxLayout * okCancBtnLo = new QHBoxLayout;
    propertyLo->addLayout(okCancBtnLo);

    QPushButton * okButton = new QPushButton("OK");
    okButton->setCheckable(false);
    okCancBtnLo->addWidget(okButton);
    connect(okButton, &QPushButton::clicked, this, &ProtocolCursor::onAcceptPropertyDialog);
    okButton->setDefault(true);

    QPushButton * cancButton = new QPushButton("Cancel");
    cancButton->setCheckable(false);
    okCancBtnLo->addWidget(cancButton);
    connect(cancButton, &QPushButton::clicked, this, &ProtocolCursor::onRejectPropertyDialog);

    QPushButton * deleteButton = new QPushButton("DELETE");
    deleteButton->setCheckable(false);
    okCancBtnLo->addWidget(deleteButton);
    connect(deleteButton, &QPushButton::clicked, this, [=] () {
        this->onAcceptPropertyDialog();
        emit cursorDeleteRequest(cursorIdx-1);
    });

    propertyDialog->setFixedWidth(okCancBtnLo->sizeHint().width());

    connect(propertyDialog, &QDialog::accepted, this, &ProtocolCursor::propertiesAccepted);
}

void ProtocolCursor::getLastRepComboItem() {
    QStandardItemModel * model = qobject_cast <QStandardItemModel *> (repetitionCb->model());
    lastRepItem = model->item(RepetitionLast);
}

void ProtocolCursor::setMaxDuration() {
    double maxDuration;
    if (((protocolType == ProtocolTypeGapfree) && (repetitionCb->currentIndex() == RepetitionAll)) ||
            ((protocolType == ProtocolTypeEpisodic) && (sweepCb->currentIndex() == SweepAll))) {
        maxDuration = protocolSection->minDuration();

    } else {
        maxDuration = protocolSection->duration();
    }

    locationSb->setMaximum(maxDuration);
}

bool ProtocolCursor::precedesLoopLevel(ProtocolCursor * cursor, int sweepIter) {
    bool ret = true;
    if (this->getLoopIdx() < cursor->getLoopIdx()) {
        ret = true;

    } else if (this->getLoopIdx() > cursor->getLoopIdx()) {
        ret = false;

    } else {
        if (this->getRepetitionType() == RepetitionAll) {
            if (cursor->getRepetitionType() == RepetitionAll) {
                /*! It may happen that one cursor is set to RepetitionAll, but it  doesn't belong to the loop.
                    In this case its repetitionsNum is 1 and its section either comes necessarily before any of the
                    repetitions of the other cursor if its in the loop, or it's sufficient to compare it with
                    only the first repetition if the other cursor comes before the loop too. */
                for (int repetitionIter = 0; repetitionIter < qMax(1, qMin(this->getRepetitionsNum(), cursor->getRepetitionsNum())); repetitionIter++) {
                    if (!(this->precedesItemLevel(cursor, sweepIter, repetitionIter))) {
                        ret = false;
                        break;
                    }
                }

            } else {
                if (this->getRepetitionsNum() == 0) {
                    ret = false;

                } else {
                    if (cursor->getRepetitionIdx() < this->getRepetitionsNum()-1) {
                        ret = false;

                    } else if (cursor->getRepetitionIdx() > this->getRepetitionsNum()-1) {
                        /*! If this is set to RepetitionAll but it doesn't belong to the loop cursor may have
                        a higher repetitionIdx */
                        ret = true;

                    } else {
                        ret = this->precedesItemLevel(cursor, sweepIter, cursor->getRepetitionIdx());
                    }
                }
            }

        } else {
            if (cursor->getRepetitionType() == RepetitionAll) {
                if (this->getRepetitionIdx() > 0) {
                    ret = false;

                } else {
                    ret = this->precedesItemLevel(cursor, sweepIter, this->getRepetitionIdx());
                }

            } else {
                if (this->getRepetitionIdx() < cursor->getRepetitionIdx()) {
                    ret = true;

                } else if (this->getRepetitionIdx() > cursor->getRepetitionIdx()) {
                    ret = false;

                } else {
                    ret = this->precedesItemLevel(cursor, sweepIter, cursor->getRepetitionIdx());
                }
            }
        }
    }
    return ret;
}

bool ProtocolCursor::precedesItemLevel(ProtocolCursor * cursor, int sweepIter, int repetitionIter) {
    bool ret = true;
    if (this->getItemIdx() < cursor->getItemIdx()) {
        ret = true;

    } else if (this->getItemIdx() > cursor->getItemIdx()) {
        ret = false;

    } else if (this->getOffset(repetitionIter, sweepIter) <= cursor->getOffset(repetitionIter, sweepIter)) {
        ret = true;

    } else {
        ret = false;
    }
    return ret;
}

bool ProtocolCursor::sameRepetitionsLoopLevel(ProtocolCursor * cursor) {
    bool ret = true;
    if (this->getLoopIdx() != cursor->getLoopIdx()) {
        if (((this->getRepetitionType() == RepetitionAll) && (this->getRepetitionsNum() != 1)) ||
                ((cursor->getRepetitionType() == RepetitionAll) && (cursor->getRepetitionsNum() != 1))) {
            /*! If this and cursor belong to different loops they can only have 1 repetition each, otherwise return false */
            ret = false;

        } else {
            ret = true;
        }

    } else {
        if (this->getRepetitionType() == RepetitionAll) {
            if (cursor->getRepetitionType() == RepetitionAll) {
                /*! Even if this and cursor belong to the same loop one may be within the loop and the other before */
                if (this->getRepetitionsNum() != cursor->getRepetitionsNum()) {
                    ret = false;

                } else {
                    ret = true;
                }

            } else {
                if (this->getRepetitionsNum() != 1) {
                    ret = false;

                } else {
                    ret = true;
                }
            }

        } else {
            if (cursor->getRepetitionType() == RepetitionAll) {
                if (cursor->getRepetitionsNum() != 1) {
                    ret = false;

                } else {
                    ret = true;
                }

            } else {
                ret = true;
            }
        }
    }
    return ret;
}

void ProtocolCursor::onRepetitionIdxChanged(int repsIdx) {
    if (protocolSection != nullptr) {
        if (protocolSection->repsNum == 0) {
            repsIdx = std::min(repsIdx, 2);
        }

        protocolSection = protocolSection->getSectionByItem(
                    protocolSection->itemIdx, repsIdx-1, protocolSection->sweepIdx);
        this->setMaxDuration();
    }
}

void ProtocolCursor::onSweepIdxChanged(int sweepIdx) {
    if (protocolSection != nullptr) {
        protocolSection = protocolSection->getSectionByItem(
                    protocolSection->itemIdx, protocolSection->repsIdx, sweepIdx-1);
        this->setMaxDuration();
    }
}

void ProtocolCursor::onAcceptPropertyDialog() {
    locationType = locationCb->currentIndex();
    locationDelay = locationSb->value();

    repetitionType = repetitionCb->currentIndex();
    repetitionIdx = repetitionSb->value();

    sweepType = sweepCb->currentIndex();
    sweepIdx = sweepSb->value();

    triggerType = triggerCb->currentIndex();
    triggerId = triggerSb->value();

    if (locationType == LocationFromStart) {
        this->setXValue(protocolSection->startingTime+locationDelay);

    } else {
        this->setXValue(protocolSection->endingTime-locationDelay);
    }

    if (propertyDialog->isVisible()) {
        propertyDialog->accept();
    }
}

void ProtocolCursor::onRejectPropertyDialog() {
    protocolSection = protocolSectionOrig;

    locationCb->setCurrentIndex(locationType);
    emit locationCb->setCurrentIndex(locationType);
    locationSb->setValue(locationDelay);

    repetitionCb->setCurrentIndex(repetitionType);
    emit repetitionCb->setCurrentIndex(repetitionType);
    repetitionSb->setValue(repetitionIdx);

    sweepCb->setCurrentIndex(sweepType);
    emit sweepCb->setCurrentIndex(sweepType);
    sweepSb->setValue(sweepIdx);

    triggerCb->setCurrentIndex(triggerType);
    emit triggerCb->setCurrentIndex(triggerType);
    triggerSb->setValue(triggerId);

    if (propertyDialog->isVisible()) {
        propertyDialog->reject();
    }
}

TriggerCursor::TriggerCursor(int id, bool high, double delay, bool terminator) :
    id(id),
    high(high),
    delay(delay),
    terminator(terminator) {

}

TriggerCursor::TriggerCursor(const TriggerCursor * cursor) :
    id(cursor->id),
    high(cursor->high),
    delay(cursor->delay),
    terminator(cursor->terminator) {

}

bool TriggerCursor::operator < (const TriggerCursor &a) const {
    return delay < a.delay;
}

TriggerTerminator::TriggerTerminator() :
    TriggerCursor(0, false, 0.0, true) {

}
