#include "RegisterWindow.h"
#include "core/DBManager.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>

RegisterWindow::RegisterWindow(QWidget* parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(420, 600);

    setupUi();
}

void RegisterWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* container = new QWidget(this);
    container->setObjectName("dialogContainer");
    mainLayout->addWidget(container);

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 180));
    shadow->setOffset(0, 0);
    container->setGraphicsEffect(shadow);

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(30, 25, 30, 25);
    layout->setSpacing(15);

    auto* topBar = new QHBoxLayout();
    topBar->addStretch();
    auto* closeBtn = new QPushButton("✕", container);
    closeBtn->setObjectName("closeDialogBtn");
    closeBtn->setFixedSize(30, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &RegisterWindow::close);
    topBar->addWidget(closeBtn);
    layout->addLayout(topBar);

    auto* title = new QLabel("Регистрация", container);
    title->setObjectName("loginTitle");
    title->setAlignment(Qt::AlignCenter);

    auto* subtitle = new QLabel("Создайте новый аккаунт", container);
    subtitle->setObjectName("loginSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(20);

    auto* userLabel = new QLabel("ЛОГИН", container);
    userLabel->setObjectName("inputLabel");
    m_usernameEdit = new QLineEdit(container);
    m_usernameEdit->setPlaceholderText("Придумайте логин");

    auto* passLabel = new QLabel("ПАРОЛЬ", container);
    passLabel->setObjectName("inputLabel");
    m_passwordEdit = new QLineEdit(container);
    m_passwordEdit->setPlaceholderText("Придумайте пароль");
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    auto* confirmLabel = new QLabel("ПОДТВЕРДИТЕ ПАРОЛЬ", container);
    confirmLabel->setObjectName("inputLabel");
    m_confirmEdit = new QLineEdit(container);
    m_confirmEdit->setPlaceholderText("Повторите пароль");
    m_confirmEdit->setEchoMode(QLineEdit::Password);

    layout->addWidget(userLabel);
    layout->addWidget(m_usernameEdit);
    layout->addWidget(passLabel);
    layout->addWidget(m_passwordEdit);
    layout->addWidget(confirmLabel);
    layout->addWidget(m_confirmEdit);
    layout->addSpacing(10);

    auto* registerBtn = new QPushButton("СОЗДАТЬ АККАУНТ", container);
    registerBtn->setObjectName("loginSubmitBtn");
    registerBtn->setMinimumHeight(45);
    registerBtn->setCursor(Qt::PointingHandCursor);
    connect(registerBtn, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);

    auto* loginBtn = new QPushButton("У МЕНЯ ЕСТЬ АККАУНТ", container);
    loginBtn->setObjectName("outlineButton");
    loginBtn->setMinimumHeight(40);
    loginBtn->setCursor(Qt::PointingHandCursor);
    connect(loginBtn, &QPushButton::clicked, this, &RegisterWindow::loginRequested);

    layout->addWidget(registerBtn);
    layout->addWidget(loginBtn);
    layout->addStretch();

    // Обработка Enter
    connect(m_usernameEdit, &QLineEdit::returnPressed, m_passwordEdit, qOverload<>(&QWidget::setFocus));
    connect(m_passwordEdit, &QLineEdit::returnPressed, m_confirmEdit, qOverload<>(&QWidget::setFocus));
    connect(m_confirmEdit, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
}

void RegisterWindow::clearFields()
{
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    m_confirmEdit->clear();
}

void RegisterWindow::onRegisterClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();
    QString confirm = m_confirmEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля!");
        return;
    }
    if (pass != confirm) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают!");
        return;
    }

    if (DBManager::instance().registerUser(user, pass)) {
        QMessageBox::information(this, "Успех", "Аккаунт создан! Теперь вы можете войти.");
        emit registrationSuccessful(user);
        accept();
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Этот логин уже занят или произошла ошибка БД.");
    }
}

void RegisterWindow::closeEvent(QCloseEvent* event)
{
    emit windowClosed();
    QDialog::closeEvent(event);
}
