#include "SimulationRepository.h"
#include "AuthService.h"
#include "DatabaseManager.h"

#include <QJsonDocument>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

SimulationRepository& SimulationRepository::instance()
{
    static SimulationRepository instance;
    return instance;
}

bool SimulationRepository::save(const QString& module,
                                const QString& algorithm,
                                const QString& summary,
                                const QJsonObject& inputData,
                                const QJsonObject& outputData,
                                const QJsonObject& payload,
                                QString& errorMessage)
{
    const auto user = AuthService::instance().currentUser();
    if (!user) {
        errorMessage = "Sign in before saving a simulation.";
        return false;
    }

    if (!DatabaseManager::instance().isOpen() && !DatabaseManager::instance().open()) {
        errorMessage = "Could not open the OSimX database.";
        return false;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        INSERT INTO simulations (user_id, module, algorithm, summary, input_data, output_data, payload)
        VALUES (:user_id, :module, :algorithm, :summary, :input_data, :output_data, :payload)
    )");
    query.bindValue(":user_id", user->id);
    query.bindValue(":module", module.left(64));
    query.bindValue(":algorithm", algorithm.left(64));
    query.bindValue(":summary", summary.left(255));
    query.bindValue(":input_data", QString::fromUtf8(QJsonDocument(inputData).toJson(QJsonDocument::Compact)));
    query.bindValue(":output_data", QString::fromUtf8(QJsonDocument(outputData).toJson(QJsonDocument::Compact)));
    query.bindValue(":payload", QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)));

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }
    m_lastInsertId = query.lastInsertId().toInt();
    return true;
}

bool SimulationRepository::save(const QString& module,
                                const QString& algorithm,
                                const QString& summary,
                                const QJsonObject& payload,
                                QString& errorMessage)
{
    // Legacy: extract input and output from the payload if possible
    QJsonObject inputData;
    QJsonObject outputData;

    if (payload.contains("processes"))
        inputData["processes"] = payload["processes"];
    if (payload.contains("timeQuantum"))
        inputData["timeQuantum"] = payload["timeQuantum"];
    if (payload.contains("blockSizes"))
        inputData["blockSizes"] = payload["blockSizes"];
    if (payload.contains("referenceString"))
        inputData["referenceString"] = payload["referenceString"];
    if (payload.contains("frameCount"))
        inputData["frameCount"] = payload["frameCount"];
    if (payload.contains("requestQueue"))
        inputData["requestQueue"] = payload["requestQueue"];
    if (payload.contains("initialHead"))
        inputData["initialHead"] = payload["initialHead"];
    if (payload.contains("maxTrack"))
        inputData["maxTrack"] = payload["maxTrack"];

    if (payload.contains("metrics"))
        outputData["metrics"] = payload["metrics"];
    if (payload.contains("timeline"))
        outputData["timeline"] = payload["timeline"];
    if (payload.contains("sequence"))
        outputData["sequence"] = payload["sequence"];
    if (payload.contains("steps"))
        outputData["steps"] = payload["steps"];
    if (payload.contains("decisionLog"))
        outputData["decisionLog"] = payload["decisionLog"];

    inputData["module"] = payload.value("module");
    inputData["algorithm"] = payload.value("algorithm");
    outputData["summary"] = summary;

    return save(module, algorithm, summary, inputData, outputData, payload, errorMessage);
}

int SimulationRepository::lastInsertId() const
{
    return m_lastInsertId;
}

QList<SavedSimulation> SimulationRepository::listForCurrentUser(QString& errorMessage) const
{
    QList<SavedSimulation> rows;
    const auto user = AuthService::instance().currentUser();
    if (!user || !DatabaseManager::instance().isOpen())
        return rows;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT id, module, algorithm, summary, created_at
        FROM simulations
        WHERE user_id = :user_id
        ORDER BY created_at DESC, id DESC
    )");
    query.bindValue(":user_id", user->id);
    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return rows;
    }

    while (query.next()) {
        SavedSimulation row;
        row.id = query.value("id").toInt();
        row.module = query.value("module").toString();
        row.algorithm = query.value("algorithm").toString();
        row.summary = query.value("summary").toString();
        row.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        if (!row.createdAt.isValid())
            row.createdAt = query.value("created_at").toDateTime();
        rows.append(row);
    }
    return rows;
}

bool SimulationRepository::load(int id, SavedSimulation& simulation, QString& errorMessage) const
{
    const auto user = AuthService::instance().currentUser();
    if (!user) {
        errorMessage = "Sign in to view saved simulations.";
        return false;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT id, module, algorithm, summary, input_data, output_data, payload, created_at
        FROM simulations
        WHERE id = :id AND user_id = :user_id
    )");
    query.bindValue(":id", id);
    query.bindValue(":user_id", user->id);
    if (!query.exec() || !query.next()) {
        errorMessage = query.lastError().text().isEmpty() ? "That simulation was not found." : query.lastError().text();
        return false;
    }

    simulation.id = query.value("id").toInt();
    simulation.module = query.value("module").toString();
    simulation.algorithm = query.value("algorithm").toString();
    simulation.summary = query.value("summary").toString();
    simulation.inputData = query.value("input_data").toString();
    simulation.outputData = query.value("output_data").toString();
    simulation.payload = query.value("payload").toString();
    simulation.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    if (!simulation.createdAt.isValid())
        simulation.createdAt = query.value("created_at").toDateTime();
    return true;
}

bool SimulationRepository::remove(int id, QString& errorMessage)
{
    const auto user = AuthService::instance().currentUser();
    if (!user) {
        errorMessage = "Sign in to remove a simulation.";
        return false;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM simulations WHERE id = :id AND user_id = :user_id");
    query.bindValue(":id", id);
    query.bindValue(":user_id", user->id);
    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }
    return true;
}
