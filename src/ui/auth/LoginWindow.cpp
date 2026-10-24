#include "LoginWindow.h"
#include "core/DBManager.h"
#include "core/AuthSettings.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QToolButton>
#include <QAction>
#include <QSettings>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>

LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(400, 550);

    setupUi();
}

void LoginWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* container = new QWidget(this);
    container->setObjectName("dialogContainer");
    mainLayout->addWidget(container);

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);

    auto* topBar = new QHBoxLayout();
    topBar->addStretch();
    auto* closeBtn = new QPushButton("✕", container);
    closeBtn->setObjectName("closeDialogBtn");
    closeBtn->setFixedSize(30, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &LoginWindow::close);
    topBar->addWidget(closeBtn);
    layout->addLayout(topBar);

    auto* title = new QLabel("Вход в аккаунт", container);
    title->setObjectName("loginTitle");
    title->setAlignment(Qt::AlignCenter);

    auto* subtitle = new QLabel("Введите свои данные для доступа", container);
    subtitle->setObjectName("loginSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(20);

    auto* userLabel = new QLabel("ЛОГИН", container);
    userLabel->setObjectName("inputLabel");
    m_usernameEdit = new QLineEdit(container);
    m_usernameEdit->setPlaceholderText("Введите логин");

    auto* passLabel = new QLabel("ПАРОЛЬ", container);
    passLabel->setObjectName("inputLabel");
    m_passwordEdit = new QLineEdit(container);
    m_passwordEdit->setPlaceholderText("Введите пароль");
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    // Глазик для пароля (если есть иконка "eye" в папке Auth)
    QIcon eyeIcon = ThemeManager::instance().getIcon("eye", QColor("#9CA3AF"), "Auth");
    if (!eyeIcon.isNull()) {
        QAction* toggleAction = m_passwordEdit->addAction(eyeIcon, QLineEdit::TrailingPosition);
        connect(toggleAction, &QAction::triggered, this, &LoginWindow::togglePasswordVisibility);
    }

    layout->addWidget(userLabel);
    layout->addWidget(m_usernameEdit);
    layout->addWidget(passLabel);
    layout->addWidget(m_passwordEdit);

    m_rememberCheck = new QCheckBox("Запомнить меня", container);
    layout->addWidget(m_rememberCheck);
    layout->addSpacing(10);

    auto* loginBtn = new QPushButton("ВОЙТИ", container);
    loginBtn->setObjectName("loginSubmitBtn");
    loginBtn->setMinimumHeight(45);
    loginBtn->setCursor(Qt::PointingHandCursor);
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);

    auto* registerBtn = new QPushButton("СОЗДАТЬ АККАУНТ", container);
    registerBtn->setObjectName("outlineButton");
    registerBtn->setMinimumHeight(40);
    registerBtn->setCursor(Qt::PointingHandCursor);
    connect(registerBtn, &QPushButton::clicked, this, &LoginWindow::registerRequested);

    layout->addWidget(loginBtn);
    layout->addWidget(registerBtn);
    layout->addStretch();
}

void LoginWindow::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
    }
    else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
    }
}

void LoginWindow::onLoginClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля!");
        return;
    }

    if (DBManager::instance().authenticateUser(user, pass)) {
        QSettings settings;
        settings.setValue(AuthSettings::kRememberMe, m_rememberCheck->isChecked());
        if (m_rememberCheck->isChecked()) {
            settings.setValue(AuthSettings::kUsername, user);
        }
        else {
            settings.remove(AuthSettings::kUsername);
        }

        emit loginSuccessful(user);
        accept();
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль!");
    }
}

void LoginWindow::closeEvent(QCloseEvent* event)
{
    emit windowClosed();
    QDialog::closeEvent(event);
}
