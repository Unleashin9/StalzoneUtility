#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>
#include <QApplication>
#include <QIcon>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSettings>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QMouseEvent>
#include <QEvent>

#include "api/StalzoneAPI.h"
#include "core/GameDatabase.h"
#include "core/ThemeManager.h"
#include "core/DBManager.h"
#include "core/AuthSettings.h"
#include "ui/pages/AuctionWindow.h"
#include "ui/pages/BarterWindow.h"
#include "ui/pages/ItemsWindow.h"
#include "ui/auth/LoginWindow.h"
#include "ui/auth/RegisterWindow.h"
#include "ui/widgets/ProfileMenu.h"
#include "ui/widgets/Toast.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setWindowTitle("STALZONE Utility");
    resize(1200, 750);

    DBManager::instance().init();

    m_api = new StalzoneAPI(this);
    m_db = new GameDatabase(this);
    m_db->load();

    m_loginWindow = new LoginWindow(this);
    m_registerWindow = new RegisterWindow(this);

    setupUi();

    // Создаём меню профиля после setupUi, чтобы m_profileBtn был готов
    m_profileMenu = new ProfileMenu(this);

    connect(m_profileMenu, &ProfileMenu::settingsUserClicked, this, [this]() {
        hideProfileMenu();
        Toast::showMessage(this, "Настройки пользователя (в разработке)", false);
        });
    connect(m_profileMenu, &ProfileMenu::settingsAppClicked, this, [this]() {
        hideProfileMenu();
        Toast::showMessage(this, "Настройки приложения (в разработке)", false);
        });
    connect(m_profileMenu, &ProfileMenu::logoutClicked, this, [this]() {
        hideProfileMenu();
        onLogout();
        });

    // Esc — скрыть меню, если открыто
    auto* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [this]() {
        if (m_profileMenu && m_profileMenu->isVisible()) {
            hideProfileMenu();
        }
        });

    // Клик вне меню — скрыть
    qApp->installEventFilter(this);

    connect(m_loginWindow, &LoginWindow::loginSuccessful, this, &MainWindow::onLoginSuccess);
    connect(m_loginWindow, &LoginWindow::registerRequested, this, &MainWindow::onShowRegisterWindow);
    connect(m_loginWindow, &LoginWindow::windowClosed, this, &MainWindow::removeBlurEffect);

    connect(m_registerWindow, &RegisterWindow::registrationSuccessful, this, &MainWindow::onShowLoginWindow);
    connect(m_registerWindow, &RegisterWindow::loginRequested, this, &MainWindow::onShowLoginWindow);
    connect(m_registerWindow, &RegisterWindow::windowClosed, this, &MainWindow::removeBlurEffect);

    QSettings settings;
    if (settings.value(AuthSettings::kRememberMe, false).toBool()) {
        QString savedUser = settings.value(AuthSettings::kUsername).toString();
        if (!savedUser.isEmpty() && DBManager::instance().userExists(savedUser)) {
            onLoginSuccess(savedUser);
        }
    }
}

