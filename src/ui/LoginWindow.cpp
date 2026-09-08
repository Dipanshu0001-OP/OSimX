#include "LoginWindow.h"
#include <QApplication>
#include <QFrame>
#include "auth/AuthService.h"
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    updateMode();
}

void LoginWindow::setupUI()
{
    setWindowTitle("OSimX");
    setWindowIcon(QIcon(":/resources/logo.png"));
    showMaximized();

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==================== LEFT PANEL ====================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setStyleSheet("background-color: #ffffff;");
    leftPanel->setFixedWidth(460);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->setContentsMargins(50, 30, 50, 30);
    leftLayout->setSpacing(12);

    QLabel *logoLabel = new QLabel(leftPanel);
    QPixmap logo(":/resources/logo.png");
    logoLabel->setPixmap(logo.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);

    QLabel *appNameLabel = new QLabel("OSimX", leftPanel);
    appNameLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #111;");
    appNameLabel->setAlignment(Qt::AlignCenter);

    QLabel *subNameLabel = new QLabel("Operating System Simulator", leftPanel);
    subNameLabel->setStyleSheet("font-size: 12px; color: #666;");
    subNameLabel->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel(leftPanel);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #111; margin-top: 12px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    formStack = new QStackedWidget(leftPanel);
    formStack->addWidget(createLoginForm());        // 0
    formStack->addWidget(createSignupChoiceForm()); // 1
    formStack->addWidget(createEmailSignupForm());  // 2

    QWidget *switchWidget = new QWidget(leftPanel);
    QHBoxLayout *switchLayout = new QHBoxLayout(switchWidget);
    switchLayout->setAlignment(Qt::AlignCenter);

    switchLabel = new QLabel(switchWidget);
    switchLabel->setStyleSheet("color: #555; font-size: 13px;");

    switchButton = new QPushButton(switchWidget);
    switchButton->setFlat(true);
    switchButton->setCursor(Qt::PointingHandCursor);
    switchButton->setStyleSheet("color: #00c853; font-weight: bold; font-size: 13px; border: none;");

    switchLayout->addWidget(switchLabel);
    switchLayout->addWidget(switchButton);

    leftLayout->addStretch();
    leftLayout->addWidget(logoLabel);
    leftLayout->addWidget(appNameLabel);
    leftLayout->addWidget(subNameLabel);
    leftLayout->addWidget(titleLabel);
    leftLayout->addSpacing(8);
    leftLayout->addWidget(formStack);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(switchWidget);
    leftLayout->addStretch();

    // ==================== RIGHT PANEL ====================
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setStyleSheet("background-color: #f4f7f5;");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(60, 50, 60, 50);

    QLabel *featuresTitle = new QLabel("Why OSimX?", rightPanel);
    featuresTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #1a1a1a;");

    QScrollArea *scrollArea = new QScrollArea(rightPanel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(20);

    QStringList features = {
        "Visualize CPU Scheduling algorithms (FCFS, SJF, SRTF, Round Robin, Priority) with real-time Gantt charts.",
        "Experiment with Memory Management techniques including First Fit, Best Fit, Worst Fit and page replacement algorithms.",
        "Understand Disk Scheduling algorithms such as SSTF, SCAN, C-SCAN and LOOK through interactive simulations.",
        "Detect and resolve Deadlocks using Banker's Algorithm and Resource Allocation Graphs.",
        "Explore Inter-Process Communication with Pipes, Message Queues and Shared Memory.",
        "Save your simulation history and compare different algorithms side by side.",
        "Built as a complete educational platform combining Operating System concepts with practical experimentation."
    };

    for (const QString &text : features) {
        QLabel *featureLabel = new QLabel("•  " + text, scrollContent);
        featureLabel->setWordWrap(true);
        featureLabel->setStyleSheet("font-size: 15px; color: #333; font-style: italic; line-height: 1.5;");
        scrollLayout->addWidget(featureLabel);
    }
    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);

    rightLayout->addWidget(featuresTitle);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(scrollArea);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);

    this->setStyleSheet(R"(
    QLineEdit {
        padding-left: 12px;
        border: 1px solid #ccc;
        border-radius: 8px;
        font-size: 14px;
        color: #222222;
        background-color: white;
        selection-background-color: #00c853;
        selection-color: white;
    }
    QLineEdit:focus {
        border: 1px solid #00c853;
    }
)");

    connect(switchButton, &QPushButton::clicked, this, [this]() {
        if (isLoginMode)
            switchToSignup();
        else
            switchToLogin();
    });
}

