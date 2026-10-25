#include "RegisterWindow.h"
#include "core/DBManager.h"
#include "core/ThemeManager.h"
#include "ui/widgets/Toast.h"
#include "ui/widgets/RequirementsPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QAction>
#include <QFrame>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QTimer>
#include <QShowEvent>
#include <QHideEvent>
#include <QKeyEvent>
#include <QDebug> // <--- для диагностики

RegisterWindow::RegisterWindow(QWidget* parent) : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(420);
    setFocusPolicy(Qt::StrongFocus);

    // Создаём панель требований
    m_reqPanel = new RequirementsPanel(this);
    m_reqPanel->hide();

    setupUi();
    adjustSize();
    setFixedHeight(height());
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
    layout->setContentsMargins(30, 20, 30, 20);
    layout->setSpacing(0);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(15);
    auto* iconLabel = new QLabel(container);
    QIcon registerIcon = ThemeManager::instance().getIcon(
        "registerBtn",
        ThemeManager::instance().accentColor(),
        "RegisterWindow");
    if (!registerIcon.isNull()) iconLabel->setPixmap(registerIcon.pixmap(48, 48));
    headerLayout->addWidget(iconLabel);

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(0);
    auto* title = new QLabel("Создать аккаунт", container);
    title->setObjectName("loginTitle");
    titleLayout->addWidget(title);
    titleLayout->addSpacing(3);
    auto* subtitle = new QLabel("Больше функций - больше возможностей", container);
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
    auto* passLabel = new QLabel("ПАРОЛЬ", container);
    passLabel->setObjectName("inputLabel");
    layout->addWidget(passLabel);

    layout->addSpacing(5);
    m_passwordEdit = new QLineEdit(container);
    m_passwordEdit->setPlaceholderText("Введите ваш пароль");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passwordEdit);

    QIcon openEyeIcon = ThemeManager::instance().getIcon("passwordEditOpen", QColor("#9CA3AF"), "RegisterWindow");
    if (!openEyeIcon.isNull()) {
        m_togglePassAction = m_passwordEdit->addAction(openEyeIcon, QLineEdit::TrailingPosition);
        connect(m_togglePassAction, &QAction::triggered, this, [this]() {
            if (m_passwordEdit->echoMode() == QLineEdit::Password) {
                m_passwordEdit->setEchoMode(QLineEdit::Normal);
                m_togglePassAction->setIcon(ThemeManager::instance().getIcon(
                    "passwordEditClose", ThemeManager::instance().accentColor(), "RegisterWindow"));
            }
            else {
                m_passwordEdit->setEchoMode(QLineEdit::Password);
                m_togglePassAction->setIcon(ThemeManager::instance().getIcon(
                    "passwordEditOpen", QColor("#9CA3AF"), "RegisterWindow"));
            }
            });
    }

    layout->addSpacing(8);
    auto* confirmLabel = new QLabel("ПОВТОРИТЕ ПАРОЛЬ", container);
    confirmLabel->setObjectName("inputLabel");
    layout->addWidget(confirmLabel);

    layout->addSpacing(5);
    m_confirmEdit = new QLineEdit(container);
    m_confirmEdit->setPlaceholderText("Введите пароль еще раз");
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_confirmEdit);

    if (!openEyeIcon.isNull()) {
        m_toggleConfirmAction = m_confirmEdit->addAction(openEyeIcon, QLineEdit::TrailingPosition);
        connect(m_toggleConfirmAction, &QAction::triggered, this, [this]() {
            if (m_confirmEdit->echoMode() == QLineEdit::Password) {
                m_confirmEdit->setEchoMode(QLineEdit::Normal);
                m_toggleConfirmAction->setIcon(ThemeManager::instance().getIcon(
                    "passwordEditClose", ThemeManager::instance().accentColor(), "RegisterWindow"));
            }
            else {
                m_confirmEdit->setEchoMode(QLineEdit::Password);
                m_toggleConfirmAction->setIcon(ThemeManager::instance().getIcon(
                    "passwordEditOpen", QColor("#9CA3AF"), "RegisterWindow"));
            }
            });
    }

    layout->addSpacing(8);
    auto* referralLabel = new QLabel("РЕФЕРАЛЬНЫЙ КОД", container);
    referralLabel->setObjectName("inputLabel");
    layout->addWidget(referralLabel);

    layout->addSpacing(5);
    m_referralEdit = new QLineEdit(container);
    m_referralEdit->setPlaceholderText("Введите реферальный код, если есть");
    layout->addWidget(m_referralEdit);

    layout->addSpacing(10);
    QString accent = ThemeManager::instance().accentColor().name();
    auto* privacyLabel = new QLabel(container);
    privacyLabel->setTextFormat(Qt::RichText);
    privacyLabel->setWordWrap(true);
    privacyLabel->setStyleSheet("color: #A8998A; font-size: 11px;");
    privacyLabel->setText(QString("Даю согласие на обработку персональных данных на условиях "
        "<a href='#' style='color: %1; text-decoration: none;'>Политики конфиденциальности</a>").arg(accent));
    layout->addWidget(privacyLabel);

    layout->addSpacing(5);
    auto* agreementLabel = new QLabel(container);
    agreementLabel->setTextFormat(Qt::RichText);
    agreementLabel->setWordWrap(true);
    agreementLabel->setStyleSheet("color: #A8998A; font-size: 11px;");
    agreementLabel->setText(QString("Нажимая «Создать аккаунт» я принимаю "
        "<a href='#' style='color: %1; text-decoration: none;'>Пользовательское соглашение</a> "
        "и подтверждаю, что мне есть 14 лет.").arg(accent));
    layout->addWidget(agreementLabel);

    layout->addSpacing(15);
    auto* registerBtn = new QPushButton("СОЗДАТЬ АККАУНТ", container);
    registerBtn->setObjectName("loginSubmitBtn");
    registerBtn->setMinimumHeight(45);
    registerBtn->setCursor(Qt::PointingHandCursor);
    connect(registerBtn, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    layout->addWidget(registerBtn);

    layout->addSpacing(15);
    addDivider(layout, container);
    layout->addSpacing(15);
    auto* bottomLayout = new QHBoxLayout();
    auto* haveAccountLabel = new QLabel("Уже есть аккаунт?", container);
    haveAccountLabel->setStyleSheet("color: #A8998A; font-size: 13px; font-weight: bold;");

    auto* loginBtn = new QPushButton("Войти", container);
    loginBtn->setObjectName("outlineButton");
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setMinimumHeight(32);
    loginBtn->setMinimumWidth(80);
    connect(loginBtn, &QPushButton::clicked, this, &RegisterWindow::loginRequested);

    bottomLayout->addWidget(haveAccountLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(loginBtn);
    layout->addLayout(bottomLayout);

    connect(m_usernameEdit, &QLineEdit::textChanged, this, &RegisterWindow::validateUsername);
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &RegisterWindow::validatePassword);

    validateUsername("");
    validatePassword("");

    connect(m_usernameEdit, &QLineEdit::returnPressed, m_passwordEdit, qOverload<>(&QWidget::setFocus));
    connect(m_passwordEdit, &QLineEdit::returnPressed, m_confirmEdit, qOverload<>(&QWidget::setFocus));
    connect(m_confirmEdit, &QLineEdit::returnPressed, m_referralEdit, qOverload<>(&QWidget::setFocus));
    connect(m_referralEdit, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
}

void RegisterWindow::validateUsername(const QString& text)
{
    bool validLength = text.length() >= 3 && text.length() <= 16;
    QRegularExpression noDigits("^[A-Za-zА-Яа-я_]+$");
    bool validChars = noDigits.match(text).hasMatch() && text.length() > 0;
    QRegularExpression onlyLatin("^[A-Za-z_]+$");
    QRegularExpression onlyCyrillic("^[А-Яа-я_]+$");
    bool validAlphabet = (onlyLatin.match(text).hasMatch() || onlyCyrillic.match(text).hasMatch()) && text.length() > 0;
    bool validStart = text.length() > 0 && text[0].isUpper();

    m_reqPanel->setRuleValid(0, validLength);
    m_reqPanel->setRuleValid(1, validChars);
    m_reqPanel->setRuleValid(2, validAlphabet);
    m_reqPanel->setRuleValid(3, validStart);
}

void RegisterWindow::validatePassword(const QString& text)
{
    bool validLength = text.length() >= 12;
    QRegularExpression passRegex("^[a-zA-Z0-9!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?~`]{12,}$");
    bool validChars = passRegex.match(text).hasMatch() && text.length() > 0;

    m_reqPanel->setRuleValid(4, validLength);
    m_reqPanel->setRuleValid(5, validChars);
}

void RegisterWindow::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    if (m_reqPanel) {
        m_reqPanel->show();
        m_reqPanel->raise();
        m_reqPanel->move(x() - m_reqPanel->width() - 15, y());
        QTimer::singleShot(0, this, [this]() {
            // После того как окно полностью активировалось — поднимаем панель ещё раз
            if (m_reqPanel && m_reqPanel->isVisible()) {
                m_reqPanel->raise();
            }
            });
    }
}