MainWindow::~MainWindow() = default;

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (m_profileMenu && m_profileMenu->isVisible() && m_profileBtn) {
            auto* me = static_cast<QMouseEvent*>(event);
            QPoint globalPos = me->globalPosition().toPoint();

            QRect menuRect(m_profileMenu->mapToGlobal(QPoint(0, 0)), m_profileMenu->size());
            QRect btnRect(m_profileBtn->mapToGlobal(QPoint(0, 0)), m_profileBtn->size());

            if (!menuRect.contains(globalPos) && !btnRect.contains(globalPos)) {
                hideProfileMenu();
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    central->setObjectName("mainContainer");
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // === ВЕРХНЯЯ ПАНЕЛЬ ===
    auto* topLayout = new QHBoxLayout();
    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    auto* appTitle = new QLabel("STALZONE Utility", this);
    appTitle->setObjectName("appTitle");
    auto* appSubtitle = new QLabel("Твой цифровой артефакт и помощник", this);
    appSubtitle->setObjectName("appSubtitle");

    titleLayout->addWidget(appTitle);
    titleLayout->addWidget(appSubtitle);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);
    btnLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_profileBtn = new QPushButton("ВОЙТИ | ЗАРЕГИСТРИРОВАТЬСЯ", this);
    m_profileBtn->setObjectName("btnLogin");
    m_profileBtn->setIcon(ThemeManager::instance().getIcon("profileBtnOff", QColor("#000000")));
    m_profileBtn->setIconSize(QSize(20, 20));
    m_profileBtn->setCursor(Qt::PointingHandCursor);
    m_profileBtn->setMinimumHeight(40);
    m_profileBtn->setFixedWidth(320);
    connect(m_profileBtn, &QPushButton::clicked, this, &MainWindow::onProfileButtonClicked);

    auto* btnExit = new QToolButton(this);
    btnExit->setObjectName("btnExit");
    btnExit->setIcon(ThemeManager::instance().getIcon("exitBtn", QColor("#000000")));
    btnExit->setIconSize(QSize(24, 24));
    btnExit->setFixedSize(40, 40);
    btnExit->setCursor(Qt::PointingHandCursor);
    connect(btnExit, &QToolButton::clicked, qApp, &QApplication::quit);

    btnLayout->addWidget(m_profileBtn);
    btnLayout->addWidget(btnExit);

    topLayout->addLayout(titleLayout);
    topLayout->addStretch();
    topLayout->addLayout(btnLayout);

    // === ЦЕНТРАЛЬНАЯ ПАНЕЛЬ (КАРТОЧКИ) ===
    auto* centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(30);
    centerLayout->setAlignment(Qt::AlignCenter);

    auto* btnAuction = new QPushButton("Аукцион", this);
    btnAuction->setObjectName("btnAuctionCard");
    btnAuction->setFixedSize(220, 180);
    btnAuction->setCursor(Qt::PointingHandCursor);
    QIcon auctionIcon = ThemeManager::instance().getIcon("auctionCard", QColor("#A3E635"));
    if (!auctionIcon.isNull()) {
        btnAuction->setIcon(auctionIcon);
        btnAuction->setIconSize(QSize(48, 48));
    }
    connect(btnAuction, &QPushButton::clicked, this, &MainWindow::onAuctionClicked);

    auto* btnBarter = new QPushButton("Калькулятор", this);
    btnBarter->setObjectName("btnBarterCard");
    btnBarter->setFixedSize(220, 180);
    btnBarter->setCursor(Qt::PointingHandCursor);
    QIcon barterIcon = ThemeManager::instance().getIcon("barterCard", QColor("#A3E635"));
    if (!barterIcon.isNull()) {
        btnBarter->setIcon(barterIcon);
        btnBarter->setIconSize(QSize(48, 48));
    }
    connect(btnBarter, &QPushButton::clicked, this, &MainWindow::onBarterClicked);

    auto* btnItems = new QPushButton("База предметов", this);
    btnItems->setObjectName("btnItemsCard");
    btnItems->setFixedSize(220, 180);
    btnItems->setCursor(Qt::PointingHandCursor);
    QIcon itemsIcon = ThemeManager::instance().getIcon("itemsCard", QColor("#A3E635"));
    if (!itemsIcon.isNull()) {
        btnItems->setIcon(itemsIcon);
        btnItems->setIconSize(QSize(48, 48));
    }
    connect(btnItems, &QPushButton::clicked, this, &MainWindow::onItemsClicked);

    centerLayout->addWidget(btnAuction);
    centerLayout->addWidget(btnBarter);
    centerLayout->addWidget(btnItems);

    // === НИЖНЯЯ ПАНЕЛЬ ===
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    auto* btnDiscord = new QToolButton(this);
    btnDiscord->setObjectName("btnDiscord");
    btnDiscord->setIcon(ThemeManager::instance().getIcon("discordBtn", QColor("#FFFFFF")));
    btnDiscord->setIconSize(QSize(24, 24));
    btnDiscord->setFixedSize(40, 40);
    btnDiscord->setCursor(Qt::PointingHandCursor);
    bottomLayout->addWidget(btnDiscord);

    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);
}

// =========================================================
// МЕНЮ ПРОФИЛЯ
// =========================================================