QWidget* LoginWindow::createLoginForm()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(8);

    // Username
    QLabel *userLabel = new QLabel("Username", page);
    userLabel->setStyleSheet("font-size: 13px; color: #333; font-weight: 500;");
    loginUsername = new QLineEdit(page);
    loginUsername->setPlaceholderText("Enter your username");
    loginUsername->setFixedHeight(40);
    loginUsername->setStyleSheet("padding-left: 12px; border: 1px solid #ccc; border-radius: 8px; font-size: 14px;");

    // Password
    QLabel *passLabel = new QLabel("Password", page);
    passLabel->setStyleSheet("font-size: 13px; color: #333; font-weight: 500;");
    loginPassword = new QLineEdit(page);
    loginPassword->setPlaceholderText("Enter your password");
    loginPassword->setEchoMode(QLineEdit::Password);
    loginPassword->setFixedHeight(40);
    loginPassword->setStyleSheet("padding-left: 12px; border: 1px solid #ccc; border-radius: 8px; font-size: 14px;");

    // Sign In button
    loginButton = new QPushButton("Sign In", page);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setFixedHeight(42);
    loginButton->setStyleSheet(
        "QPushButton { background-color: #00c853; color: white; border: none; border-radius: 8px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00b34a; }"
    );

    // OR
    QLabel *orLabel = new QLabel("OR", page);
    orLabel->setAlignment(Qt::AlignCenter);
    orLabel->setStyleSheet("color: #888; font-size: 13px; margin: 8px 0;");

    // Google button with logo
    googleLoginButton = new QPushButton("  Continue with Google", page);
    googleLoginButton->setCursor(Qt::PointingHandCursor);
    googleLoginButton->setFixedHeight(42);
    googleLoginButton->setIcon(QIcon(":/resources/googlelogo.webp"));
    googleLoginButton->setIconSize(QSize(20, 20));
    googleLoginButton->setStyleSheet(
        "QPushButton { background-color: white; border: 1px solid #dadce0; border-radius: 8px; font-size: 14px; color: #3c4043; text-align: center; }"
        "QPushButton:hover { background-color: #f7f8f8; }"
    );

    layout->addWidget(userLabel);
    layout->addWidget(loginUsername);
    layout->addSpacing(6);
    layout->addWidget(passLabel);
    layout->addWidget(loginPassword);
    layout->addSpacing(10);
    layout->addWidget(loginButton);
    layout->addWidget(orLabel);
    layout->addWidget(googleLoginButton);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(googleLoginButton, &QPushButton::clicked, this, &LoginWindow::onGoogleClicked);

    return page;
}

QWidget* LoginWindow::createSignupChoiceForm()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(14);

    googleSignupButton = new QPushButton("  Continue with Google", page);
googleSignupButton->setCursor(Qt::PointingHandCursor);
googleSignupButton->setFixedHeight(44);
googleSignupButton->setIcon(QIcon(":/resources/googlelogo.webp"));
googleSignupButton->setIconSize(QSize(20, 20));
googleSignupButton->setStyleSheet(
    "QPushButton { background-color: white; border: 1px solid #dadce0; border-radius: 8px; font-size: 14px; color: #3c4043; }"
    "QPushButton:hover { background-color: #f7f8f8; }"
);;

    emailSignupButton = new QPushButton("  Continue with Email", page);
    emailSignupButton->setCursor(Qt::PointingHandCursor);
    emailSignupButton->setFixedHeight(44);
    emailSignupButton->setStyleSheet(
        "QPushButton { background-color: #00c853; color: white; border: none; border-radius: 8px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00b34a; }"
    );

    layout->addWidget(googleSignupButton);
    layout->addWidget(emailSignupButton);

    connect(googleSignupButton, &QPushButton::clicked, this, &LoginWindow::onGoogleClicked);
    connect(emailSignupButton, &QPushButton::clicked, this, &LoginWindow::showEmailSignupForm);

    return page;
}

