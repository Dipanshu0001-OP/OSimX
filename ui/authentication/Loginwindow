#include "LoginWindow.h"
#include "database/AuthService.h"

#include <QCheckBox>
#include <QIcon>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
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
    setWindowIcon(QIcon(":/resources/logo.png"));
    setMinimumSize(980, 640);

    /*
     * =========================================================
     * ROOT
     * =========================================================
     */

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);


    /*
     * =========================================================
     * LEFT BRAND PANEL
     * =========================================================
     */

    auto* brandPanel = new QWidget;
    brandPanel->setObjectName("brandPanel");

    // Important: brandPanel is QWidget, not QFrame.
    // Style it directly so the dark background is guaranteed.
    brandPanel->setStyleSheet(R"(
        QWidget#brandPanel {
            background-color: #172033;
        }

        QLabel {
            background: transparent;
        }

        QLabel#brandTitle {
            background: transparent;
            color: #ffffff;
            font-size: 42px;
            font-weight: 850;
        }

        QLabel#brandSubtitle {
            background: transparent;
            color: #c7d0dd;
            font-size: 16px;
        }

        QLabel#brandBody {
            background: transparent;
            color: #e0e6ee;
            font-size: 15px;
        }

        QLabel#brandPoint {
            background: transparent;
            color: #f4f7fa;
            font-size: 14px;
            padding: 6px 0;
        }

        QFrame#brandDivider {
            background: #394357;
            border: none;
            min-height: 2px;
            max-height: 2px;
        }
    )");

    auto* brand = new QVBoxLayout(brandPanel);
    brand->setContentsMargins(64, 56, 64, 56);
    brand->setSpacing(16);


    /*
     * Logo
     */

    auto* logo = new QLabel;
    logo->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    logo->setStyleSheet("background: transparent;");

    const QPixmap pixmap(":/resources/logo.png");

    if (!pixmap.isNull()) {
        logo->setPixmap(
            pixmap.scaled(
                88,
                88,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    }

    brand->addWidget(logo);


    /*
     * App name
     */

    auto* appName = new QLabel("OSimX");
    appName->setObjectName("brandTitle");
    brand->addWidget(appName);


    /*
     * Subtitle
     */

    auto* tag = new QLabel("Operating System Simulator");
    tag->setObjectName("brandSubtitle");
    brand->addWidget(tag);


    /*
     * Divider
     */

    auto* divider = new QFrame;
    divider->setObjectName("brandDivider");
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Plain);

    brand->addSpacing(22);
    brand->addWidget(divider);


    /*
     * Description
     */

    auto* purpose = new QLabel(
        "Learn how an operating system makes decisions "
        "by turning algorithms into visible, step-by-step simulations."
    );

    purpose->setObjectName("brandBody");
    purpose->setWordWrap(true);
    purpose->setStyleSheet(
        "background: transparent; color: #e0e6ee;"
    );

    brand->addWidget(purpose);


    /*
     * Feature points
     */

    const QStringList points = {
        "Build a process or request queue",
        "Run the algorithm",
        "Replay every decision and its reason"
    };

    for (const QString& item : points) {

        auto* point = new QLabel("•  " + item);

        point->setObjectName("brandPoint");
        point->setWordWrap(true);
        point->setStyleSheet(
            "background: transparent; color: #f4f7fa;"
        );

        brand->addWidget(point);
    }

    brand->addStretch();

    root->addWidget(brandPanel, 1);


    /*
     * =========================================================
     * RIGHT SIDE
     * =========================================================
     */

    auto* rightPanel = new QWidget;

    rightPanel->setStyleSheet(R"(
        QWidget {
            background: #eef2f6;
        }

        QLabel {
            background: transparent;
        }
    )");

    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(60, 48, 60, 48);
    rightLayout->setAlignment(Qt::AlignCenter);


    /*
     * =========================================================
     * LOGIN CARD
     * =========================================================
     */

    auto* card = new QFrame;
    card->setObjectName("authCard");
    card->setMaximumWidth(500);

    card->setStyleSheet(R"(
        QFrame#authCard {
            background: #ffffff;
            border: 1px solid #dfe4ec;
            border-radius: 16px;
        }

        QLabel {
            background: transparent;
        }

        QLabel#authTitle {
            background: transparent;
            color: #172033;
            font-size: 26px;
            font-weight: 800;
        }

        QLabel#authSubtitle {
            background: transparent;
            color: #667085;
            font-size: 14px;
        }

        QLabel#fieldLabel {
            background: transparent;
            color: #344054;
            font-weight: 700;
        }

        QLabel#statusLabel {
            background: transparent;
        }

        QLineEdit#authField {
            background: #ffffff;
            color: #172033;
            border: 1px solid #d4dae4;
            border-radius: 9px;
            padding: 9px 11px;
            selection-background-color: #2aa56a;
            selection-color: #ffffff;
        }

        QLineEdit#authField:focus {
            border: 1px solid #2aa56a;
            background: #ffffff;
        }

        QCheckBox {
            background: transparent;
            color: #526071;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #98a2b3;
            border-radius: 4px;
            background: #ffffff;
        }

        QCheckBox::indicator:hover {
            border: 1px solid #2aa56a;
        }

        QCheckBox::indicator:checked {
            background: #2aa56a;
            border: 1px solid #2aa56a;
        }

        QPushButton#primaryButton {
            background: #2aa56a;
            color: #ffffff;
            border: none;
            border-radius: 9px;
            min-height: 46px;
            font-weight: 700;
        }

        QPushButton#primaryButton:hover {
            background: #228b58;
        }

        QPushButton#primaryButton:pressed {
            background: #1c754a;
        }

        QPushButton#linkButton {
            background: transparent;
            color: #208e5b;
            border: none;
            padding: 2px 4px;
            font-weight: 700;
        }

        QPushButton#linkButton:hover {
            color: #176c45;
        }
    )");


    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(42, 38, 42, 38);
    cardLayout->setSpacing(12);


    /*
     * =========================================================
     * HEADING
     * =========================================================
     */

    auto* heading = new QLabel("Welcome back");
    heading->setObjectName("authTitle");

    auto* subheading = new QLabel(
        "Sign in to continue to your OSimX workspace."
    );

    subheading->setObjectName("authSubtitle");
    subheading->setWordWrap(true);

    cardLayout->addWidget(heading);
    cardLayout->addWidget(subheading);
    cardLayout->addSpacing(10);


    /*
     * =========================================================
     * USERNAME
     * =========================================================
     */

    auto* usernameLabel = new QLabel("Username");
    usernameLabel->setObjectName("fieldLabel");

    usernameEdit = new QLineEdit;
    usernameEdit->setObjectName("authField");
    usernameEdit->setPlaceholderText("Enter your username");
    usernameEdit->setClearButtonEnabled(true);

    cardLayout->addWidget(usernameLabel);
    cardLayout->addWidget(usernameEdit);


    /*
     * =========================================================
     * PASSWORD
     * =========================================================
     */

    auto* passwordLabel = new QLabel("Password");
    passwordLabel->setObjectName("fieldLabel");

    passwordEdit = new QLineEdit;
    passwordEdit->setObjectName("authField");
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setClearButtonEnabled(true);

    cardLayout->addWidget(passwordLabel);
    cardLayout->addWidget(passwordEdit);


    /*
     * =========================================================
     * SHOW PASSWORD
     * =========================================================
     */

    auto* showPassword = new QCheckBox("Show password");

    showPassword->setStyleSheet(R"(
        QCheckBox {
            background: transparent;
            color: #526071;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #98a2b3;
            border-radius: 4px;
            background: #ffffff;
        }

        QCheckBox::indicator:hover {
            border: 1px solid #2aa56a;
        }

        QCheckBox::indicator:checked {
            background: #2aa56a;
            border: 1px solid #2aa56a;
        }
    )");

    cardLayout->addWidget(showPassword);

    connect(
        showPassword,
        &QCheckBox::toggled,
        this,
        [this](bool checked) {
            passwordEdit->setEchoMode(
                checked
                    ? QLineEdit::Normal
                    : QLineEdit::Password
            );
        }
    );


    /*
     * =========================================================
     * STATUS
     * =========================================================
     */

    statusLabel = new QLabel;
    statusLabel->setObjectName("statusLabel");
    statusLabel->setWordWrap(true);
    statusLabel->setMinimumHeight(30);
    statusLabel->setStyleSheet(
        "background: transparent;"
    );

    cardLayout->addWidget(statusLabel);


    /*
     * =========================================================
     * SIGN IN BUTTON
     * =========================================================
     */

    loginButton = new QPushButton("Sign In");
    loginButton->setObjectName("primaryButton");
    loginButton->setMinimumHeight(46);
    loginButton->setCursor(Qt::PointingHandCursor);

    /*
     * Makes Enter activate Sign In when appropriate.
     */
    loginButton->setDefault(true);
    loginButton->setAutoDefault(true);

    cardLayout->addWidget(loginButton);


    /*
     * =========================================================
     * SIGNUP
     * =========================================================
     */

    auto* signupRow = new QHBoxLayout;
    signupRow->setAlignment(Qt::AlignCenter);

    auto* newUserLabel = new QLabel("New to OSimX?");
    newUserLabel->setStyleSheet(
        "background: transparent; color: #344054;"
    );

    auto* signupLink = new QPushButton("Create an account");
    signupLink->setObjectName("linkButton");
    signupLink->setCursor(Qt::PointingHandCursor);

    signupRow->addWidget(newUserLabel);
    signupRow->addWidget(signupLink);

    cardLayout->addLayout(signupRow);


    /*
     * IMPORTANT:
     * No unnecessary "Enter works everywhere..." note.
     */


    rightLayout->addWidget(card);

    root->addWidget(rightPanel, 1);


    /*
     * =========================================================
     * SIGNALS
     * =========================================================
     */

    connect(
        loginButton,
        &QPushButton::clicked,
        this,
        &LoginWindow::submitLogin
    );

    /*
     * Enter in username:
     * move to password.
     */
    connect(
        usernameEdit,
        &QLineEdit::returnPressed,
        this,
        [this] {
            passwordEdit->setFocus();
        }
    );

    /*
     * Enter in password:
     * submit login.
     */
    connect(
        passwordEdit,
        &QLineEdit::returnPressed,
        this,
        &LoginWindow::submitLogin
    );

    connect(
        signupLink,
        &QPushButton::clicked,
        this,
        &LoginWindow::signupRequested
    );


    /*
     * =========================================================
     * KEYBOARD NAVIGATION
     * =========================================================
     */

    setTabOrder(usernameEdit, passwordEdit);
    setTabOrder(passwordEdit, loginButton);

    usernameEdit->setFocus();
}


void LoginWindow::setStatus(
    const QString& message,
    bool error
)
{
    statusLabel->setText(message);

    statusLabel->setStyleSheet(
        error
            ? "background: transparent; color: #b42318; font-weight: 600;"
            : "background: transparent; color: #18864b; font-weight: 600;"
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

    const bool ok =
        AuthService::instance().login(
            username,
            password,
            error
        );

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
