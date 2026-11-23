#ifndef OSIMX_COMPARISON_REPOSITORY_H
#define OSIMX_COMPARISON_REPOSITORY_H

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

struct ComparisonItem {
    int id = -1;
    QString algorithm;
    QJsonObject metrics;
    int simulationId = -1;
};

struct SavedComparison {
    int id = -1;
    QString module;
    QString title;
    QString comparisonData;
    QDateTime createdAt;
    QList<ComparisonItem> items;
};

class ComparisonRepository {
public:
    static ComparisonRepository& instance();

    bool save(const QString& module,
              const QString& title,
              const QJsonObject& comparisonData,
              const QList<ComparisonItem>& items,
              QString& errorMessage);

    QList<SavedComparison> listForCurrentUser(QString& errorMessage) const;
    bool load(int id, SavedComparison& comparison, QString& errorMessage) const;
    bool remove(int id, QString& errorMessage);

private:
    ComparisonRepository() = default;
};

#endif
