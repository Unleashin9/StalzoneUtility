#include "MainWindow.h"

// Подключаем конкретные виджеты
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

// Подключаем наш API и новую страницу аукциона
#include "api/StalzoneAPI.h"
#include "ui/pages/AuctionWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("STALZONE Utility — v0.1.0");
    resize(1200, 750);

    // 1. Сначала создаем API-менеджер.
    // Передаем 'this' в качестве родителя, чтобы Qt автоматически удалила его
    // при закрытии главного окна (защита от утечек памяти).
    m_api = new StalzoneAPI(this);

    // 2. Собираем интерфейс.
    setupUi();

    // 3. Связываем меню слева со стопкой страниц справа.
    connect(m_sidebar, &QListWidget::currentRowChanged,
        m_pages, &QStackedWidget::setCurrentIndex);
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    createSidebar();
    createPages();

    mainLayout->addWidget(m_sidebar, 1);
    mainLayout->addWidget(m_pages, 4);
}

void MainWindow::createSidebar()
{
    m_sidebar = new QListWidget(this);
    m_sidebar->setFixedWidth(220);

    // Стилизация бокового меню (тёмная тема)
    m_sidebar->setStyleSheet(
        "QListWidget {"
        "  background-color: #1e1e1e;"
        "  color: #cccccc;"
        "  border: none;"
        "  font-size: 14px;"
        "  padding: 8px 0;"
        "}"
        "QListWidget::item {"
        "  padding: 10px 16px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #2d5a27;"
        "  color: #ffffff;"
        "}"
    );

    const QStringList sections = {
        "🏠  Главная",
        "🧮  Калькуляторы",
        "📦  База предметов",
        "🔨  Убежище",
        "💰  Аукцион",
        "👤  Игроки"
    };
    m_sidebar->addItems(sections);
    m_sidebar->setCurrentRow(0);
}

void MainWindow::createPages()
{
    m_pages = new QStackedWidget(this);

    // --- 0. Главная (заглушка) ---
    auto* homePage = new QWidget(this);
    homePage->setStyleSheet("background-color: #1e1e1e; color: #cccccc;");
    auto* homeLayout = new QVBoxLayout(homePage);
    auto* homeLabel = new QLabel("<h1>Добро пожаловать в STALZONE Utility</h1>", homePage);
    homeLabel->setAlignment(Qt::AlignCenter);
    homeLayout->addWidget(homeLabel);
    m_pages->addWidget(homePage);

    // --- 1. Калькуляторы (заглушка) ---
    auto* calcPage = new QWidget(this);
    calcPage->setStyleSheet("background-color: #1e1e1e; color: #cccccc;");
    auto* calcLayout = new QVBoxLayout(calcPage);
    auto* calcLabel = new QLabel("<h1>Калькуляторы (в разработке)</h1>", calcPage);
    calcLabel->setAlignment(Qt::AlignCenter);
    calcLayout->addWidget(calcLabel);
    m_pages->addWidget(calcPage);

    // --- 2. База предметов (заглушка) ---
    auto* dbPage = new QWidget(this);
    dbPage->setStyleSheet("background-color: #1e1e1e; color: #cccccc;");
    auto* dbLayout = new QVBoxLayout(dbPage);
    auto* dbLabel = new QLabel("<h1>База предметов (в разработке)</h1>", dbPage);
    dbLabel->setAlignment(Qt::AlignCenter);
    dbLayout->addWidget(dbLabel);
    m_pages->addWidget(dbPage);

    // --- 3. Убежище (заглушка) ---
    auto* hideoutPage = new QWidget(this);
    hideoutPage->setStyleSheet("background-color: #1e1e1e; color: #cccccc;");
    auto* hideoutLayout = new QVBoxLayout(hideoutPage);
    auto* hideoutLabel = new QLabel("<h1>Убежище (в разработке)</h1>", hideoutPage);
    hideoutLabel->setAlignment(Qt::AlignCenter);
    hideoutLayout->addWidget(hideoutLabel);
    m_pages->addWidget(hideoutPage);

    // --- 4. АУКЦИОН (реальная страница) ---
    // Создаем наш новый виджет и передаем ему API для запросов.
    auto* auctionWindow = new AuctionWindow(m_api, this);
    m_pages->addWidget(auctionWindow);

    // --- 5. Игроки (заглушка) ---
    auto* playersPage = new QWidget(this);
    playersPage->setStyleSheet("background-color: #1e1e1e; color: #cccccc;");
    auto* playersLayout = new QVBoxLayout(playersPage);
    auto* playersLabel = new QLabel("<h1>Игроки (в разработке)</h1>", playersPage);
    playersLabel->setAlignment(Qt::AlignCenter);
    playersLayout->addWidget(playersLabel);
    m_pages->addWidget(playersPage);
}
