#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::open()
{
    if (m_db.isOpen())
        return true;

    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName("127.0.0.1");      // localhost
    m_db.setPort(3306);
    m_db.setDatabaseName("osimx_db");
    m_db.setUserName("root");           // change if needed
    m_db.setPassword("saloni2107@sql");               // put your MySQL password here

    if (!m_db.open()) {
        qDebug() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Database connected successfully.";
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}