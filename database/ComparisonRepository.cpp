#include "ComparisonRepository.h"
#include "AuthService.h"
#include "DatabaseManager.h"

#include <QJsonDocument>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

ComparisonRepository& ComparisonRepository::instance()
{
    static ComparisonRepository instance;
    return instance;
}

bool ComparisonRepository::save(const QString& module,
                                const QString& title,
                                const QJsonObject& comparisonData,
                                const QList<ComparisonItem>& items,
                                QString& errorMessage)
{
    const auto user = AuthService::instance().currentUser();
    if (!user) {
        errorMessage = "Sign in before saving a comparison.";
        return false;
    }

    if (!DatabaseManager::instance().isOpen() && !DatabaseManager::instance().open()) {
        errorMessage = "Could not open the OSimX database.";
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().database();

    // Begin transaction
    db.transaction();

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO comparisons (user_id, module, title, comparison_data)
        VALUES (:user_id, :module, :title, :comparison_data)
    )");
    query.bindValue(":user_id", user->id);
    query.bindValue(":module", module.left(64));
    query.bindValue(":title", title.left(255));
    query.bindValue(":comparison_data",
        QString::fromUtf8(QJsonDocument(comparisonData).toJson(QJsonDocument::Compact)));

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        db.rollback();
        return false;
    }

    const int comparisonId = query.lastInsertId().toInt();

    for (const auto& item : items) {
        QSqlQuery itemQuery(db);
        itemQuery.prepare(R"(
            INSERT INTO comparison_items (comparison_id, algorithm, metrics_json, simulation_id)
            VALUES (:comparison_id, :algorithm, :metrics_json, :simulation_id)
        )");
        itemQuery.bindValue(":comparison_id", comparisonId);
        itemQuery.bindValue(":algorithm", item.algorithm.left(64));
        itemQuery.bindValue(":metrics_json",
            QString::fromUtf8(QJsonDocument(item.metrics).toJson(QJsonDocument::Compact)));
        itemQuery.bindValue(":simulation_id",
            item.simulationId > 0 ? QVariant(item.simulationId) : QVariant());

        if (!itemQuery.exec()) {
            errorMessage = itemQuery.lastError().text();
            db.rollback();
            return false;
        }
    }

    db.commit();
    return true;
}

QList<SavedComparison> ComparisonRepository::listForCurrentUser(QString& errorMessage) const
{
    QList<SavedComparison> rows;
    const auto user = AuthService::instance().currentUser();
    if (!user || !DatabaseManager::instance().isOpen())
        return rows;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT id, module, title, created_at
        FROM comparisons
        WHERE user_id = :user_id
        ORDER BY created_at DESC, id DESC
    )");
    query.bindValue(":user_id", user->id);
    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return rows;
    }

    while (query.next()) {
        SavedComparison row;
        row.id = query.value("id").toInt();
        row.module = query.value("module").toString();
        row.title = query.value("title").toString();
        row.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        if (!row.createdAt.isValid())
            row.createdAt = query.value("created_at").toDateTime();
        rows.append(row);
    }
    return rows;
}

bool ComparisonRepository::load(int id, SavedComparison& comparison, QString& errorMessage) const
{
    const auto user = AuthService::instance().currentUser();
    if (!user) {
        errorMessage = "Sign in to view saved comparisons.";
        return false;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT id, module, title, comparison_data, created_at
        FROM comparisons
        WHERE id = :id AND user_id = :user_id
    )");
    query.bindValue(":id", id);
    query.bindValue(":user_id", user->id);
    if (!query.exec() || !query.next()) {
        errorMessage = query.lastError().text().isEmpty()
            ? "That comparison was not found." : query.lastError().text();
        return false;
    }

    comparison.id = query.value("id").toInt();
    comparison.module = query.value("module").toString();
    comparison.title = query.value("title").toString();
    comparison.comparisonData = query.value("comparison_data").toString();
    comparison.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    if (!comparison.createdAt.isValid())
        comparison.createdAt = query.value("created_at").toDateTime();

    // Load comparison items
    QSqlQuery itemQuery(DatabaseManager::instance().database());
    itemQuery.prepare(R"(
        SELECT id, algorithm, metrics_json, simulation_id
        FROM comparison_items
        WHERE comparison_id = :comparison_id
    )");
    itemQuery.bindValue(":comparison_id", comparison.id);
    if (itemQuery.exec()) {
        while (itemQuery.next()) {
            ComparisonItem item;
            item.id = itemQuery.value("id").toInt();
            item.algorithm = itemQuery.value("algorithm").toString();
            const auto doc = QJsonDocument::fromJson(
                itemQuery.value("metrics_json").toString().toUtf8());
            if (doc.isObject())
                item.metrics = doc.object();
            item.simulationId = itemQuery.value("simulation_id").toInt();
            comparison.items.append(item);
        }
    }
    return true;
}

bool ComparisonRepository::remove(int id, QString& errorMessage)
{
    const auto user = AuthService::instance().currentUser();
    if (!user) {
        errorMessage = "Sign in to remove a comparison.";
        return false;
    }

    // Items are cascaded by FK
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM comparisons WHERE id = :id AND user_id = :user_id");
    query.bindValue(":id", id);
    query.bindValue(":user_id", user->id);
    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }
    return true;
}
