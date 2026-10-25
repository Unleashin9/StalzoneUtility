#include "SettingsWindow.h"
#include "core/ThemeManager.h"
#include "ui/widgets/Toast.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QShowEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QStackedWidget>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QPainter>
#include <QPainterPath>
#include <QFrame>
#include <QLineEdit>
#include <QRegularExpression>

SettingsWindow::SettingsWindow(QWidget* parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setObjectName("settingsRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
}

void SettingsWindow::setUserInfo(const UserInfo& info)
{
    m_userInfo = info;
    if (m_userInfoLabel) {
        m_userInfoLabel->setText(QString("ID: %1  |  Дата регистрации: %2")
            .arg(m_userInfo.id)
            .arg(m_userInfo.createdAt));
    }
    if (m_usernameLabel) m_usernameLabel->setText(m_userInfo.username);
    if (m_accountNameLabel) m_accountNameLabel->setText(m_userInfo.username);
    if (m_accountDetailsLabel) {
        m_accountDetailsLabel->setText(QString("Почта не подключена | ID: %1 | Дата регистрации: %2")
            .arg(m_userInfo.id).arg(m_userInfo.createdAt));
    }
    if (m_loginField) m_loginField->setText(m_userInfo.username);

    loadAvatar();
}

void SettingsWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === ВЕРХНЯЯ ПАНЕЛЬ ===
    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(60);
    topBar->setAttribute(Qt::WA_StyledBackground, true);

    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(50, 0, 50, 0);
    topBarLayout->setSpacing(15);

    auto* logoBtn = new QPushButton("STALZONE Utility", topBar);
    logoBtn->setObjectName("logoBtn");
    logoBtn->setIcon(ThemeManager::instance().getIcon(
        "szutility", ThemeManager::instance().accentColor(), QString()));
    logoBtn->setIconSize(QSize(32, 32));
    logoBtn->setCursor(Qt::PointingHandCursor);
    logoBtn->setFlat(true);
    logoBtn->setFocusPolicy(Qt::NoFocus);
    logoBtn->setStyleSheet("background: transparent; border: none;");
    connect(logoBtn, &QPushButton::clicked, this, &SettingsWindow::backRequested);

    m_userInfoLabel = new QLabel(topBar);
    m_userInfoLabel->setStyleSheet("color: #A8998A; font-size: 14px; font-weight: bold; margin-left: 20px;");

    auto* emailStatusLabel = new QLabel("• ПОЧТА НЕ ПОДТВЕРЖДЕНА", topBar);
    emailStatusLabel->setStyleSheet(
        "background-color: #1F1A16; color: #E5533D; border: 1px solid #E5533D;"
        "border-radius: 6px; padding: 4px 10px; font-size: 12px; font-weight: bold;"
    );

    topBarLayout->addWidget(logoBtn, 0, Qt::AlignVCenter);
    topBarLayout->addWidget(m_userInfoLabel, 0, Qt::AlignVCenter);
    topBarLayout->addStretch();
    topBarLayout->addWidget(emailStatusLabel, 0, Qt::AlignVCenter);

    mainLayout->addWidget(topBar);

    // === СРЕДНЯЯ ЗОНА ===
    auto* middleLayout = new QHBoxLayout();
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(0);

    auto* sidebar = new QWidget(this);
    sidebar->setObjectName("settingsSidebar");
    sidebar->setFixedWidth(220);
    sidebar->setAttribute(Qt::WA_StyledBackground, true);

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(15, 20, 15, 20);
    sidebarLayout->setSpacing(4);

    auto createSidebarItem = [this, sidebar](const QString& text, const QString& iconName, int pageIndex, const QString& pageTitle) -> QPushButton* {
        auto* btn = new QPushButton(" " + text, sidebar);
        btn->setObjectName("sidebarItem");
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(36);
        btn->setIconSize(QSize(18, 18));

        auto updateIcon = [btn, iconName](bool checked) {
            QColor color = checked ? ThemeManager::instance().accentColor() : QColor("#A8998A");
            btn->setIcon(ThemeManager::instance().getIcon(iconName, color, "SettingsWindow"));
            };

        updateIcon(false);
        connect(btn, &QPushButton::toggled, btn, updateIcon);

        connect(btn, &QPushButton::clicked, this, [this, pageIndex, pageTitle]() {
            if (m_pages) m_pages->setCurrentIndex(pageIndex);
            if (m_pageTitleLabel) m_pageTitleLabel->setText(pageTitle);
            });

        return btn;
        };

    auto* accountLabel = new QLabel("АККАУНТ", sidebar);
    accountLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(accountLabel);
    sidebarLayout->addSpacing(4);

    auto* overviewBtn = createSidebarItem("Обзор", "review", 0, "Информация об аккаунте");
    overviewBtn->setChecked(true);
    sidebarLayout->addWidget(overviewBtn);

    auto* securityBtn = createSidebarItem("Безопасность", "security", 1, "Безопасность");
    sidebarLayout->addWidget(securityBtn);

    auto* notificationsBtn = createSidebarItem("Уведомления", "notifications", 2, "Уведомления");
    sidebarLayout->addWidget(notificationsBtn);

    auto* partnershipBtn = createSidebarItem("Сотрудничество", "partnership", 3, "Сотрудничество");
    sidebarLayout->addWidget(partnershipBtn);

    sidebarLayout->addSpacing(16);

    auto* subscriptionLabel = new QLabel("ПОДПИСКА", sidebar);
    subscriptionLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(subscriptionLabel);
    sidebarLayout->addSpacing(4);

    auto* subsBtn = createSidebarItem("Подписки", "subscriptions", 4, "Подписки");
    sidebarLayout->addWidget(subsBtn);

    auto* balanceBtn = createSidebarItem("Баланс", "balance", 5, "Баланс");
    sidebarLayout->addWidget(balanceBtn);

    sidebarLayout->addSpacing(16);

    auto* appLabel = new QLabel("ПРИЛОЖЕНИЕ", sidebar);
    appLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(appLabel);
    sidebarLayout->addSpacing(4);

    m_settingsSidebarBtn = createSidebarItem("Настройки", "settings", 6, "Настройки приложения");
    sidebarLayout->addWidget(m_settingsSidebarBtn);

    sidebarLayout->addStretch();

    auto* divider = new QFrame(sidebar);
    divider->setObjectName("sidebarDivider");
    divider->setFixedHeight(1);
    divider->setStyleSheet("background-color: #3A2F26; margin: 0px 5px;");
    sidebarLayout->addWidget(divider);
    sidebarLayout->addSpacing(10);

    auto* bottomAccountLabel = new QLabel("АККАУНТ", sidebar);
    bottomAccountLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(bottomAccountLabel);
    sidebarLayout->addSpacing(4);

    auto* userRowLayout = new QHBoxLayout();
    userRowLayout->setContentsMargins(0, 0, 0, 0);
    userRowLayout->setSpacing(6);

    m_sidebarAvatarLabel = new QLabel(sidebar);
    m_sidebarAvatarLabel->setFixedSize(32, 32);

    m_usernameLabel = new QLabel(m_userInfo.username, sidebar);
    m_usernameLabel->setObjectName("sidebarUsername");

    userRowLayout->addWidget(m_sidebarAvatarLabel);
    userRowLayout->addWidget(m_usernameLabel);
    userRowLayout->addStretch();

    sidebarLayout->addLayout(userRowLayout);

    middleLayout->addWidget(sidebar);

    // === ПРАВАЯ ЧАСТЬ ===
    auto* contentWidget = new QWidget(this);
    contentWidget->setObjectName("settingsContent");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    auto* headerPanel = new QWidget(this);
    headerPanel->setObjectName("contentHeaderPanel");
    headerPanel->setFixedHeight(60);
    headerPanel->setAttribute(Qt::WA_StyledBackground, true);

    auto* headerPanelLayout = new QHBoxLayout(headerPanel);
    headerPanelLayout->setContentsMargins(30, 0, 30, 0);

    m_pageTitleLabel = new QLabel("Информация об аккаунте", headerPanel);
    m_pageTitleLabel->setObjectName("pageTitle");

    headerPanelLayout->addWidget(m_pageTitleLabel);
    headerPanelLayout->addStretch();

    contentLayout->addWidget(headerPanel);

    m_pages = new QStackedWidget(this);
    m_pages->setObjectName("pagesStack");

    // =========================================================
    // === СТРАНИЦА 0: ОБЗОР ===
    // =========================================================
    auto* pageOverview = new QWidget();
    pageOverview->setObjectName("innerContent");
    auto* overviewLayout = new QVBoxLayout(pageOverview);
    overviewLayout->setContentsMargins(10, 15, 10, 15);
    overviewLayout->setSpacing(15);
    overviewLayout->setAlignment(Qt::AlignTop);

    // --- Панель профиля ---
    auto* profilePanel = new QWidget(pageOverview);
    profilePanel->setObjectName("profilePanel");
    profilePanel->setAttribute(Qt::WA_StyledBackground, true);

    auto* profileLayout = new QHBoxLayout(profilePanel);
    profileLayout->setContentsMargins(20, 20, 20, 20);
    profileLayout->setSpacing(20);

    m_avatarLabel = new QLabel(profilePanel);
    m_avatarLabel->setObjectName("avatarImage");
    m_avatarLabel->setFixedSize(64, 64);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(4);
    textLayout->setAlignment(Qt::AlignVCenter);

    m_accountNameLabel = new QLabel("Загрузка...", profilePanel);
    m_accountNameLabel->setObjectName("accountNameLabel");

    m_accountDetailsLabel = new QLabel("Загрузка...", profilePanel);
    m_accountDetailsLabel->setObjectName("accountDetailsLabel");

    textLayout->addWidget(m_accountNameLabel);
    textLayout->addWidget(m_accountDetailsLabel);

    auto* changeAvatarBtn = new QPushButton("Изменить аватар", profilePanel);
    changeAvatarBtn->setObjectName("outlineButton");
    changeAvatarBtn->setCursor(Qt::PointingHandCursor);
    changeAvatarBtn->setMinimumHeight(36);
    connect(changeAvatarBtn, &QPushButton::clicked, this, &SettingsWindow::onChangeAvatarClicked);

    profileLayout->addWidget(m_avatarLabel);
    profileLayout->addLayout(textLayout);
    profileLayout->addStretch();
    profileLayout->addWidget(changeAvatarBtn);

    overviewLayout->addWidget(profilePanel);

    // --- Ряд из двух панелей (выровнены по верху) ---
    auto* panelsRow = new QHBoxLayout();
    panelsRow->setSpacing(15);
    panelsRow->setContentsMargins(0, 0, 0, 0);

    // =========================================================
    // === ЛЕВАЯ ПАНЕЛЬ: ДАННЫЕ ДЛЯ ВХОДА ===
    // =========================================================
    auto* loginDataPanel = new QWidget(pageOverview);
    loginDataPanel->setObjectName("profilePanel");
    loginDataPanel->setAttribute(Qt::WA_StyledBackground, true);

    auto* loginDataLayout = new QVBoxLayout(loginDataPanel);
    loginDataLayout->setContentsMargins(20, 20, 20, 20);
    loginDataLayout->setSpacing(6);

    auto* loginDataTitle = new QLabel("Данные для входа", loginDataPanel);
    loginDataTitle->setObjectName("panelTitle");
    loginDataLayout->addWidget(loginDataTitle);
    loginDataLayout->addSpacing(6);

    auto* div1 = new QFrame(loginDataPanel);
    div1->setObjectName("dividerLine");
    div1->setFixedHeight(1);
    loginDataLayout->addWidget(div1);
    loginDataLayout->addSpacing(10);

    auto* loginLabel = new QLabel("ЛОГИН", loginDataPanel);
    loginLabel->setObjectName("inputLabel");
    loginDataLayout->addWidget(loginLabel);
    loginDataLayout->addSpacing(4);
    m_loginField = new QLineEdit(loginDataPanel);
    m_loginField->setReadOnly(true);
    loginDataLayout->addWidget(m_loginField);
    loginDataLayout->addSpacing(8);

    auto* emailLabel = new QLabel("ПОЧТА", loginDataPanel);
    emailLabel->setObjectName("inputLabel");
    loginDataLayout->addWidget(emailLabel);
    loginDataLayout->addSpacing(4);
    m_emailField = new QLineEdit(loginDataPanel);
    m_emailField->setPlaceholderText("Введите почту");
    loginDataLayout->addWidget(m_emailField);
    loginDataLayout->addSpacing(10);

    auto* div2 = new QFrame(loginDataPanel);
    div2->setObjectName("dividerLine");
    div2->setFixedHeight(1);
    loginDataLayout->addWidget(div2);
    loginDataLayout->addSpacing(10);

    auto* passChangeTitle = new QLabel("Смена пароля", loginDataPanel);
    passChangeTitle->setObjectName("panelSubtitle");
    loginDataLayout->addWidget(passChangeTitle);
    loginDataLayout->addSpacing(8);

    auto* currentPassLabel = new QLabel("ТЕКУЩИЙ ПАРОЛЬ", loginDataPanel);
    currentPassLabel->setObjectName("inputLabel");
    loginDataLayout->addWidget(currentPassLabel);
    loginDataLayout->addSpacing(4);
    m_currentPassField = new QLineEdit(loginDataPanel);
    m_currentPassField->setEchoMode(QLineEdit::Password);
    m_currentPassField->setPlaceholderText("Введите текущий пароль");
    loginDataLayout->addWidget(m_currentPassField);
    loginDataLayout->addSpacing(8);

    auto* newPassRow = new QHBoxLayout();
    newPassRow->setSpacing(8);

    auto* newPassCol = new QVBoxLayout();
    newPassCol->setSpacing(4);
    auto* newPassLabel = new QLabel("НОВЫЙ ПАРОЛЬ", loginDataPanel);
    newPassLabel->setObjectName("inputLabel");
    m_newPassField = new QLineEdit(loginDataPanel);
    m_newPassField->setEchoMode(QLineEdit::Password);
    m_newPassField->setPlaceholderText("Новый пароль");
    newPassCol->addWidget(newPassLabel);
    newPassCol->addWidget(m_newPassField);

    auto* confirmPassCol = new QVBoxLayout();
    confirmPassCol->setSpacing(4);
    auto* confirmPassLabel = new QLabel("ПОВТОРИТЕ ПАРОЛЬ", loginDataPanel);
    confirmPassLabel->setObjectName("inputLabel");
    m_confirmPassField = new QLineEdit(loginDataPanel);
    m_confirmPassField->setEchoMode(QLineEdit::Password);
    m_confirmPassField->setPlaceholderText("Повторите пароль");
    confirmPassCol->addWidget(confirmPassLabel);
    confirmPassCol->addWidget(m_confirmPassField);

    newPassRow->addLayout(newPassCol);
    newPassRow->addLayout(confirmPassCol);
    loginDataLayout->addLayout(newPassRow);
    loginDataLayout->addSpacing(10);

    auto* savePassBtn = new QPushButton("Сохранить пароль", loginDataPanel);
    savePassBtn->setObjectName("accentButton");
    savePassBtn->setMinimumHeight(40);
    savePassBtn->setCursor(Qt::PointingHandCursor);
    connect(savePassBtn, &QPushButton::clicked, this, &SettingsWindow::onSavePasswordClicked);
    loginDataLayout->addWidget(savePassBtn);

    panelsRow->addWidget(loginDataPanel, 1, Qt::AlignTop);

    // =========================================================
    // === ПРАВАЯ ПАНЕЛЬ: СПОСОБЫ ВХОДА ===
    // =========================================================
    auto* loginMethodsPanel = new QWidget(pageOverview);
    loginMethodsPanel->setObjectName("profilePanel");
    loginMethodsPanel->setAttribute(Qt::WA_StyledBackground, true);

    auto* loginMethodsLayout = new QVBoxLayout(loginMethodsPanel);
    loginMethodsLayout->setContentsMargins(20, 20, 20, 20);
    loginMethodsLayout->setSpacing(6);

    auto* methodsTitle = new QLabel("Способы входа", loginMethodsPanel);
    methodsTitle->setObjectName("panelTitle");
    loginMethodsLayout->addWidget(methodsTitle);
    loginMethodsLayout->addSpacing(6);

    auto* divM1 = new QFrame(loginMethodsPanel);
    divM1->setObjectName("dividerLine");
    divM1->setFixedHeight(1);
    loginMethodsLayout->addWidget(divM1);
    loginMethodsLayout->addSpacing(10);

    auto createMethodRow = [this, loginMethodsPanel](const QString& iconName, const QString& iconFolder,
        const QString& title, const QString& subtitle,
        const QString& badgeText, bool isActive) -> QWidget* {
            auto* row = new QWidget(loginMethodsPanel);
            row->setObjectName("methodRow"); // <--- ID для прозрачного фона
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(0, 6, 0, 6);
            rowLayout->setSpacing(8);        // <--- УМЕНЬШИЛИ С 15 ДО 8

            auto* icon = new QLabel(row);
            icon->setFixedSize(28, 28);
            QIcon qicon = ThemeManager::instance().getIcon(iconName, QColor("#A8998A"), iconFolder);
            if (!qicon.isNull()) icon->setPixmap(qicon.pixmap(24, 24));
            rowLayout->addWidget(icon, 0, Qt::AlignVCenter);

            auto* textLayout = new QVBoxLayout();
            textLayout->setSpacing(2);
            auto* titleLabel = new QLabel(title, row);
            titleLabel->setObjectName("methodTitle");
            auto* subtitleLabel = new QLabel(subtitle, row);
            subtitleLabel->setObjectName("methodSubtitle");
            textLayout->addWidget(titleLabel);
            textLayout->addWidget(subtitleLabel);
            rowLayout->addLayout(textLayout);

            rowLayout->addStretch();

            auto* badge = new QLabel(badgeText, row);
            badge->setObjectName(isActive ? "badgeActive" : "badgeInactive");
            rowLayout->addWidget(badge, 0, Qt::AlignVCenter);

            return row;
        };

    loginMethodsLayout->addWidget(createMethodRow("password", "SettingsWindow",
        "Пароль", "Основной способ входа", "НАСТРОЕН", true));
    loginMethodsLayout->addSpacing(4);
    auto* divM2 = new QFrame(loginMethodsPanel); divM2->setObjectName("dividerLine"); divM2->setFixedHeight(1);
    loginMethodsLayout->addWidget(divM2);
    loginMethodsLayout->addSpacing(4);

    loginMethodsLayout->addWidget(createMethodRow("discord", "",
        "Discord", "Уведомления в Discord", "НЕ ПОДКЛЮЧЕН", false));
    loginMethodsLayout->addSpacing(4);
    auto* divM3 = new QFrame(loginMethodsPanel); divM3->setObjectName("dividerLine"); divM3->setFixedHeight(1);
    loginMethodsLayout->addWidget(divM3);
    loginMethodsLayout->addSpacing(4);

    loginMethodsLayout->addWidget(createMethodRow("telegram", "",
        "Telegram", "Уведомления в Telegram", "НЕ ПОДКЛЮЧЕН", false));
    loginMethodsLayout->addSpacing(4);
    auto* divM4 = new QFrame(loginMethodsPanel); divM4->setObjectName("dividerLine"); divM4->setFixedHeight(1);
    loginMethodsLayout->addWidget(divM4);
    loginMethodsLayout->addSpacing(4);

    loginMethodsLayout->addWidget(createMethodRow("exbo", "SettingsWindow",
        "EXBO", "Подтверждает игрового персонажа", "НЕ ПОДКЛЮЧЕН", false));

    loginMethodsLayout->addSpacing(10);
    auto* divBottom = new QFrame(loginMethodsPanel); divBottom->setObjectName("dividerLine"); divBottom->setFixedHeight(1);
    loginMethodsLayout->addWidget(divBottom);
    loginMethodsLayout->addSpacing(10);

    auto* bindingsRow = new QHBoxLayout();
    bindingsRow->setContentsMargins(0, 0, 0, 0);
    bindingsRow->setSpacing(10);
    auto* bindingsLabel = new QLabel("Привязки находятся в Настройках", loginMethodsPanel);
    bindingsLabel->setObjectName("bindingsLabel");
    auto* openSettingsBtn = new QPushButton("Открыть", loginMethodsPanel);
    openSettingsBtn->setObjectName("outlineButton");
    openSettingsBtn->setMinimumHeight(32);
    openSettingsBtn->setMinimumWidth(100);
    openSettingsBtn->setCursor(Qt::PointingHandCursor);
    connect(openSettingsBtn, &QPushButton::clicked, this, [this]() {
        if (m_settingsSidebarBtn) m_settingsSidebarBtn->setChecked(true);
        if (m_pages) m_pages->setCurrentIndex(6);
        if (m_pageTitleLabel) m_pageTitleLabel->setText("Настройки приложения");
        });
    bindingsRow->addWidget(bindingsLabel);
    bindingsRow->addStretch();
    bindingsRow->addWidget(openSettingsBtn);
    loginMethodsLayout->addLayout(bindingsRow);

    panelsRow->addWidget(loginMethodsPanel, 1, Qt::AlignTop);

    overviewLayout->addLayout(panelsRow);
    m_pages->addWidget(pageOverview);

    // Заглушки для страниц 1-6
    for (int i = 1; i <= 6; ++i) {
        auto* page = new QWidget();
        page->setObjectName("innerContent");
        m_pages->addWidget(page);
    }

    contentLayout->addWidget(m_pages, 1);
    middleLayout->addWidget(contentWidget, 1);
    mainLayout->addLayout(middleLayout, 1);

    // === НИЖНЯЯ ПАНЕЛЬ ===
    auto* bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    bottomBar->setFixedHeight(60);
    bottomBar->setAttribute(Qt::WA_StyledBackground, true);

    auto* bottomBarLayout = new QHBoxLayout(bottomBar);
    bottomBarLayout->setContentsMargins(50, 0, 50, 0);
    bottomBarLayout->setSpacing(20);

    auto* privacyBtn = new QPushButton("Политика конфиденциальности", bottomBar);
    privacyBtn->setObjectName("footerLink");
    privacyBtn->setCursor(Qt::PointingHandCursor);
    connect(privacyBtn, &QPushButton::clicked, this, [this]() {
        Toast::showMessage(this, "Политика конфиденциальности (в разработке)", false);
        });

    auto* termsBtn = new QPushButton("Пользовательское соглашение", bottomBar);
    termsBtn->setObjectName("footerLink");
    termsBtn->setCursor(Qt::PointingHandCursor);
    connect(termsBtn, &QPushButton::clicked, this, [this]() {
        Toast::showMessage(this, "Пользовательское соглашение (в разработке)", false);
        });

    auto* discordBtn = new QToolButton(bottomBar);
    discordBtn->setObjectName("btnDiscord");
    discordBtn->setIcon(ThemeManager::instance().getIcon("discord", QColor("#FFFFFF"), ""));
    discordBtn->setIconSize(QSize(24, 24));
    discordBtn->setFixedSize(40, 40);
    discordBtn->setCursor(Qt::PointingHandCursor);
    connect(discordBtn, &QToolButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://discord.gg/zYsh9xPefd"));
        });

    bottomBarLayout->addWidget(privacyBtn);
    bottomBarLayout->addWidget(termsBtn);
    bottomBarLayout->addStretch();
    bottomBarLayout->addWidget(discordBtn, 0, Qt::AlignVCenter);

    mainLayout->addWidget(bottomBar);
}

