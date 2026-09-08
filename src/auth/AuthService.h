#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "User.h"
#include <QString>
#include <QList>
#include <optional>

class AuthService
{
public:
    static AuthService& instance();

    bool signup(const QString& fullName,
                const QString& username,
                const QString& email,
                const QString& password,
                QString& errorMessage);

    bool login(const QString& username,
               const QString& password,
               QString& errorMessage);

    void logout();

    bool isLoggedIn() const;
    std::optional<User> currentUser() const;

private:
    AuthService() = default;
    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;

    User m_currentUser;
    bool m_loggedIn = false;

    // Temporary in-memory storage (will be replaced by MySQL)
    QList<User> m_users;

    // Helpers
    QString generateSalt() const;
    QString hashPassword(const QString& password, const QString& salt) const;
    QString createPasswordHash(const QString& password) const;       // returns salt$hash
    bool verifyPassword(const QString& password, const QString& stored) const;

    bool usernameExists(const QString& username) const;
    bool emailExists(const QString& email) const;
};

#endif // AUTHSERVICE_H