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
#include "ui/pages/SettingsWindow.h"
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

    m_profileMenu = new ProfileMenu(this);

    connect(m_profileMenu, &ProfileMenu::settingsUserClicked, this, &MainWindow::onOpenUserSettings);
    connect(m_profileMenu, &ProfileMenu::settingsAppClicked, this, [this]() {
        hideProfileMenu();
        Toast::showMessage(this, "Настройки приложения (в разработке)", false);
        });
    connect(m_profileMenu, &ProfileMenu::logoutClicked, this, [this]() {
        hideProfileMenu();
        onLogout();
        });

    auto* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [this]() {
        if (m_profileMenu && m_profileMenu->isVisible()) hideProfileMenu();
        });

    qApp->installEventFilter(this);

    connect(m_loginWindow, &LoginWindow::loginSuccessful, this, &MainWindow::onLoginSuccess);
    connect(m_loginWindow, &LoginWindow::registerRequested, this, &MainWindow::onShowRegisterWindow);
    connect(m_loginWindow, &LoginWindow::windowClosed, this, &MainWindow::removeBlurEffect);

    connect(m_registerWindow, &RegisterWindow::registrationSuccessful, this, &MainWindow::onShowLoginWindow);
    connect(m_registerWindow, &RegisterWindow::loginRequested, this, &MainWindow::onShowLoginWindow);
    connect(m_registerWindow, &RegisterWindow::windowClosed, this, &MainWindow::removeBlurEffect);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &MainWindow::updateIcons);

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

    // ГЛАВНЫЙ ВЕРТИКАЛЬНЫЙ СЛОЙ (TopBar -> Контент -> BottomBar)
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === 1. ВЕРХНЯЯ ПАНЕЛЬ ===
    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(60);
    topBar->setAttribute(Qt::WA_StyledBackground, true);

    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(50, 0, 50, 0);
    topBarLayout->setSpacing(15);

    auto* logoBtn = new QPushButton("STALZONE Utility", topBar);
    logoBtn->setObjectName("logoBtn");
    logoBtn->setIcon(ThemeManager::instance().getIcon("szutility", ThemeManager::instance().accentColor(), QString()));
    logoBtn->setIconSize(QSize(32, 32));
    logoBtn->setCursor(Qt::PointingHandCursor);
    logoBtn->setFlat(true);
    logoBtn->setFocusPolicy(Qt::NoFocus);
    logoBtn->setStyleSheet("background: transparent; border: none;");
    connect(logoBtn, &QPushButton::clicked, this, &MainWindow::onBackToMain);

    auto* btnWidget = new QWidget(topBar);
    auto* btnLayout = new QHBoxLayout(btnWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(10);

    m_profileBtn = new QPushButton("ВОЙТИ | ЗАРЕГИСТРИРОВАТЬСЯ", btnWidget);
    m_profileBtn->setObjectName("btnLogin");
    m_profileBtn->setIcon(ThemeManager::instance().getIcon("profileBtnOff", QColor("#000000"), "MainWindow"));
    m_profileBtn->setIconSize(QSize(18, 18));
    m_profileBtn->setCursor(Qt::PointingHandCursor);
    m_profileBtn->setFixedSize(300, 36);
    connect(m_profileBtn, &QPushButton::clicked, this, &MainWindow::onProfileButtonClicked);

    auto* btnExit = new QToolButton(btnWidget);
    btnExit->setObjectName("btnExit");
    btnExit->setIcon(ThemeManager::instance().getIcon("exitBtn", QColor("#000000"), "MainWindow"));
    btnExit->setIconSize(QSize(20, 20));
    btnExit->setFixedSize(36, 36);
    btnExit->setCursor(Qt::PointingHandCursor);
    connect(btnExit, &QToolButton::clicked, qApp, &QApplication::quit);

    btnLayout->addWidget(m_profileBtn);
    btnLayout->addWidget(btnExit);

    topBarLayout->addWidget(logoBtn, 0, Qt::AlignVCenter);
    topBarLayout->addStretch();
    topBarLayout->addWidget(btnWidget, 0, Qt::AlignVCenter);

    mainLayout->addWidget(topBar);

    // === 2. СРЕДНЯЯ ЧАСТЬ (Sidebar + Контент) ===
    auto* middleWidget = new QWidget(this);
    auto* middleLayout = new QHBoxLayout(middleWidget);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(0);

    // --- Левый Sidebar ---
    auto* sidebar = new QWidget(this);
    sidebar->setObjectName("mainSidebar");
    sidebar->setFixedWidth(240);
    sidebar->setAttribute(Qt::WA_StyledBackground, true);

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(15, 20, 15, 20);
    sidebarLayout->setSpacing(4);

    auto* auctionBtn = new QPushButton(" Аукцион", sidebar);
    auctionBtn->setObjectName("mainSidebarBtn");
    auctionBtn->setIcon(ThemeManager::instance().getIcon("auction", QColor("#A8998A"), "MainWindow"));
    auctionBtn->setIconSize(QSize(18, 18));
    auctionBtn->setCursor(Qt::PointingHandCursor);
    auctionBtn->setMinimumHeight(40);
    connect(auctionBtn, &QPushButton::clicked, this, &MainWindow::onAuctionClicked);

    sidebarLayout->addWidget(auctionBtn);
    sidebarLayout->addStretch(); // Прижимаем кнопки к верху

    middleLayout->addWidget(sidebar);

    // --- Правая часть (Контент) ---
    auto* contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);
    contentLayout->setAlignment(Qt::AlignCenter);

    auto* welcomeLabel = new QLabel("Добро пожаловать в STALZONE Utility!", contentWidget);
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #A8998A;");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(welcomeLabel);

    middleLayout->addWidget(contentWidget, 1);

    mainLayout->addWidget(middleWidget, 1);

    // === 3. НИЖНЯЯ ПАНЕЛЬ ===
    auto* bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    bottomBar->setFixedHeight(60);
    bottomBar->setAttribute(Qt::WA_StyledBackground, true);

    auto* bottomBarLayout = new QHBoxLayout(bottomBar);
    bottomBarLayout->setContentsMargins(50, 0, 50, 0);
    bottomBarLayout->setSpacing(20);

    auto* versionLabel = new QLabel("STALZONE Utility v0.1.0", bottomBar);
    versionLabel->setStyleSheet("color: #4A3F36; font-size: 11px;");

    auto* btnDiscord = new QToolButton(bottomBar);
    btnDiscord->setObjectName("btnDiscord");
    btnDiscord->setIcon(ThemeManager::instance().getIcon("discord", QColor("#FFFFFF"), ""));
    btnDiscord->setIconSize(QSize(24, 24));
    btnDiscord->setFixedSize(40, 40);
    btnDiscord->setCursor(Qt::PointingHandCursor);

    bottomBarLayout->addWidget(versionLabel);
    bottomBarLayout->addStretch();
    bottomBarLayout->addWidget(btnDiscord);

    mainLayout->addWidget(bottomBar);
}