void SettingsWindow::onChangeAvatarClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите аватар", "", "Изображения (*.png *.jpg *.jpeg)");
    if (fileName.isEmpty()) return;

    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString avatarDir = appDataPath + "/avatars";
    QDir().mkpath(avatarDir);

    QString destPath = QString("%1/user_%2.png").arg(avatarDir).arg(m_userInfo.id);

    QFile::remove(destPath);

    if (QFile::copy(fileName, destPath)) {
        Toast::showMessage(this, "Аватар успешно обновлен!", false);
        loadAvatar();
    }
    else {
        Toast::showMessage(this, "Ошибка при сохранении аватара", true);
    }
}

void SettingsWindow::onSavePasswordClicked()
{
    QString currentPass = m_currentPassField->text();
    QString newPass = m_newPassField->text();
    QString confirmPass = m_confirmPassField->text();

    if (currentPass.isEmpty() || newPass.isEmpty() || confirmPass.isEmpty()) {
        Toast::showMessage(this, "Заполните все поля пароля", true);
        return;
    }

    if (!DBManager::instance().authenticateUser(m_userInfo.username, currentPass)) {
        Toast::showMessage(this, "Неверный текущий пароль", true);
        return;
    }

    QRegularExpression passRegex("^[a-zA-Z0-9!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?~`]{12,}$");
    if (!passRegex.match(newPass).hasMatch()) {
        Toast::showMessage(this, "Пароль должен быть от 12 символов (латиница, цифры, спецсимволы)", true);
        return;
    }

    if (newPass != confirmPass) {
        Toast::showMessage(this, "Пароли не совпадают", true);
        return;
    }

    if (DBManager::instance().updatePassword(m_userInfo.username, newPass)) {
        Toast::showMessage(this, "Пароль успешно изменён", false);
        m_currentPassField->clear();
        m_newPassField->clear();
        m_confirmPassField->clear();
    }
    else {
        Toast::showMessage(this, "Ошибка при смене пароля", true);
    }
}

