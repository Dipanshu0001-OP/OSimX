#ifndef OSIMX_USER_H
#define OSIMX_USER_H

#include <QString>
#include <QDateTime>

struct User {
    int id = -1;
    QString fullName;
    QString username;
    QString email;
    QString passwordHash;
    QDateTime createdAt;
};

#endif
