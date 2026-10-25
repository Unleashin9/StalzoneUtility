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
    if (m_usernameLabel) {
        m_usernameLabel->setText(m_userInfo.username); // <--- Обновляем имя в сайдбаре
    }
}

void SettingsWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // =========================================================
    // === ВЕРХНЯЯ ПАНЕЛЬ ===
    // =========================================================
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
        "background-color: #1F1A16;"
        "color: #E5533D;"
        "border: 1px solid #E5533D;"
        "border-radius: 6px;"
        "padding: 4px 10px;"
        "font-size: 12px;"
        "font-weight: bold;"
    );

    topBarLayout->addWidget(logoBtn, 0, Qt::AlignVCenter);
    topBarLayout->addWidget(m_userInfoLabel, 0, Qt::AlignVCenter);
    topBarLayout->addStretch();
    topBarLayout->addWidget(emailStatusLabel, 0, Qt::AlignVCenter);

    mainLayout->addWidget(topBar);

    // =========================================================
    // === СРЕДНЯЯ ЗОНА (САЙДБАР + КОНТЕНТ) ===
    // =========================================================
    auto* middleLayout = new QHBoxLayout();
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(0);

    // --- ЛЕВЫЙ САЙДБАР ---
    auto* sidebar = new QWidget(this);
    sidebar->setObjectName("settingsSidebar");
    sidebar->setFixedWidth(220);
    sidebar->setAttribute(Qt::WA_StyledBackground, true);

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(15, 20, 15, 20);
    sidebarLayout->setSpacing(4);

    auto createSidebarItem = [this, sidebar](const QString& text, const QString& iconName) -> QPushButton* {
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

        return btn;
        };

    // --- СЕКЦИЯ "АККАУНТ" ---
    auto* accountLabel = new QLabel("АККАУНТ", sidebar);
    accountLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(accountLabel);
    sidebarLayout->addSpacing(4);

    auto* overviewBtn = createSidebarItem("Обзор", "review");
    overviewBtn->setChecked(true);
    sidebarLayout->addWidget(overviewBtn);

    auto* securityBtn = createSidebarItem("Безопасность", "security");
    sidebarLayout->addWidget(securityBtn);

    auto* notificationsBtn = createSidebarItem("Уведомления", "notifications");
    sidebarLayout->addWidget(notificationsBtn);

    auto* partnershipBtn = createSidebarItem("Сотрудничество", "partnership");
    sidebarLayout->addWidget(partnershipBtn);

    sidebarLayout->addSpacing(16);

    // --- СЕКЦИЯ "ПОДПИСКА" ---
    auto* subscriptionLabel = new QLabel("ПОДПИСКА", sidebar);
    subscriptionLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(subscriptionLabel);
    sidebarLayout->addSpacing(4);

    auto* subsBtn = createSidebarItem("Подписки", "subscriptions");
    sidebarLayout->addWidget(subsBtn);

    auto* balanceBtn = createSidebarItem("Баланс", "balance");
    sidebarLayout->addWidget(balanceBtn);

    sidebarLayout->addSpacing(16);

    // --- СЕКЦИЯ "ПРИЛОЖЕНИЕ" ---
    auto* appLabel = new QLabel("ПРИЛОЖЕНИЕ", sidebar);
    appLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(appLabel);
    sidebarLayout->addSpacing(4);

    auto* settingsBtn = createSidebarItem("Настройки", "settings");
    sidebarLayout->addWidget(settingsBtn);

    // --- НИЖНЯЯ ЧАСТЬ САЙДБАРА (АККАУНТ) ---
    sidebarLayout->addStretch(); // Распирает всё, что выше, наверх

    auto* bottomAccountLabel = new QLabel("АККАУНТ", sidebar);
    bottomAccountLabel->setObjectName("sidebarSectionTitle");
    sidebarLayout->addWidget(bottomAccountLabel);
    sidebarLayout->addSpacing(4);

    m_usernameLabel = new QLabel(m_userInfo.username, sidebar);
    m_usernameLabel->setObjectName("sidebarUsername");
    sidebarLayout->addWidget(m_usernameLabel);

    middleLayout->addWidget(sidebar);

    // --- КОНТЕНТ (пока пустой) ---
    auto* contentWidget = new QWidget(this);
    contentWidget->setObjectName("settingsContent");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setAlignment(Qt::AlignTop);

    middleLayout->addWidget(contentWidget, 1);

    mainLayout->addLayout(middleLayout, 1);

    // =========================================================
    // === НИЖНЯЯ ПАНЕЛЬ ===
    // =========================================================
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

void SettingsWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    setUserInfo(m_userInfo);
}
