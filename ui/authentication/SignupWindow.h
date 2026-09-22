#ifndef OSIMX_SIGNUP_WINDOW_H
#define OSIMX_SIGNUP_WINDOW_H

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QLabel;

class SignupWindow : public QWidget {
    Q_OBJECT
public:
    explicit SignupWindow(QWidget* parent = nullptr);

signals:
    void signupSuccessful();
    void loginRequested();

private slots:
    void submitSignup();

private:
    QLineEdit* fullNameEdit = nullptr;
    QLineEdit* usernameEdit = nullptr;
    QLineEdit* emailEdit = nullptr;
    QLineEdit* passwordEdit = nullptr;
    QLineEdit* confirmEdit = nullptr;
    QPushButton* createButton = nullptr;
    QLabel* statusLabel = nullptr;

    void setupUI();
    void setStatus(const QString& message, bool error);
};

#endif
