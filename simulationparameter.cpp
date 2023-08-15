#include "simulationparameter.h"

SimulationParameter::SimulationParameter()
    : QStandardItem()
{}

SimulationParameter::SimulationParameter(QString name, QString _typename, QVariantMap jsonMap)
    : QStandardItem()
    , name(name)
    , _typename(_typename)
    , json_value(jsonMap)
{
    setText(name);
}

int SimulationParameter::type() {
  if (_typename == "bdm::Param") {
    return ItemType::Param;
  }
  if (_typename == "bdm::SimParam") {
    return ItemType::SimParam;
  }
  return ItemType::UserType;
}