void MainWindow::updateIcons()
{
    QColor accent = ThemeManager::instance().accentColor();
    QColor iconColor = accent.lightness() > 128 ? QColor("#000000") : QColor("#FFFFFF");

    if (auto* logo = findChild<QPushButton*>("logoBtn")) {
        logo->setIcon(ThemeManager::instance().getIcon("szutility", accent, QString()));
    }
    if (auto* auctionSidebarBtn = findChild<QPushButton*>("mainSidebarBtn")) {
        // Цвет иконки аукциона в сайдбаре всегда серый (не зависит от темы), но перекрасим на всякий случай
        auctionSidebarBtn->setIcon(ThemeManager::instance().getIcon("auction", QColor("#A8998A"), "MainWindow"));
    }
    if (m_profileBtn) {
        m_profileBtn->setIcon(ThemeManager::instance().getIcon(
            m_currentUser.isEmpty() ? "profileBtnOff" : "profileBtnOn", iconColor, "MainWindow"));
    }
    if (auto* exitBtn = findChild<QToolButton*>("btnExit")) {
        exitBtn->setIcon(ThemeManager::instance().getIcon("exitBtn", iconColor, "MainWindow"));
    }
    if (auto* discordBtn = findChild<QToolButton*>("btnDiscord")) {
        discordBtn->setIcon(ThemeManager::instance().getIcon("discord", QColor("#FFFFFF"), ""));
    }
}

