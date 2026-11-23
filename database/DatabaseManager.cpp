#include "DatabaseManager.h"

#include <QDir>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDebug>
#include <QVariant>

namespace {

QString envOr(const char* key, const QString& fallback)
{
    const QByteArray value = qgetenv(key);
    return value.isEmpty() ? fallback : QString::fromUtf8(value);
}

} // namespace

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::open()
{
    if (m_db.isOpen())
        return true;

    const bool requestMySql = envOr("OSIMX_DB_BACKEND", "sqlite").trimmed().compare("mysql", Qt::CaseInsensitive) == 0;
    if ((requestMySql && openMySql()) || openSqlite()) {
        if (!ensureSchema()) {
            qDebug() << "Schema setup failed:" << m_db.lastError().text();
            close();
            return false;
        }
        migrateIfNeeded();
        qDebug() << "Database ready:" << m_description;
        return true;
    }

    qDebug() << "Database connection failed.";
    return false;
}

bool DatabaseManager::openMySql()
{
    if (!QSqlDatabase::isDriverAvailable("QMYSQL"))
        return false;

    const QString host = envOr("OSIMX_DB_HOST", "127.0.0.1");
    const int port = envOr("OSIMX_DB_PORT", "3306").toInt();
    const QString name = envOr("OSIMX_DB_NAME", "osimx_db");
    if (!QRegularExpression("^[A-Za-z0-9_]+$").match(name).hasMatch())
        return false;
    const QString user = envOr("OSIMX_DB_USER", "root");
    const QString password = envOr("OSIMX_DB_PASSWORD", "");

    {
        QSqlDatabase bootstrap = QSqlDatabase::contains("osimx_bootstrap")
            ? QSqlDatabase::database("osimx_bootstrap")
            : QSqlDatabase::addDatabase("QMYSQL", "osimx_bootstrap");
        bootstrap.setHostName(host);
        bootstrap.setPort(port);
        bootstrap.setUserName(user);
        bootstrap.setPassword(password);
        bootstrap.setDatabaseName("mysql");
        if (bootstrap.open()) {
            QSqlQuery query(bootstrap);
            query.exec(QString("CREATE DATABASE IF NOT EXISTS `%1` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci").arg(name));
            bootstrap.close();
        }
    }
    QSqlDatabase::removeDatabase("osimx_bootstrap");

    m_db = QSqlDatabase::contains("osimx_connection")
        ? QSqlDatabase::database("osimx_connection")
        : QSqlDatabase::addDatabase("QMYSQL", "osimx_connection");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(name);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (!m_db.open()) {
        qDebug() << "MySQL unavailable, falling back to local SQLite:" << m_db.lastError().text();
        m_db.close();
        return false;
    }

    m_sqlite = false;
    m_description = QString("MySQL %1@%2/%3").arg(user, host, name);
    return true;
}

bool DatabaseManager::openSqlite()
{
    if (m_db.isValid() && m_db.isOpen())
        m_db.close();
    if (QSqlDatabase::contains("osimx_connection")) {
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase("osimx_connection");
    }

    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
        return false;

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    const QString path = dir + "/osimx.sqlite";

    m_db = QSqlDatabase::addDatabase("QSQLITE", "osimx_connection");
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        qDebug() << "SQLite open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON");

    m_sqlite = true;
    m_description = QString("SQLite %1").arg(path);
    return true;
}

bool DatabaseManager::execStatements(const QStringList& statements)
{
    QSqlQuery query(m_db);
    for (const QString& statement : statements) {
        if (!query.exec(statement)) {
            qDebug() << "SQL failed:" << statement << query.lastError().text();
            return false;
        }
    }
    return true;
}

