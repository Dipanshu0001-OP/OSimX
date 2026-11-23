#ifndef OSIMX_SIMULATION_REPOSITORY_H
#define OSIMX_SIMULATION_REPOSITORY_H

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

struct SavedSimulation {
    int id = -1;
    QString module;
    QString algorithm;
    QString summary;
    QString inputData;
    QString outputData;
    QString payload;
    QDateTime createdAt;
};

class SimulationRepository {
public:
    static SimulationRepository& instance();

    bool save(const QString& module,
              const QString& algorithm,
              const QString& summary,
              const QJsonObject& inputData,
              const QJsonObject& outputData,
              const QJsonObject& payload,
              QString& errorMessage);

    // Legacy overload: puts the whole payload as both input/output
    bool save(const QString& module,
              const QString& algorithm,
              const QString& summary,
              const QJsonObject& payload,
              QString& errorMessage);

    QList<SavedSimulation> listForCurrentUser(QString& errorMessage) const;
    bool load(int id, SavedSimulation& simulation, QString& errorMessage) const;
    bool remove(int id, QString& errorMessage);

    int lastInsertId() const;

private:
    SimulationRepository() = default;
    int m_lastInsertId = -1;
};

#endif
