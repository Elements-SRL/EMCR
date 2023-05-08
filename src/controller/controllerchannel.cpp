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

void ControllerChannel::onApplyTurnChannelOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->turnChannelsOn(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setOn(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyTurnStimulusOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->enableStimulus(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setInStimActive(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: stimulus on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyTurnDocOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues){
    this->mDev->getMessageDispatcher()->digitalOffsetCompensation(channelIndexes, onValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setCompensatingDoc(onValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: DOC on/off status:" << onValues[i] << "\n";
    }
}

void ControllerChannel::onApplyVoltageHoldValues(vector<uint16_t> channelIndexes, vector<Measurement_t> vHoldValues){
    this->mDev->getMessageDispatcher()->setVoltageHoldTuner(channelIndexes, vHoldValues, true);
    for (int i = 0; i < channelIndexes.size(); i++){
        this->mDev->getChannels()[channelIndexes[i]]->setVhold(vHoldValues[i]);
        qDebug() << "[Channel " << channelIndexes[i] << "]: vHold set:" << vHoldValues[i].value << "\n";
    }
}

void ControllerChannel::onCompensationApplied(vector<uint16_t> channelIndexes, vector<bool> cfastEn, vector<bool> cslowRsEn, vector<bool> rsCpEn, vector<bool> rsPgEn, vector<double> cfastValues, vector<double> cslowValues, vector<double> rsValues, vector<double> rsCpValues, vector<double> rsPgValues, vector<uint16_t> rsBWValueIdxs){
    int ongoingClampingMode = mDev->getOngoingClampingModality();
    vector<std::vector<double>> compValueMatrix;
    vector<RangedMeasurement> cfastFeatures;
    vector<RangedMeasurement> cslowFeatures;
    vector<RangedMeasurement> rsFeatures;
    vector<RangedMeasurement> rsCpFeatures;
    vector<RangedMeasurement> rsPgFeatures;

    compValueMatrix.resize(channelIndexes.size(), std::vector<double>(MessageDispatcher::CompensationUserParamsNum));
    cfastFeatures.resize(channelIndexes.size());
    cslowFeatures.resize(channelIndexes.size());
    rsFeatures.resize(channelIndexes.size());
    rsCpFeatures.resize(channelIndexes.size());
    rsPgFeatures.resize(channelIndexes.size());

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

    /*! \todo MPAC discriminare tra voltage e current clamp*/
    if(ongoingClampingMode == E384CL_VOLTAGE_CLAMP_MODE){
        this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_CpVc, cfastValues, true);
    } else if(ongoingClampingMode == E384CL_ZERO_CURRENT_CLAMP_MODE || ongoingClampingMode == E384CL_CURRENT_CLAMP_MODE) {
        this->mDev->getMessageDispatcher()->setCompValues(channelIndexes, MessageDispatcher::U_CpCc, cfastValues, true);
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
    if(ongoingClampingMode == E384CL_VOLTAGE_CLAMP_MODE){
        this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_CpVc, cfastFeatures, defaultParamValue);
    } else if(ongoingClampingMode == E384CL_ZERO_CURRENT_CLAMP_MODE || ongoingClampingMode == E384CL_CURRENT_CLAMP_MODE) {
        this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_CpCc, cfastFeatures, defaultParamValue);
    } else {
        /*! \todo MPAC ancora da fare*/
    }
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_Cm, cslowFeatures, defaultParamValue);
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_Rs, rsFeatures, defaultParamValue);
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_RsCp, rsCpFeatures, defaultParamValue);
    this->mDev->getMessageDispatcher()->getCompFeatures(MessageDispatcher::U_RsPg, rsPgFeatures, defaultParamValue);

    emit sigCompValuesDispatched(compValueMatrix, cfastFeatures, cslowFeatures, rsFeatures, rsCpFeatures, rsPgFeatures);



}


