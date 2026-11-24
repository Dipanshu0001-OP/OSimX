
#include "LoginWindow.h"
#include "AuthLayout.h"
#include "database/AuthService.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void LoginWindow::setupUI()
{
    setWindowTitle("OSimX · Sign in");

    auto* layout = AuthLayout::create(this, 490);

    auto* eyebrow = AuthLayout::makeText(
        "YOUR OSIMX WORKSPACE", "authEyebrow");

    auto* heading = AuthLayout::makeText(
        "Welcome back", "authTitle");

    auto* subheading = AuthLayout::makeText(
        "Sign in to continue to your OSimX workspace.",
        "authSubtitle");

    layout->addWidget(eyebrow);
    layout->addSpacing(5);
    layout->addWidget(heading);
    layout->addWidget(subheading);
    layout->addSpacing(18);

    auto* usernameLabel = AuthLayout::makeText(
        "Username", "fieldLabel");

    usernameEdit = new QLineEdit;
    usernameEdit->setObjectName("authField");
    usernameEdit->setPlaceholderText(
        "Enter your username");
    usernameEdit->setClearButtonEnabled(true);

    layout->addWidget(usernameLabel);
    layout->addWidget(usernameEdit);
    layout->addSpacing(8);

    auto* passwordLabel = AuthLayout::makeText(
        "Password", "fieldLabel");

    passwordEdit = new QLineEdit;
    passwordEdit->setObjectName("authField");
    passwordEdit->setPlaceholderText(
        "Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setClearButtonEnabled(true);

    layout->addWidget(passwordLabel);
    layout->addWidget(passwordEdit);

    auto* showPassword = new QCheckBox("Show password");
    layout->addWidget(showPassword);

    connect(showPassword, &QCheckBox::toggled,
            this, [this](bool checked) {
        passwordEdit->setEchoMode(
            checked ? QLineEdit::Normal
                    : QLineEdit::Password);
    });

    statusLabel = new QLabel;
    statusLabel->setObjectName("statusLabel");
    statusLabel->setWordWrap(true);
    statusLabel->setMinimumHeight(28);

    layout->addWidget(statusLabel);
    layout->addSpacing(4);

    loginButton = new QPushButton("Sign In");
    loginButton->setObjectName("primaryButton");
    loginButton->setMinimumHeight(46);
    loginButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(loginButton);
    layout->addSpacing(12);

    auto* signupRow = new QHBoxLayout;
    signupRow->setSpacing(5);
    signupRow->setAlignment(Qt::AlignCenter);

    auto* newUserLabel = AuthLayout::makeText(
        "New to OSimX?", "authFooterText");

    auto* signupLink = new QPushButton(
        "Create an account");
    signupLink->setObjectName("linkButton");
    signupLink->setCursor(Qt::PointingHandCursor);

    signupRow->addWidget(newUserLabel);
    signupRow->addWidget(signupLink);

    layout->addLayout(signupRow);

    connect(loginButton, &QPushButton::clicked,
            this, &LoginWindow::submitLogin);

    connect(usernameEdit, &QLineEdit::returnPressed,
            this, [this] {
        passwordEdit->setFocus();
    });

    connect(passwordEdit, &QLineEdit::returnPressed,
            this, &LoginWindow::submitLogin);

    connect(signupLink, &QPushButton::clicked,
            this, &LoginWindow::signupRequested);

    setTabOrder(usernameEdit, passwordEdit);
    setTabOrder(passwordEdit, showPassword);
    setTabOrder(showPassword, loginButton);
    setTabOrder(loginButton, signupLink);

    usernameEdit->setFocus();
}

void LoginWindow::setStatus(const QString& message,
                            bool error)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(
        error
            ? "color: #A63E35; font-weight: 600;"
            : "color: #426F4D; font-weight: 600;"
    );
}

void LoginWindow::submitLogin()
{
    const QString username =
        usernameEdit->text().trimmed();

    const QString password =
        passwordEdit->text();

    QString error;

    loginButton->setEnabled(false);

    const bool ok = AuthService::instance().login(
        username, password, error);

    loginButton->setEnabled(true);

    if (!ok) {
        setStatus(error, true);
        passwordEdit->selectAll();
        passwordEdit->setFocus();
        return;
    }

    setStatus(
        QString("Signed in. Welcome, %1.").arg(username),
        false
    );

    emit loginSuccessful();
}
