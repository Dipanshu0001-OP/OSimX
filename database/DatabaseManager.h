#ifndef OSIMX_DATABASE_MANAGER_H
#define OSIMX_DATABASE_MANAGER_H

#include <QSqlDatabase>

class DatabaseManager {
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

#endif
