#include "LoginWindow.h"
#include "core/DBManager.h"
#include "core/AuthSettings.h"
#include "core/ThemeManager.h"
#include "ui/widgets/Toast.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QAction>
#include <QFrame>
#include <QSettings>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QKeyEvent>

LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent)
{
    // Возвращаем Qt::Dialog — окно получает фокус, Esc работает
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(420);
    setFocusPolicy(Qt::StrongFocus); // <--- явно разрешаем фокус

    setupUi();
    adjustSize();
    setFixedHeight(height());
}

void LoginWindow::setupUi()
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
    layout->setContentsMargins(30, 20, 30, 20);
    layout->setSpacing(0);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(15);

    auto* iconLabel = new QLabel(container);
    QIcon loginIcon = ThemeManager::instance().getIcon("profileBtn", QColor("#A3E635"), "LoginWindow");
    if (!loginIcon.isNull()) iconLabel->setPixmap(loginIcon.pixmap(48, 48));
    headerLayout->addWidget(iconLabel);

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(0);
    auto* title = new QLabel("Войти", container);
    title->setObjectName("loginTitle");
    titleLayout->addWidget(title);
    titleLayout->addSpacing(3);
    auto* subtitle = new QLabel("Откройте новые возможности", container);
    subtitle->setObjectName("loginSubtitle");
    titleLayout->addWidget(subtitle);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    layout->addSpacing(15);
    addDivider(layout, container);

    layout->addSpacing(15);
    auto* userLabel = new QLabel("ЛОГИН", container);
    userLabel->setObjectName("inputLabel");
    layout->addWidget(userLabel);

    layout->addSpacing(5);
    m_usernameEdit = new QLineEdit(container);
    m_usernameEdit->setPlaceholderText("Введите ваш логин");
    layout->addWidget(m_usernameEdit);

    layout->addSpacing(8);
    auto* passHeaderLayout = new QHBoxLayout();
    passHeaderLayout->setContentsMargins(0, 0, 0, 0);
    auto* passLabel = new QLabel("ПАРОЛЬ", container);
    passLabel->setObjectName("inputLabel");
    auto* forgotBtn = new QPushButton("Забыли пароль?", container);
    forgotBtn->setObjectName("linkButton");
    forgotBtn->setCursor(Qt::PointingHandCursor);
    passHeaderLayout->addWidget(passLabel);
    passHeaderLayout->addStretch();
    passHeaderLayout->addWidget(forgotBtn);
    layout->addLayout(passHeaderLayout);

    layout->addSpacing(5);
    m_passwordEdit = new QLineEdit(container);
    m_passwordEdit->setPlaceholderText("Введите пароль");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passwordEdit);

    QIcon openEyeIcon = ThemeManager::instance().getIcon("passwordEditOpen", QColor("#9CA3AF"), "LoginWindow");
    if (!openEyeIcon.isNull()) {
        m_togglePasswordAction = m_passwordEdit->addAction(openEyeIcon, QLineEdit::TrailingPosition);
        connect(m_togglePasswordAction, &QAction::triggered, this, &LoginWindow::togglePasswordVisibility);
    }

    layout->addSpacing(10);
    m_rememberCheck = new QCheckBox("Запомнить меня", container);
    layout->addWidget(m_rememberCheck);

    layout->addSpacing(10);
    auto* loginBtn = new QPushButton("ВОЙТИ", container);
    loginBtn->setObjectName("loginSubmitBtn");
    loginBtn->setMinimumHeight(45);
    loginBtn->setCursor(Qt::PointingHandCursor);
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    layout->addWidget(loginBtn);

    layout->addSpacing(15);
    addDivider(layout, container);

    layout->addSpacing(15);
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    auto* noAccountLabel = new QLabel("Нет аккаунта?", container);
    noAccountLabel->setStyleSheet("color: #9CA3AF; font-size: 13px; font-weight: bold;");

    auto* registerBtn = new QPushButton("Создать аккаунт", container);
    registerBtn->setObjectName("outlineButton");
    registerBtn->setCursor(Qt::PointingHandCursor);
    registerBtn->setMinimumHeight(32);
    registerBtn->setMinimumWidth(140);
    connect(registerBtn, &QPushButton::clicked, this, &LoginWindow::registerRequested);

    bottomLayout->addWidget(noAccountLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(registerBtn);
    layout->addLayout(bottomLayout);

    connect(m_usernameEdit, &QLineEdit::returnPressed, m_passwordEdit, qOverload<>(&QWidget::setFocus));
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::addDivider(QVBoxLayout* layout, QWidget* parent)
{
    auto* line = new QFrame(parent);
    line->setObjectName("dividerLine");
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedHeight(1);
    line->setStyleSheet("background-color: #333333; border: none;");
    layout->addWidget(line);
}

void LoginWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    QDialog::keyPressEvent(event);
}

void LoginWindow::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        if (m_togglePasswordAction) {
            m_togglePasswordAction->setIcon(
                ThemeManager::instance().getIcon("passwordEditClose", QColor("#A3E635"), "LoginWindow"));
        }
    }
    else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        if (m_togglePasswordAction) {
            m_togglePasswordAction->setIcon(
                ThemeManager::instance().getIcon("passwordEditOpen", QColor("#9CA3AF"), "LoginWindow"));
        }
    }
}

void LoginWindow::onLoginClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        Toast::showMessage(this, "Заполните все поля!", true);
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
        settings.sync();

        Toast::showMessage(this, "Успешный вход!", false);

        QTimer::singleShot(1000, this, [this, user]() {
            emit loginSuccessful(user);
            accept();
            });
    }
    else {
        Toast::showMessage(this, "Неверный логин или пароль!", true);
    }
}

void LoginWindow::closeEvent(QCloseEvent* event)
{
    emit windowClosed();
    QDialog::closeEvent(event);
}