bool DatabaseManager::ensureSchema()
{
    if (m_sqlite) {
        return execStatements({
            R"(CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                full_name TEXT NOT NULL,
                username TEXT NOT NULL UNIQUE,
                email TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                created_at TEXT NOT NULL DEFAULT (datetime('now')),
                login_time TEXT NULL
            ))",
            R"(CREATE TABLE IF NOT EXISTS login_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                logged_in_at TEXT NOT NULL DEFAULT (datetime('now')),
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            ))",
            "CREATE INDEX IF NOT EXISTS idx_login_history_user ON login_history(user_id, logged_in_at)",
            R"(CREATE TABLE IF NOT EXISTS simulations (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                module TEXT NOT NULL,
                algorithm TEXT NOT NULL,
                summary TEXT NOT NULL,
                input_data TEXT NOT NULL DEFAULT '{}',
                output_data TEXT NOT NULL DEFAULT '{}',
                payload TEXT NOT NULL,
                created_at TEXT NOT NULL DEFAULT (datetime('now')),
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            ))",
            "CREATE INDEX IF NOT EXISTS idx_simulations_user_time ON simulations(user_id, created_at)",
            "CREATE INDEX IF NOT EXISTS idx_simulations_module ON simulations(user_id, module)",
            R"(CREATE TABLE IF NOT EXISTS comparisons (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                module TEXT NOT NULL,
                title TEXT NOT NULL,
                comparison_data TEXT NOT NULL,
                created_at TEXT NOT NULL DEFAULT (datetime('now')),
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            ))",
            "CREATE INDEX IF NOT EXISTS idx_comparisons_user_time ON comparisons(user_id, created_at)",
            R"(CREATE TABLE IF NOT EXISTS comparison_items (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                comparison_id INTEGER NOT NULL,
                algorithm TEXT NOT NULL,
                metrics_json TEXT NOT NULL,
                simulation_id INTEGER NULL,
                FOREIGN KEY (comparison_id) REFERENCES comparisons(id) ON DELETE CASCADE,
                FOREIGN KEY (simulation_id) REFERENCES simulations(id) ON DELETE SET NULL
            ))",
            "CREATE INDEX IF NOT EXISTS idx_comparison_items ON comparison_items(comparison_id)"
        });
    }

    // MySQL schema
    const bool usersOk = execStatements({
        R"(CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            full_name VARCHAR(100) NOT NULL,
            username VARCHAR(50) NOT NULL UNIQUE,
            email VARCHAR(100) NOT NULL UNIQUE,
            password_hash VARCHAR(255) NOT NULL,
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            login_time DATETIME NULL
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)"
    });
    if (!usersOk)
        return false;

    // Safely add columns that may not exist yet
    QSqlQuery alter(m_db);
    alter.exec("ALTER TABLE users ADD COLUMN login_time DATETIME NULL");
    alter.exec("ALTER TABLE users ADD COLUMN created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP");

    const bool loginHistoryOk = execStatements({
        R"(CREATE TABLE IF NOT EXISTS login_history (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            logged_in_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            CONSTRAINT fk_login_history_user
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            INDEX idx_login_history_user (user_id, logged_in_at)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)"
    });
    if (!loginHistoryOk)
        return false;

    const bool simOk = execStatements({
        R"(CREATE TABLE IF NOT EXISTS simulations (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            module VARCHAR(64) NOT NULL,
            algorithm VARCHAR(64) NOT NULL,
            summary VARCHAR(255) NOT NULL,
            input_data LONGTEXT NOT NULL DEFAULT ('{}'),
            output_data LONGTEXT NOT NULL DEFAULT ('{}'),
            payload LONGTEXT NOT NULL,
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            CONSTRAINT fk_simulations_user
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            INDEX idx_simulations_user_time (user_id, created_at),
            INDEX idx_simulations_module (user_id, module)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)"
    });
    if (!simOk)
        return false;

    const bool compOk = execStatements({
        R"(CREATE TABLE IF NOT EXISTS comparisons (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            module VARCHAR(64) NOT NULL,
            title VARCHAR(255) NOT NULL,
            comparison_data LONGTEXT NOT NULL,
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            CONSTRAINT fk_comparisons_user
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            INDEX idx_comparisons_user_time (user_id, created_at)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)"
    });
    if (!compOk)
        return false;

    const bool compItemsOk = execStatements({
        R"(CREATE TABLE IF NOT EXISTS comparison_items (
            id INT AUTO_INCREMENT PRIMARY KEY,
            comparison_id INT NOT NULL,
            algorithm VARCHAR(64) NOT NULL,
            metrics_json LONGTEXT NOT NULL,
            simulation_id INT NULL,
            CONSTRAINT fk_comparison_items_comparison
                FOREIGN KEY (comparison_id) REFERENCES comparisons(id) ON DELETE CASCADE,
            CONSTRAINT fk_comparison_items_simulation
                FOREIGN KEY (simulation_id) REFERENCES simulations(id) ON DELETE SET NULL,
            INDEX idx_comparison_items (comparison_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)"
    });
    if (!compItemsOk)
        return false;

    return true;
}

bool DatabaseManager::migrateIfNeeded()
{
    // Add input_data and output_data columns to old simulations tables
    QSqlQuery q(m_db);
    q.exec("ALTER TABLE simulations ADD COLUMN input_data LONGTEXT NOT NULL DEFAULT '{}'");
    q.exec("ALTER TABLE simulations ADD COLUMN output_data LONGTEXT NOT NULL DEFAULT '{}'");
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

bool DatabaseManager::usingSqlite() const
{
    return m_sqlite;
}

QString DatabaseManager::backendDescription() const
{
    return m_description;
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}
