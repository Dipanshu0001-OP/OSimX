#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool open();
    void close();
    bool isOpen() const;

    QSqlDatabase database() const;

private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H