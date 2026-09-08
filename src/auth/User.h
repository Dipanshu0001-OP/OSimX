#ifndef USER_H
#define USER_H

#include <QString>

struct User {
    int id = -1;
    QString fullName;
    QString username;
    QString email;
    QString passwordHash;   // Will store "salt$hash"
};

#endif // USER_H