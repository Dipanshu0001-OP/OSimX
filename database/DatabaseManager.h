#ifndef OSIMX_DATABASE_MANAGER_H
#define OSIMX_DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QStringList>

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool open();
    void close();
    bool isOpen() const;
    bool usingSqlite() const;
    QString backendDescription() const;
    QSqlDatabase database() const;

private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool openMySql();
    bool openSqlite();
    bool ensureSchema();
    bool execStatements(const QStringList& statements);
    bool migrateIfNeeded();

    QSqlDatabase m_db;
    bool m_sqlite = false;
    QString m_description;
};

#endif
