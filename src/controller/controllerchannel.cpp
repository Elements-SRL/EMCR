#include "controllerchannel.h"

ControllerChannel::ControllerChannel(ModelDevice * mDev) :
    mDev(mDev)
{

}

void ControllerChannel::setModelDevice(ModelDevice * mDev){
    this->mDev = mDev;
}

void ControllerChannel::onSingleChannelClicked(uint16_t changedChannelIndexes, bool newChannelState){
    this->mDev->getChannels()[changedChannelIndexes]->setSelected(newChannelState);
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState){
    ModelBoard* boardToUpdate = this->mDev->getBoards()[changedBoardIndex];
    uint16_t numOfChannelsToUpadate = boardToUpdate->getChannelsOnBoard().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        boardToUpdate->getChannelsOnBoard()[i]->setSelected(newChannelState);
    }
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onOneRowClicked(uint16_t changedRowIndex, bool newChannelState){
    uint16_t numOfBoardsToUpadate = this->mDev->getBoards().size();
    for(uint16_t i = 0; i < numOfBoardsToUpadate; i++){
        this->mDev->getBoards()[i]->getChannelsOnBoard()[changedRowIndex]->setSelected(newChannelState);
    }
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onAllChannelsClicked(bool newChannelState){
    uint16_t numOfChannelsToUpadate = this->mDev->getChannels().size();
    for(uint16_t i = 0; i < numOfChannelsToUpadate; i++){
        this->mDev->getChannels()[i]->setSelected(newChannelState);
    }
    emit sigSelectedChannelsUpdated();
}

void ControllerChannel::onApplyTurnChannelOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setOn(onValues[i]);
//        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyTurnStimulusOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->enableStimulus(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setInStimActive(onValues[i]);
//        qDebug() << "[Channel " << channelIndexes[i] << "]: stimulus on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyTurnDocOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    this->mDev->getMessageDispatcher()->digitalOffsetCompensation(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setCompensatingDoc(onValues[i]);
//        qDebug() << "[Channel " << channelIndexes[i] << "]: DOC on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues){
    if (mDev->getOngoingClampingModality() == ClampingModality_t::VOLTAGE_CLAMP) {
        this->mDev->getMessageDispatcher()->setVoltageHoldTuner(channelIndexes, holdValues, true);
        for (int i = 0; i < channelIndexes.size(); i++){
            this->mDev->getChannels()[channelIndexes[i]]->setVhold(holdValues[i]);
//                    qDebug() << "[Channel " << channelIndexes[i] << "]: vHold set:" << holdValues[i].value << "\n";
        }

    } else {
        this->mDev->getMessageDispatcher()->setCurrentHoldTuner(channelIndexes, holdValues, true);
        for (int i = 0; i < channelIndexes.size(); i++){
            this->mDev->getChannels()[channelIndexes[i]]->setChold(holdValues[i]);
//                    qDebug() << "[Channel " << channelIndexes[i] << "]: cHold set:" << holdValues[i].value << "\n";
        }
    }
}

void ControllerChannel::onCompensationApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues){
    int ongoingClampingMode = mDev->getOngoingClampingModality();
    std::vector<std::vector<double>> compValueMatrix;
    std::vector<RangedMeasurement> cfastFeatures;
    std::vector<RangedMeasurement> cslowFeatures;
    std::vector<RangedMeasurement> rsFeatures;
    std::vector<RangedMeasurement> rsCpFeatures;
    std::vector<RangedMeasurement> rsPgFeatures;
    std::vector<RangedMeasurement> ccCfastFeatures;

    compValueMatrix.resize(channelIndexes.size(), std::vector<double>(MessageDispatcher::CompensationUserParamsNum));
    cfastFeatures.resize(channelIndexes.size());
    cslowFeatures.resize(channelIndexes.size());
    rsFeatures.resize(channelIndexes.size());
    rsCpFeatures.resize(channelIndexes.size());
    rsPgFeatures.resize(channelIndexes.size());
    ccCfastFeatures.resize(channelIndexes.size());

    for(int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setCompensatingCfast(cfastEn[i]);
        this->mDev->getChannels()[channelIndexes[i]]->setCompensatingCslowRs(cslowRsEn[i]);
        this->mDev->getChannels()[channelIndexes[i]]->setCompensatingRsCp(rsCpEn[i]);
        this->mDev->getChannels()[channelIndexes[i]]->setCompensatingRsPg(rsPgEn[i]);
    }

    this->mDev->getMessageDispatcher()->enableCompensation(channelIndexes, MessageDispatcher::CompCfast, cfastEn, true);
    this->mDev->getMessageDispatcher()->enableCompensation(channelIndexes, MessageDispatcher::CompCslow, cslowRsEn, true);
    this->mDev->getMessageDispatcher()->enableCompensation(channelIndexes, MessageDispatcher::CompRsCorr, rsCpEn, true);
    this->mDev->getMessageDispatcher()->enableCompensation(channelIndexes, MessageDispatcher::CompRsPred, rsPgEn, true);
    this->mDev->getMessageDispatcher()->enableCompensation(channelIndexes, MessageDispatcher::CompCcCfast, ccCfastEn, true);

    /*! \todo MPAC discriminare tra voltage e current clamp*/
    if(ongoingClampingMode == ClampingModality_t::VOLTAGE_CLAMP){
        this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_CpVc, cfastValues, true);
    } else if(ongoingClampingMode == ClampingModality_t::ZERO_CURRENT_CLAMP || ongoingClampingMode == ClampingModality_t::CURRENT_CLAMP) {
        this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_CpCc, ccCfastValues, true);
    } else {
        /*! \todo MPAC ancora da fare*/
    }
    this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_Cm, cslowValues, true);
    this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_Rs, rsValues, true);
    this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_RsCp, rsCpValues, true);
    this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_RsPg, rsPgValues, true);
    this->mDev->getMessageDispatcher()->setCompOptions(channelIndexes, MessageDispatcher::CompRsCorr, rsBWValueIdxs, true);

    this->mDev->getMessageDispatcher()->getCompValueMatrix(compValueMatrix);

    /*! \todo MPAC discriminare tra voltage e current clamp*/
    double defaultParamValue;
    if(ongoingClampingMode == ClampingModality_t::VOLTAGE_CLAMP){
        this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_CpVc, cfastFeatures, defaultParamValue);
    } else if(ongoingClampingMode == ClampingModality_t::ZERO_CURRENT_CLAMP || ongoingClampingMode == ClampingModality_t::CURRENT_CLAMP) {
        this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_CpCc, ccCfastFeatures, defaultParamValue);
    } else {
        /*! \todo MPAC ancora da fare*/
    }
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_Cm, cslowFeatures, defaultParamValue);
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_Rs, rsFeatures, defaultParamValue);
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_RsCp, rsCpFeatures, defaultParamValue);
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_RsPg, rsPgFeatures, defaultParamValue);

    emit sigCompValuesDispatched(compValueMatrix, cfastFeatures, cslowFeatures, rsFeatures, rsCpFeatures, rsPgFeatures, ccCfastFeatures);



}


