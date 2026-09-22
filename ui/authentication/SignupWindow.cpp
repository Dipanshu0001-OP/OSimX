#include "SignupWindow.h"
#include "database/AuthService.h"
#include <QCheckBox>
#include <QFrame>
#include <QIcon>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

SignupWindow::SignupWindow(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void SignupWindow::setupUI()
{
    setWindowTitle("OSimX · Create account");
    setWindowIcon(QIcon(":/resources/logo.png"));
    setMinimumSize(860, 620);

    auto* background = new QWidget;
    auto* backgroundLayout = new QVBoxLayout(background);
    backgroundLayout->setContentsMargins(30, 28, 30, 28);
    backgroundLayout->setAlignment(Qt::AlignCenter);

    auto* card = new QWidget;
    card->setObjectName("authCard");
    card->setMaximumWidth(580);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(44, 38, 44, 38);
    layout->setSpacing(10);

    auto* title = new QLabel("Create your OSimX account");
    title->setObjectName("authTitle");
    auto* subtitle = new QLabel("Your account stores your identity and gives you a clean starting point for the simulator.");
    subtitle->setObjectName("authSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(8);

    auto addField = [&](const QString& label, QLineEdit*& target, const QString& placeholder, QLineEdit::EchoMode echo = QLineEdit::Normal) {
        auto* caption = new QLabel(label);
        caption->setObjectName("fieldLabel");
        target = new QLineEdit;
        target->setPlaceholderText(placeholder);
        target->setObjectName("authField");
        target->setEchoMode(echo);
        target->setClearButtonEnabled(true);
        layout->addWidget(caption);
        layout->addWidget(target);
    };

    addField("Full name", fullNameEdit, "Your full name");
    addField("Username", usernameEdit, "Choose a username");
    addField("Email", emailEdit, "you@example.com");
    addField("Password", passwordEdit, "At least 6 characters", QLineEdit::Password);
    addField("Confirm password", confirmEdit, "Repeat your password", QLineEdit::Password);

    auto* showPassword = new QCheckBox("Show passwords");
    layout->addWidget(showPassword);
    connect(showPassword, &QCheckBox::toggled, this, [this](bool checked) {
        const auto mode = checked ? QLineEdit::Normal : QLineEdit::Password;
        passwordEdit->setEchoMode(mode);
        confirmEdit->setEchoMode(mode);
    });

    statusLabel = new QLabel;
    statusLabel->setWordWrap(true);
    statusLabel->setMinimumHeight(30);
    layout->addWidget(statusLabel);

    createButton = new QPushButton("Create Account");
    createButton->setObjectName("primaryButton");
    createButton->setMinimumHeight(46);
    createButton->setCursor(Qt::PointingHandCursor);
    layout->addWidget(createButton);

    auto* loginRow = new QHBoxLayout;
    loginRow->setAlignment(Qt::AlignCenter);
    loginRow->addWidget(new QLabel("Already have an account?"));
    auto* loginLink = new QPushButton("Sign in");
    loginLink->setObjectName("linkButton");
    loginRow->addWidget(loginLink);
    layout->addLayout(loginRow);

    auto* hint = new QLabel("Enter moves through the fields in order; Enter on Confirm password creates the account.");
    hint->setObjectName("formHint");
    hint->setWordWrap(true);
    layout->addWidget(hint);

    backgroundLayout->addWidget(card);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(background);
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(scroll);

    connect(createButton, &QPushButton::clicked, this, &SignupWindow::submitSignup);
    connect(fullNameEdit, &QLineEdit::returnPressed, this, [this] { usernameEdit->setFocus(); });
    connect(usernameEdit, &QLineEdit::returnPressed, this, [this] { emailEdit->setFocus(); });
    connect(emailEdit, &QLineEdit::returnPressed, this, [this] { passwordEdit->setFocus(); });
    connect(passwordEdit, &QLineEdit::returnPressed, this, [this] { confirmEdit->setFocus(); });
    connect(confirmEdit, &QLineEdit::returnPressed, this, &SignupWindow::submitSignup);
    connect(loginLink, &QPushButton::clicked, this, &SignupWindow::loginRequested);

    setTabOrder(fullNameEdit, usernameEdit);
    setTabOrder(usernameEdit, emailEdit);
    setTabOrder(emailEdit, passwordEdit);
    setTabOrder(passwordEdit, confirmEdit);
    setTabOrder(confirmEdit, createButton);
    fullNameEdit->setFocus();
}

void SignupWindow::setStatus(const QString& message, bool error)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(error ? "color:#b42318; font-weight:600;" : "color:#18864b; font-weight:600;");
}

void SignupWindow::submitSignup()
{
    const QString fullName = fullNameEdit->text().trimmed();
    const QString username = usernameEdit->text().trimmed();
    const QString email = emailEdit->text().trimmed();
    const QString password = passwordEdit->text();
    const QString confirm = confirmEdit->text();

    if (password != confirm) {
        setStatus("Passwords do not match.", true);
        confirmEdit->setFocus();
        return;
    }

    QString error;
    createButton->setEnabled(false);
    const bool ok = AuthService::instance().signup(fullName, username, email, password, error);
    createButton->setEnabled(true);

    if (!ok) {
        setStatus(error, true);
        return;
    }

    setStatus("Account created. Opening your simulation workspace...", false);
    emit signupSuccessful();
}
