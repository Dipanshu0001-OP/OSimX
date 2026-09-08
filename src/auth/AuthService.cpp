#include "AuthService.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QUuid>
#include <QDebug>

AuthService& AuthService::instance()
{
    static AuthService instance;
    return instance;
}

bool AuthService::signup(const QString& fullName,
                         const QString& username,
                         const QString& email,
                         const QString& password,
                         QString& errorMessage)
{
    if (fullName.trimmed().isEmpty() ||
        username.trimmed().isEmpty() ||
        email.trimmed().isEmpty() ||
        password.isEmpty()) {
        errorMessage = "All fields are required.";
        return false;
    }

    if (password.length() < 6) {
        errorMessage = "Password must be at least 6 characters.";
        return false;
    }

    if (!DatabaseManager::instance().isOpen()) {
        if (!DatabaseManager::instance().open()) {
            errorMessage = "Could not connect to database.";
            return false;
        }
    }

    if (usernameExists(username)) {
        errorMessage = "Username already exists.";
        return false;
    }

    if (emailExists(email)) {
        errorMessage = "Email already registered.";
        return false;
    }

    QString passwordHash = createPasswordHash(password);

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        INSERT INTO users (full_name, username, email, password_hash)
        VALUES (:full_name, :username, :email, :password_hash)
    )");

    query.bindValue(":full_name", fullName.trimmed());
    query.bindValue(":username", username.trimmed());
    query.bindValue(":email", email.trimmed());
    query.bindValue(":password_hash", passwordHash);

    if (!query.exec()) {
        errorMessage = "Signup failed: " + query.lastError().text();
        return false;
    }

    // Auto-login after successful signup
    return login(username, password, errorMessage);
}

bool AuthService::login(const QString& username,
                        const QString& password,
                        QString& errorMessage)
{
    if (!DatabaseManager::instance().isOpen()) {
        if (!DatabaseManager::instance().open()) {
            errorMessage = "Could not connect to database.";
            return false;
        }
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT id, full_name, username, email, password_hash
        FROM users
        WHERE username = :username
    )");
    query.bindValue(":username", username.trimmed());

    if (!query.exec()) {
        errorMessage = "Login failed: " + query.lastError().text();
        return false;
    }

    if (!query.next()) {
        errorMessage = "Username not found.";
        return false;
    }

    QString storedHash = query.value("password_hash").toString();

    if (!verifyPassword(password, storedHash)) {
        errorMessage = "Incorrect password.";
        return false;
    }

    // Update login_time
    QSqlQuery updateQuery(DatabaseManager::instance().database());
    updateQuery.prepare("UPDATE users SET login_time = NOW() WHERE id = :id");
    updateQuery.bindValue(":id", query.value("id").toInt());
    updateQuery.exec();

    // Set current user
    m_currentUser.id = query.value("id").toInt();
    m_currentUser.fullName = query.value("full_name").toString();
    m_currentUser.username = query.value("username").toString();
    m_currentUser.email = query.value("email").toString();
    m_currentUser.passwordHash = storedHash;

    m_loggedIn = true;
    return true;
}

void AuthService::logout()
{
    m_loggedIn = false;
    m_currentUser = User();
}

bool AuthService::isLoggedIn() const
{
    return m_loggedIn;
}

std::optional<User> AuthService::currentUser() const
{
    if (m_loggedIn)
        return m_currentUser;
    return std::nullopt;
}

// ===================== Password Helpers =====================

QString AuthService::generateSalt() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(32);
}

QString AuthService::hashPassword(const QString& password, const QString& salt) const
{
    QByteArray data = (salt + password).toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

QString AuthService::createPasswordHash(const QString& password) const
{
    QString salt = generateSalt();
    QString hash = hashPassword(password, salt);
    return salt + "$" + hash;
}

bool AuthService::verifyPassword(const QString& password, const QString& stored) const
{
    QStringList parts = stored.split('$');
    if (parts.size() != 2)
        return false;

    QString salt = parts[0];
    QString storedHash = parts[1];
    QString computedHash = hashPassword(password, salt);

    return computedHash == storedHash;
}

bool AuthService::usernameExists(const QString& username) const
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id FROM users WHERE username = :username");
    query.bindValue(":username", username.trimmed());
    if (query.exec() && query.next())
        return true;
    return false;
}

bool AuthService::emailExists(const QString& email) const
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id FROM users WHERE email = :email");
    query.bindValue(":email", email.trimmed());
    if (query.exec() && query.next())
        return true;
    return false;
}