void RegisterWindow::hideEvent(QHideEvent* event)
{
    QDialog::hideEvent(event);
    if (m_reqPanel) m_reqPanel->hide();
}

void RegisterWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    QDialog::keyPressEvent(event);
}

void RegisterWindow::addDivider(QVBoxLayout* layout, QWidget* parent)
{
    auto* line = new QFrame(parent);
    line->setObjectName("dividerLine");
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedHeight(1);
    line->setStyleSheet("background-color: #3A2F26; border: none;");
    layout->addWidget(line);
}

void RegisterWindow::clearFields()
{
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    m_confirmEdit->clear();
    m_referralEdit->clear();
}

void RegisterWindow::onRegisterClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();
    QString confirm = m_confirmEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        Toast::showMessage(this, "Заполните все поля!", true);
        return;
    }

    QRegularExpression noDigits("^[A-Za-zА-Яа-я_]+$");
    QRegularExpression onlyLatin("^[A-Za-z_]+$");
    QRegularExpression onlyCyrillic("^[А-Яа-я_]+$");
    bool isUserValid = (user.length() >= 3 && user.length() <= 16) &&
        noDigits.match(user).hasMatch() &&
        (onlyLatin.match(user).hasMatch() || onlyCyrillic.match(user).hasMatch()) &&
        user[0].isUpper();

    if (!isUserValid) {
        Toast::showMessage(this, "Логин не соответствует требованиям!", true);
        return;
    }

    QRegularExpression passRegex("^[a-zA-Z0-9!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?~`]{12,}$");
    if (!passRegex.match(pass).hasMatch()) {
        Toast::showMessage(this, "Пароль должен быть от 12 символов (латиница, цифры, спецсимволы)", true);
        return;
    }

    if (pass != confirm) {
        Toast::showMessage(this, "Пароли не совпадают!", true);
        return;
    }

    if (DBManager::instance().registerUser(user, pass)) {
        Toast::showMessage(this, QString("Аккаунт %1 создан!").arg(user), false);
        QTimer::singleShot(1500, this, [this, user]() {
            emit registrationSuccessful(user);
            accept();
            });
    }
    else {
        Toast::showMessage(this, QString("Логин %1 занят!").arg(user), true);
    }
}

void RegisterWindow::closeEvent(QCloseEvent* event)
{
    if (m_reqPanel) m_reqPanel->hide();
    emit windowClosed();
    QDialog::closeEvent(event);
}
