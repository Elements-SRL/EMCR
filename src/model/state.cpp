#include "model/state.h"

#include <QString>
#include <QStringList>

QStringList getListOfTriggerStates(YAML::State state, QStringList * list){
    for (auto tt : YAML::triggerTypeStrings){
        list->append(QString::fromStdString(tt));
    }
    return * list;
}
