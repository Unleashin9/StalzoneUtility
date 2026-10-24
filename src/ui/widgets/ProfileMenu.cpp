#include "ProfileMenu.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QIcon>

ProfileMenu::ProfileMenu(QWidget* parent) : QWidget(parent)
{
    setObjectName("profileMenu");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
    hide();
}

void ProfileMenu::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(2);

    auto createBtn = [this](const QString& text, const QString& iconName) {
        auto* btn = new QPushButton(this);
        btn->setObjectName("profileMenuItem");
        btn->setText("  " + text);
        btn->setIcon(ThemeManager::instance().getIcon(iconName, QColor("#9CA3AF")));
        btn->setIconSize(QSize(18, 18));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(38);
        return btn;
        };

    m_settingsUserBtn = createBtn("Настройки пользователя", "settings");
    m_settingsAppBtn = createBtn("Настройки приложения", "settings");
    m_logoutBtn = createBtn("Выйти из аккаунта", "logout");

    m_layout->addWidget(m_settingsUserBtn);
    m_layout->addWidget(m_settingsAppBtn);
    m_layout->addWidget(m_logoutBtn);

    connect(m_settingsUserBtn, &QPushButton::clicked, this, &ProfileMenu::settingsUserClicked);
    connect(m_settingsAppBtn, &QPushButton::clicked, this, &ProfileMenu::settingsAppClicked);
    connect(m_logoutBtn, &QPushButton::clicked, this, &ProfileMenu::logoutClicked);

    setFixedWidth(260);
    adjustSize();
}

void ProfileMenu::setUserName(const QString& name)
{
    Q_UNUSED(name);
}
