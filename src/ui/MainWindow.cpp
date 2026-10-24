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
#include <QSettings>
#include <QMenu>

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

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    central->setObjectName("mainContainer");
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

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

    auto* centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(30);
    centerLayout->setAlignment(Qt::AlignCenter);

    auto* btnAuction = new QPushButton("Аукцион", this);
    btnAuction->setObjectName("btnAuctionCard");
    btnAuction->setFixedSize(220, 160);
    btnAuction->setCursor(Qt::PointingHandCursor);
    connect(btnAuction, &QPushButton::clicked, this, &MainWindow::onAuctionClicked);

    auto* btnBarter = new QPushButton("Калькулятор", this);
    btnBarter->setObjectName("btnBarterCard");
    btnBarter->setFixedSize(220, 160);
    btnBarter->setCursor(Qt::PointingHandCursor);
    connect(btnBarter, &QPushButton::clicked, this, &MainWindow::onBarterClicked);

    auto* btnItems = new QPushButton("База предметов", this);
    btnItems->setObjectName("btnItemsCard");
    btnItems->setFixedSize(220, 160);
    btnItems->setCursor(Qt::PointingHandCursor);
    connect(btnItems, &QPushButton::clicked, this, &MainWindow::onItemsClicked);

    centerLayout->addWidget(btnAuction);
    centerLayout->addWidget(btnBarter);
    centerLayout->addWidget(btnItems);

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

void MainWindow::onProfileButtonClicked()
{
    if (m_currentUser.isEmpty()) {
        m_registerWindow->hide();
        applyBlurEffect();
        m_loginWindow->show();
        m_loginWindow->raise();
        m_loginWindow->activateWindow();
    }
    else {
        QMenu menu(this);
        QAction* logoutAction = menu.addAction("Выйти");
        if (menu.exec(m_profileBtn->mapToGlobal(QPoint(0, m_profileBtn->height()))) == logoutAction) {
            onLogout();
        }
    }
}

void MainWindow::onLoginSuccess(const QString& username)
{
    m_currentUser = username;
    updateProfileButton();
    m_loginWindow->hide();
    removeBlurEffect();
}

void MainWindow::onShowRegisterWindow()
{
    m_loginWindow->hide();
    m_registerWindow->clearFields();
    m_registerWindow->show();
    m_registerWindow->raise();
    m_registerWindow->activateWindow();
}

void MainWindow::onShowLoginWindow()
{
    m_registerWindow->hide();
    m_loginWindow->show();
    m_loginWindow->raise();
    m_loginWindow->activateWindow();
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
        m_profileBtn->setText(" " + m_currentUser);
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

void MainWindow::onBackToMain()
{
    if (m_auctionWindow) m_auctionWindow->hide();
    if (m_barterWindow)  m_barterWindow->hide();
    if (m_itemsWindow)   m_itemsWindow->hide();

    showFullScreen();
    raise();
    activateWindow();
}
