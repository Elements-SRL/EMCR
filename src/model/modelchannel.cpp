#include "modelchannel.h"

ModelChannel::ModelChannel()
{

}

ModelChannel::~ModelChannel(){

}

uint16_t ModelChannel::getId(){
    return this->id;
}

bool ModelChannel::isOn(){
    return this->on;
}
bool ModelChannel::isCompensating(){
    return this->compensating;
}

bool ModelChannel::isInStimActive(){
    return this->inStimActive;
}

bool ModelChannel::isSelected(){
    return this->selected;
}

Measurement_t ModelChannel::getVhold(){
    return this->vHold;
}

Measurement_t ModelChannel::getVdoc(){
    return this->vDoc;
}


void ModelChannel::setId(uint16_t id){
    this->id = id;
}

void ModelChannel::setOn(bool on){
    this->on = on;
}

void ModelChannel::setCompensating(bool compensating){
    this->compensating = compensating;
}

void ModelChannel::setInStimActive(bool active){
    this->inStimActive = active;
}

void ModelChannel::setSelected(bool selected){
    this->selected = selected;
}

void ModelChannel::setVhold(Measurement_t vHold){
    this->vHold = vHold;
}

void ModelChannel::setVdoc(Measurement_t vDoc){
    this->vDoc = vDoc;
}
