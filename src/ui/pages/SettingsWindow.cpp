#include "SettingsWindow.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>

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
}

void SettingsWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(60);
    topBar->setAttribute(Qt::WA_StyledBackground, true);

    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(10, 0, 10, 0);
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

    auto* contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setAlignment(Qt::AlignTop);

    mainLayout->addWidget(contentWidget);
}

void SettingsWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    setUserInfo(m_userInfo);
}
