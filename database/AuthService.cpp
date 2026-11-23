#include "AuthService.h"
#include "DatabaseManager.h"
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

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
    if (fullName.trimmed().isEmpty() || username.trimmed().isEmpty() ||
        email.trimmed().isEmpty() || password.isEmpty()) {
        errorMessage = "Please complete every field.";
        return false;
    }

    if (password.length() < 6) {
        errorMessage = "Password must contain at least 6 characters.";
        return false;
    }

    const QRegularExpression emailPattern(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)");
    if (!emailPattern.match(email.trimmed()).hasMatch()) {
        errorMessage = "Enter a valid email address.";
        return false;
    }

    if (!DatabaseManager::instance().isOpen() && !DatabaseManager::instance().open()) {
        errorMessage = "Could not connect to the OSimX database.";
        return false;
    }

    if (usernameExists(username)) {
        errorMessage = "That username is already in use.";
        return false;
    }

    if (emailExists(email)) {
        errorMessage = "That email address is already registered.";
        return false;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        INSERT INTO users (full_name, username, email, password_hash)
        VALUES (:full_name, :username, :email, :password_hash)
    )");
    query.bindValue(":full_name", fullName.trimmed());
    query.bindValue(":username", username.trimmed());
    query.bindValue(":email", email.trimmed());
    query.bindValue(":password_hash", createPasswordHash(password));

    if (!query.exec()) {
        errorMessage = "Could not create the account: " + query.lastError().text();
        return false;
    }

    return login(username, password, errorMessage);
}

bool AuthService::login(const QString& username,
                        const QString& password,
                        QString& errorMessage)
{
    if (username.trimmed().isEmpty() || password.isEmpty()) {
        errorMessage = "Enter both your username and password.";
        return false;
    }

    if (!DatabaseManager::instance().isOpen() && !DatabaseManager::instance().open()) {
        errorMessage = "Could not connect to the OSimX database.";
        return false;
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
        errorMessage = "No account was found for that username.";
        return false;
    }

    const QString storedHash = query.value("password_hash").toString();
    if (!verifyPassword(password, storedHash)) {
        errorMessage = "The password is incorrect.";
        return false;
    }

    const int userId = query.value("id").toInt();

    // Update last login timestamp
    QSqlQuery updateQuery(DatabaseManager::instance().database());
    updateQuery.prepare("UPDATE users SET login_time = CURRENT_TIMESTAMP WHERE id = :id");
    updateQuery.bindValue(":id", userId);
    updateQuery.exec();

    // Record in login_history table
    recordLogin(userId);

    m_currentUser.id = userId;
    m_currentUser.fullName = query.value("full_name").toString();
    m_currentUser.username = query.value("username").toString();
    m_currentUser.email = query.value("email").toString();
    m_currentUser.passwordHash = storedHash;
    m_loggedIn = true;
    return true;
}

void AuthService::recordLogin(int userId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO login_history (user_id) VALUES (:user_id)");
    query.bindValue(":user_id", userId);
    if (!query.exec())
        qDebug() << "Could not record login history:" << query.lastError().text();
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
    return m_loggedIn ? std::optional<User>(m_currentUser) : std::nullopt;
}

QList<LoginRecord> AuthService::loginHistory(QString& errorMessage) const
{
    QList<LoginRecord> records;
    const auto user = currentUser();
    if (!user || !DatabaseManager::instance().isOpen())
        return records;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        SELECT id, logged_in_at
        FROM login_history
        WHERE user_id = :user_id
        ORDER BY logged_in_at DESC
        LIMIT 50
    )");
    query.bindValue(":user_id", user->id);
    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return records;
    }

    while (query.next()) {
        LoginRecord r;
        r.id = query.value("id").toInt();
        r.loggedInAt = QDateTime::fromString(query.value("logged_in_at").toString(), Qt::ISODate);
        if (!r.loggedInAt.isValid())
            r.loggedInAt = query.value("logged_in_at").toDateTime();
        records.append(r);
    }
    return records;
}

QString AuthService::generateSalt() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(32);
}

QString AuthService::hashPassword(const QString& password, const QString& salt) const
{
    const QByteArray data = (salt + password).toUtf8();
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

QString AuthService::createPasswordHash(const QString& password) const
{
    const QString salt = generateSalt();
    return salt + "$" + hashPassword(password, salt);
}

bool AuthService::verifyPassword(const QString& password, const QString& stored) const
{
    const QStringList parts = stored.split('$');
    return parts.size() == 2 && hashPassword(password, parts[0]) == parts[1];
}

bool AuthService::usernameExists(const QString& username) const
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id FROM users WHERE username = :username LIMIT 1");
    query.bindValue(":username", username.trimmed());
    return query.exec() && query.next();
}

bool AuthService::emailExists(const QString& email) const
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id FROM users WHERE email = :email LIMIT 1");
    query.bindValue(":email", email.trimmed());
    return query.exec() && query.next();
}