void MainWindow::showProfileMenu()
{
    if (!m_profileMenu || !m_profileBtn) return;

    m_profileMenu->show();
    m_profileMenu->raise();
    m_profileMenu->adjustSize();

    int menuWidth = m_profileMenu->width();
    int menuHeight = m_profileMenu->height();

    QPoint globalAnchor = m_profileBtn->mapToGlobal(
        QPoint(m_profileBtn->width() / 2, m_profileBtn->height()));
    QPoint localAnchor = mapFromGlobal(globalAnchor);

    int x = localAnchor.x() - menuWidth / 2;
    int y = localAnchor.y() + 8;

    if (x < 10) x = 10;
    if (x + menuWidth > width() - 10) x = width() - menuWidth - 10;
    if (y + menuHeight > height() - 10) y = height() - menuHeight - 10;

    QPoint endPos(x, y);
    QPoint startPos(x, y - 8);
    m_profileMenu->move(startPos);

    auto* posAnim = new QPropertyAnimation(m_profileMenu, "pos", m_profileMenu);
    posAnim->setDuration(200);
    posAnim->setStartValue(startPos);
    posAnim->setEndValue(endPos);
    posAnim->setEasingCurve(QEasingCurve::OutCubic);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);

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

void MainWindow::blockMainWindow()
{
    if (centralWidget()) centralWidget()->setEnabled(false);
}

void MainWindow::unblockMainWindow()
{
    if (centralWidget()) centralWidget()->setEnabled(true);
}

void MainWindow::positionDialog(QWidget* dialog)
{
    dialog->adjustSize();
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
    m_currentUserInfo = DBManager::instance().getUserInfo(username);
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
    m_loginWindow->setFocus();
}

void MainWindow::onLogout()
{
    m_currentUser.clear();
    m_currentUserInfo = UserInfo();
    QSettings settings;
    settings.setValue(AuthSettings::kRememberMe, false);
    settings.remove(AuthSettings::kUsername);
    updateProfileButton();
}

void MainWindow::updateProfileButton()
{
    if (m_currentUser.isEmpty()) {
        m_profileBtn->setText("ВОЙТИ | ЗАРЕГИСТРИРОВАТЬСЯ");
        m_profileBtn->setIcon(ThemeManager::instance().getIcon("profileBtnOff", QColor("#000000"), "MainWindow"));
    }
    else {
        m_profileBtn->setText("ЛИЧНЫЙ КАБИНЕТ");
        m_profileBtn->setIcon(ThemeManager::instance().getIcon("profileBtnOn", QColor("#000000"), "MainWindow"));
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

void MainWindow::onOpenUserSettings()
{
    hideProfileMenu();

    if (!m_settingsWindow) {
        m_settingsWindow = std::make_unique<SettingsWindow>();
        connect(m_settingsWindow.get(), &SettingsWindow::backRequested, this, &MainWindow::onBackToMain);
    }

    m_settingsWindow->setUserInfo(m_currentUserInfo);
    m_settingsWindow->showFullScreen();
    m_settingsWindow->raise();
    m_settingsWindow->activateWindow();
    hide();
}

void MainWindow::onBackToMain()
{
    if (m_auctionWindow) m_auctionWindow->hide();
    if (m_barterWindow)  m_barterWindow->hide();
    if (m_itemsWindow)   m_itemsWindow->hide();
    if (m_settingsWindow) m_settingsWindow->hide();

    showFullScreen();
    raise();
    activateWindow();
}
