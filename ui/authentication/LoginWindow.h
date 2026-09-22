#ifndef OSIMX_LOGIN_WINDOW_H
#define OSIMX_LOGIN_WINDOW_H

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QLabel;

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget* parent = nullptr);

signals:
    void loginSuccessful();
    void signupRequested();

private slots:
    void submitLogin();

private:
    QLineEdit* usernameEdit = nullptr;
    QLineEdit* passwordEdit = nullptr;
    QPushButton* loginButton = nullptr;
    QLabel* statusLabel = nullptr;

    void setupUI();
    void setStatus(const QString& message, bool error);
};

#endif