void SettingsWindow::loadAvatar()
{
    if (!m_avatarLabel || !m_sidebarAvatarLabel) return;

    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString destPath = QString("%1/avatars/user_%2.png").arg(appDataPath).arg(m_userInfo.id);

    QPixmap sourcePixmap;
    if (QFile::exists(destPath)) {
        sourcePixmap.load(destPath);
    }
    else {
        QIcon defaultIcon = ThemeManager::instance().getIcon("profileBtnOn", QColor("#A8998A"), "MainWindow");
        sourcePixmap = defaultIcon.pixmap(128, 128);
    }

    m_avatarLabel->setPixmap(getCircularPixmap(sourcePixmap, 64));
    m_sidebarAvatarLabel->setPixmap(getCircularPixmap(sourcePixmap, 32));
}

QPixmap SettingsWindow::getCircularPixmap(const QPixmap& source, int size)
{
    QPixmap target(size, size);
    target.fill(Qt::transparent);

    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int x = (scaled.width() - size) / 2;
    int y = (scaled.height() - size) / 2;
    painter.drawPixmap(0, 0, scaled, x, y, size, size);

    painter.setClipping(false);
    QPen pen(ThemeManager::instance().accentColor());
    pen.setWidthF(1.0);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    QRectF borderRect(0.5, 0.5, size - 1.0, size - 1.0);
    painter.drawEllipse(borderRect);

    return target;
}

void SettingsWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    setUserInfo(m_userInfo);
}
