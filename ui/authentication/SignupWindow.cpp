
#include "SignupWindow.h"
#include "AuthLayout.h"
#include "database/AuthService.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SignupWindow::SignupWindow(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void SignupWindow::setupUI()
{
    setWindowTitle("OSimX · Create account");

    auto* layout = AuthLayout::create(this, 510);
    layout->setSpacing(8);
    layout->setContentsMargins(38, 28, 38, 28);

    auto* eyebrow = AuthLayout::makeText(
        "GET STARTED WITH OSIMX", "authEyebrow");

    auto* title = AuthLayout::makeText(
        "Create your account", "authTitle");

    auto* subtitle = AuthLayout::makeText(
        "Create your personal workspace for "
        "exploring operating-system simulations.",
        "authSubtitle");

    layout->addWidget(eyebrow);
    layout->addSpacing(4);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(12);

    auto addField = [&](const QString& label,
                        QLineEdit*& target,
                        const QString& placeholder,
                        QLineEdit::EchoMode echo =
                            QLineEdit::Normal) {
        auto* caption = AuthLayout::makeText(
            label, "fieldLabel");

        target = new QLineEdit;
        target->setObjectName("authField");
        target->setPlaceholderText(placeholder);
        target->setEchoMode(echo);
        target->setClearButtonEnabled(true);

        layout->addWidget(caption);
        layout->addWidget(target);
        layout->addSpacing(4);
    };

    addField(
        "Full name",
        fullNameEdit,
        "Your full name"
    );

    addField(
        "Username",
        usernameEdit,
        "Choose a username"
    );

    addField(
        "Email",
        emailEdit,
        "you@example.com"
    );

    addField(
        "Password",
        passwordEdit,
        "At least 6 characters",
        QLineEdit::Password
    );

    addField(
        "Confirm password",
        confirmEdit,
        "Repeat your password",
        QLineEdit::Password
    );

    auto* showPassword = new QCheckBox(
        "Show passwords");

    layout->addWidget(showPassword);

    connect(showPassword, &QCheckBox::toggled,
            this, [this](bool checked) {
        const auto mode =
            checked ? QLineEdit::Normal
                    : QLineEdit::Password;

        passwordEdit->setEchoMode(mode);
        confirmEdit->setEchoMode(mode);
    });

    statusLabel = new QLabel;
    statusLabel->setObjectName("statusLabel");
    statusLabel->setWordWrap(true);
    statusLabel->setMinimumHeight(28);

    layout->addWidget(statusLabel);

    createButton = new QPushButton("Create Account");
    createButton->setObjectName("primaryButton");
    createButton->setMinimumHeight(46);
    createButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(createButton);
    layout->addSpacing(10);

    auto* loginRow = new QHBoxLayout;
    loginRow->setSpacing(5);
    loginRow->setAlignment(Qt::AlignCenter);

    auto* existingUser = AuthLayout::makeText(
        "Already have an account?",
        "authFooterText");

    auto* loginLink = new QPushButton("Sign in");
    loginLink->setObjectName("linkButton");
    loginLink->setCursor(Qt::PointingHandCursor);

    loginRow->addWidget(existingUser);
    loginRow->addWidget(loginLink);

    layout->addLayout(loginRow);


    layout->addSpacing(7);
    

    connect(createButton, &QPushButton::clicked,
            this, &SignupWindow::submitSignup);

    connect(fullNameEdit, &QLineEdit::returnPressed,
            this, [this] {
        usernameEdit->setFocus();
    });

    connect(usernameEdit, &QLineEdit::returnPressed,
            this, [this] {
        emailEdit->setFocus();
    });

    connect(emailEdit, &QLineEdit::returnPressed,
            this, [this] {
        passwordEdit->setFocus();
    });

    connect(passwordEdit, &QLineEdit::returnPressed,
            this, [this] {
        confirmEdit->setFocus();
    });

    connect(confirmEdit, &QLineEdit::returnPressed,
            this, &SignupWindow::submitSignup);

    connect(loginLink, &QPushButton::clicked,
            this, &SignupWindow::loginRequested);

    setTabOrder(fullNameEdit, usernameEdit);
    setTabOrder(usernameEdit, emailEdit);
    setTabOrder(emailEdit, passwordEdit);
    setTabOrder(passwordEdit, confirmEdit);
    setTabOrder(confirmEdit, showPassword);
    setTabOrder(showPassword, createButton);
    setTabOrder(createButton, loginLink);

    fullNameEdit->setFocus();
}

void SignupWindow::setStatus(const QString& message,
                             bool error)
{
    statusLabel->setText(message);

    statusLabel->setStyleSheet(
        error
            ? "color: #A63E35; font-weight: 600;"
            : "color: #426F4D; font-weight: 600;"
    );
}

void SignupWindow::submitSignup()
{
    const QString fullName =
        fullNameEdit->text().trimmed();

    const QString username =
        usernameEdit->text().trimmed();

    const QString email =
        emailEdit->text().trimmed();

    const QString password =
        passwordEdit->text();

    const QString confirm =
        confirmEdit->text();

    if (password != confirm) {
        setStatus("Passwords do not match.", true);
        confirmEdit->setFocus();
        return;
    }

    QString error;

    createButton->setEnabled(false);

    const bool ok =
        AuthService::instance().signup(
            fullName,
            username,
            email,
            password,
            error
        );

    createButton->setEnabled(true);

    if (!ok) {
        setStatus(error, true);
        return;
    }

    setStatus(
        "Account created. Opening your simulation workspace...",
        false
    );

    emit signupSuccessful();
}
