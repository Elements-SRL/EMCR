#include "model/state.h"

#include <QString>
#include <QStringList>

QStringList getListOfTriggerStates(QStringList * list){
    for (auto tt : YAML::triggerTypeStrings){
        list->append(QString::fromStdString(tt));
    }
    return * list;
}

YAML::TriggerType getTriggerTypeFromString(std::string toFind){
    return (YAML::TriggerType)(std::find(YAML::triggerTypeStrings.begin(), YAML::triggerTypeStrings.end(), toFind)-YAML::triggerTypeStrings.begin());
}