QWidget* LoginWindow::createEmailSignupForm()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(6);

    auto makeLabel = [](const QString &text, QWidget *parent) {
        QLabel *label = new QLabel(text, parent);
        label->setStyleSheet("font-size: 13px; color: #333; font-weight: 500;");
        return label;
    };

    auto makeEdit = [](const QString &placeholder, QWidget *parent, bool isPassword = false) {
        QLineEdit *edit = new QLineEdit(parent);
        edit->setPlaceholderText(placeholder);
        edit->setFixedHeight(38);
        edit->setStyleSheet("padding-left: 12px; border: 1px solid #ccc; border-radius: 8px;");
        if (isPassword) edit->setEchoMode(QLineEdit::Password);
        return edit;
    };

    layout->addWidget(makeLabel("Full Name", page));
    fullNameEdit = makeEdit("Enter your full name", page);
    layout->addWidget(fullNameEdit);

    layout->addWidget(makeLabel("Username", page));
    signupUsername = makeEdit("Choose a username", page);
    layout->addWidget(signupUsername);

    layout->addWidget(makeLabel("Email", page));
    emailEdit = makeEdit("Enter your email", page);
    layout->addWidget(emailEdit);

    layout->addWidget(makeLabel("Password", page));
    signupPassword = makeEdit("Create a password", page, true);
    layout->addWidget(signupPassword);

    layout->addWidget(makeLabel("Repeat Password", page));
    repeatPassword = makeEdit("Repeat your password", page, true);
    layout->addWidget(repeatPassword);

    layout->addSpacing(8);

    createAccountButton = new QPushButton("Create Account", page);
    createAccountButton->setCursor(Qt::PointingHandCursor);
    createAccountButton->setFixedHeight(42);
    createAccountButton->setStyleSheet(
        "QPushButton { background-color: #00c853; color: white; border: none; border-radius: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00b34a; }"
    );
    layout->addWidget(createAccountButton);

    backButton = new QPushButton("← Back", page);
    backButton->setFlat(true);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet("color: #00c853; font-size: 13px; border: none;");
    layout->addWidget(backButton, 0, Qt::AlignCenter);

    connect(createAccountButton, &QPushButton::clicked, this, &LoginWindow::onCreateAccountClicked);
    connect(backButton, &QPushButton::clicked, this, [this]() {
        formStack->setCurrentIndex(1);
    });

    return page;
}

void LoginWindow::updateMode()
{
    if (isLoginMode) {
        titleLabel->setText("Sign in to OSimX");
        switchLabel->setText("Don't have an account?");
        switchButton->setText("Sign up");
        formStack->setCurrentIndex(0);
    } else {
        titleLabel->setText("Create your account");
        switchLabel->setText("Already have an account?");
        switchButton->setText("Sign in");
        formStack->setCurrentIndex(1);
    }
}

void LoginWindow::switchToLogin()
{
    isLoginMode = true;
    updateMode();
}

void LoginWindow::switchToSignup()
{
    isLoginMode = false;
    updateMode();
}

void LoginWindow::showEmailSignupForm()
{
    formStack->setCurrentIndex(2);
}

void LoginWindow::onGoogleClicked()
{
    QMessageBox::information(this, "Google Login", "Google authentication will be added later.");
}

void LoginWindow::onLoginClicked()
{
    QString username = loginUsername->text().trimmed();
    QString password = loginPassword->text();

    QString errorMessage;
    bool success = AuthService::instance().login(username, password, errorMessage);

    if (success) {
        QMessageBox::information(this, "Success", "Login successful! Welcome " + username);
        emit loginSuccessful();   // Later we will open Main Window
        this->close();
    } else {
        QMessageBox::warning(this, "Login Failed", errorMessage);
    }
}

void LoginWindow::onCreateAccountClicked()
{
    QString fullName = fullNameEdit->text().trimmed();
    QString username = signupUsername->text().trimmed();
    QString email = emailEdit->text().trimmed();
    QString password = signupPassword->text();
    QString repeatPass = repeatPassword->text();

    if (password != repeatPass) {
        QMessageBox::warning(this, "Error", "Passwords do not match.");
        return;
    }

    QString errorMessage;
    bool success = AuthService::instance().signup(fullName, username, email, password, errorMessage);

    if (success) {
        QMessageBox::information(this, "Success", "Account created successfully!");
        emit loginSuccessful();
        this->close();
    } else {
        QMessageBox::warning(this, "Signup Failed", errorMessage);
    }
}