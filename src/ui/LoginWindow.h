#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSuccessful();

private slots:
    void switchToLogin();
    void switchToSignup();
    void showEmailSignupForm();
    void onGoogleClicked();
    void onLoginClicked();
    void onCreateAccountClicked();

private:
    QLabel *titleLabel;
    QLabel *switchLabel;
    QPushButton *switchButton;

    // Login form
    QLineEdit *loginUsername;
    QLineEdit *loginPassword;
    QPushButton *loginButton;
    QPushButton *googleLoginButton;

    // Signup choice
    QPushButton *googleSignupButton;
    QPushButton *emailSignupButton;

    // Full email signup form
    QLineEdit *fullNameEdit;
    QLineEdit *signupUsername;
    QLineEdit *emailEdit;
    QLineEdit *signupPassword;
    QLineEdit *repeatPassword;
    QPushButton *createAccountButton;
    QPushButton *backButton;

    QStackedWidget *formStack;
    bool isLoginMode = true;

    void setupUI();
    void updateMode();
    QWidget* createLoginForm();
    QWidget* createSignupChoiceForm();
    QWidget* createEmailSignupForm();
};

#endif // LOGINWINDOW_H