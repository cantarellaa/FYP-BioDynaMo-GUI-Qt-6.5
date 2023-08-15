#ifndef SIMULATIONPARAMETER_H
#define SIMULATIONPARAMETER_H

#include <QJsonValue>
#include <QStandardItem>

class SimulationParameter : public QStandardItem {
private:
  QString name;
  QString _typename;
  QVariantMap json_value;

public:
    SimulationParameter();
    SimulationParameter(QString name, QString _typename, QVariantMap jsonMap);

    enum ItemType { Type = 0, UserType = 1000, Param = 1001, SimParam = 1002 };

    int type();
};

#endif // SIMULATIONPARAMETER_H