void MainWindow::showProfileMenu()
{
    if (!m_profileMenu || !m_profileBtn) return;

    // Сначала показываем меню, чтобы Qt применила QSS и посчитала финальный размер
    m_profileMenu->show();
    m_profileMenu->raise();
    m_profileMenu->adjustSize();

    int menuWidth = m_profileMenu->width();
    int menuHeight = m_profileMenu->height();

    // Точка (центр нижней грани кнопки) в глобальных координатах
    QPoint globalAnchor = m_profileBtn->mapToGlobal(
        QPoint(m_profileBtn->width() / 2, m_profileBtn->height()));

    // Переводим в координаты MainWindow
    QPoint localAnchor = mapFromGlobal(globalAnchor);

    // Верхний левый угол меню: по центру кнопки
    int x = localAnchor.x() - menuWidth / 2;
    int y = localAnchor.y() + 8;

    // Ограничиваем по границам окна
    if (x < 10) x = 10;
    if (x + menuWidth > width() - 10) x = width() - menuWidth - 10;
    if (y + menuHeight > height() - 10) y = height() - menuHeight - 10;

    QPoint endPos(x, y);
    QPoint startPos(x, y - 8);

    // Резкое перемещение в стартовую позицию, чтобы не было «прыжка»
    m_profileMenu->move(startPos);

    // Диагностика (потом уберём)
    qDebug() << "=== PROFILE MENU DEBUG ===";
    qDebug() << "Btn width:" << m_profileBtn->width() << "Btn x:" << m_profileBtn->x();
    qDebug() << "Global anchor:" << globalAnchor;
    qDebug() << "Local anchor:" << localAnchor;
    qDebug() << "Menu size:" << menuWidth << "x" << menuHeight;
    qDebug() << "MainWindow size:" << width() << "x" << height();
    qDebug() << "Final pos:" << endPos;

    // Анимация позиции
    auto* posAnim = new QPropertyAnimation(m_profileMenu, "pos", m_profileMenu);
    posAnim->setDuration(200);
    posAnim->setStartValue(startPos);
    posAnim->setEndValue(endPos);
    posAnim->setEasingCurve(QEasingCurve::OutCubic);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);

    // Анимация прозрачности
    auto* effect = new QGraphicsOpacityEffect(m_profileMenu);
    effect->setOpacity(0.0);
    m_profileMenu->setGraphicsEffect(effect);

    auto* fadeAnim = new QPropertyAnimation(effect, "opacity", m_profileMenu);
    fadeAnim->setDuration(200);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::hideProfileMenu()
{
    if (!m_profileMenu || !m_profileMenu->isVisible()) return;

    QPoint currentPos = m_profileMenu->pos();
    QPoint endPos(currentPos.x(), currentPos.y() - 6);

    auto* posAnim = new QPropertyAnimation(m_profileMenu, "pos", m_profileMenu);
    posAnim->setDuration(150);
    posAnim->setStartValue(currentPos);
    posAnim->setEndValue(endPos);
    posAnim->setEasingCurve(QEasingCurve::InCubic);

    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(m_profileMenu->graphicsEffect());
    if (effect) {
        auto* fadeAnim = new QPropertyAnimation(effect, "opacity", m_profileMenu);
        fadeAnim->setDuration(150);
        fadeAnim->setStartValue(effect->opacity());
        fadeAnim->setEndValue(0.0);
        fadeAnim->setEasingCurve(QEasingCurve::InCubic);
        connect(fadeAnim, &QPropertyAnimation::finished, this, [this]() {
            m_profileMenu->hide();
            m_profileMenu->setGraphicsEffect(nullptr);
            });
        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else {
        connect(posAnim, &QPropertyAnimation::finished, m_profileMenu, &QWidget::hide);
    }

    posAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

// =========================================================
// БЛОКИРОВКА / РАЗБЛОКИРОВКА MAINWINDOW
// =========================================================

void MainWindow::blockMainWindow()
{
    if (centralWidget()) centralWidget()->setEnabled(false);
}

void MainWindow::unblockMainWindow()
{
    if (centralWidget()) centralWidget()->setEnabled(true);
}

// =========================================================
// ЛОГИКА АВТОРИЗАЦИИ
// =========================================================

void MainWindow::positionDialog(QWidget* dialog)
{
    int margin = 50;
    int x = this->x() + this->width() - dialog->width() - margin;
    int y = this->y() + (this->height() - dialog->height()) / 2;
    dialog->move(x, y);
}

void MainWindow::onProfileButtonClicked()
{
    if (m_currentUser.isEmpty()) {
        m_registerWindow->hide();
        applyBlurEffect();
        blockMainWindow();
        positionDialog(m_loginWindow);
        m_loginWindow->show();
        m_loginWindow->raise();
        m_loginWindow->activateWindow();
        m_loginWindow->setFocus();
    }
    else {
        if (m_profileMenu && m_profileMenu->isVisible()) {
            hideProfileMenu();
        }
        else {
            showProfileMenu();
        }
    }
}

void MainWindow::onLoginSuccess(const QString& username)
{
    m_currentUser = username;
    updateProfileButton();
    m_loginWindow->hide();
    removeBlurEffect();
    unblockMainWindow();
}

void MainWindow::onShowRegisterWindow()
{
    m_loginWindow->hide();
    m_registerWindow->clearFields();
    positionDialog(m_registerWindow);
    m_registerWindow->show();
    m_registerWindow->raise();
    m_registerWindow->activateWindow();
    m_registerWindow->setFocus();
}

void MainWindow::onShowLoginWindow()
{
    m_registerWindow->hide();
    positionDialog(m_loginWindow);
    m_loginWindow->show();
    m_loginWindow->raise();
    m_loginWindow->activateWindow();
    m_registerWindow->setFocus();
}

void MainWindow::onLogout()
{
    m_currentUser.clear();
    QSettings settings;
    settings.setValue(AuthSettings::kRememberMe, false);
    settings.remove(AuthSettings::kUsername);
    updateProfileButton();
}

void MainWindow::updateProfileButton()
{
    if (m_currentUser.isEmpty()) {
        m_profileBtn->setText("ВОЙТИ | ЗАРЕГИСТРИРОВАТЬСЯ");
        m_profileBtn->setIcon(ThemeManager::instance().getIcon("profileBtnOff", QColor("#000000")));
    }
    else {
        m_profileBtn->setText("ЛИЧНЫЙ КАБИНЕТ");
        m_profileBtn->setIcon(ThemeManager::instance().getIcon("profileBtnOn", QColor("#000000")));
    }
}

void MainWindow::applyBlurEffect()
{
    auto* blur = new QGraphicsBlurEffect(this);
    blur->setBlurRadius(10);
    centralWidget()->setGraphicsEffect(blur);
}

void MainWindow::removeBlurEffect()
{
    centralWidget()->setGraphicsEffect(nullptr);
    unblockMainWindow();
}

// =========================================================
// ПЕРЕКЛЮЧЕНИЕ ОКОН РАЗДЕЛОВ
// =========================================================

void MainWindow::showSubWindow(QWidget* window)
{
    window->showFullScreen();
    window->raise();
    window->activateWindow();
    hide();
}

void MainWindow::onAuctionClicked()
{
    if (!m_auctionWindow) {
        m_auctionWindow = std::make_unique<AuctionWindow>(m_api, m_db);
        connect(m_auctionWindow.get(), &AuctionWindow::backRequested, this, &MainWindow::onBackToMain);
    }
    showSubWindow(m_auctionWindow.get());
}

void MainWindow::onBarterClicked()
{
    if (!m_barterWindow) {
        m_barterWindow = std::make_unique<BarterWindow>(m_db);
        connect(m_barterWindow.get(), &BarterWindow::backRequested, this, &MainWindow::onBackToMain);
    }
    showSubWindow(m_barterWindow.get());
}

void MainWindow::onItemsClicked()
{
    if (!m_itemsWindow) {
        m_itemsWindow = std::make_unique<ItemsWindow>(m_db);
        connect(m_itemsWindow.get(), &ItemsWindow::backRequested, this, &MainWindow::onBackToMain);
    }
    showSubWindow(m_itemsWindow.get());
}

void MainWindow::onBackToMain()
{
    if (m_auctionWindow) m_auctionWindow->hide();
    if (m_barterWindow)  m_barterWindow->hide();
    if (m_itemsWindow)   m_itemsWindow->hide();

    showFullScreen();
    raise();
    activateWindow();
}
