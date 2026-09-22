#ifndef OSIMX_AUTH_SERVICE_H
#define OSIMX_AUTH_SERVICE_H

#include "models/User.h"
#include <QString>
#include <optional>

class AuthService {
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

    QString generateSalt() const;
    QString hashPassword(const QString& password, const QString& salt) const;
    QString createPasswordHash(const QString& password) const;
    bool verifyPassword(const QString& password, const QString& stored) const;
    bool usernameExists(const QString& username) const;
    bool emailExists(const QString& email) const;
};

#endif
