#include "MainWindow.h"

// Конкретные виджеты подключаем здесь — в .cpp, а не в .h.
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

// Конструктор. Порядок инициализации: сначала база (QMainWindow),
// потом члены класса (у нас = nullptr по умолчанию, ничего писать не надо).
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Заголовок окна и стартовый размер.
    setWindowTitle("STALZONE Utility — v0.1.0");
    resize(1200, 750);  // Разумный дефолт для десктопа

    // Собираем интерфейс.
    setupUi();

    // СВЯЗЫВАЕМ сигнал и слот:
    // QListWidget испускает сигнал currentRowChanged(int) каждый раз,
    // когда пользователь меняет выделенный пункт.
    // Мы соединяем его со слотом QStackedWidget::setCurrentIndex,
    // который просто переключает видимую страницу на ту же позицию.
    // Это классический пример сигнал-слот в Qt — вся мощь фреймворка здесь.
    connect(m_sidebar, &QListWidget::currentRowChanged,
        m_pages, &QStackedWidget::setCurrentIndex);
}

void MainWindow::setupUi()
{
    // CentralWidget — это "тело" QMainWindow (без меню и статус-бара).
    // Qt требует, чтобы у QMainWindow был центральный виджет. Создаём пустой.
    auto* central = new QWidget(this);
    setCentralWidget(central);

    // Главный layout — горизонтальный: слева sidebar, справа pages.
    auto* mainLayout = new QHBoxLayout(central);

    // Отступы (margin) и промежутки (spacing). 0 отступов = панели прижаты к краям.
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Создаём части интерфейса.
    createSidebar();
    createPages();

    // Добавляем в layout. Коэффициенты "1" и "4" — соотношение ширины:
    // sidebar займёт ~1/5, pages — ~4/5.
    mainLayout->addWidget(m_sidebar, 1);
    mainLayout->addWidget(m_pages, 4);
}

void MainWindow::createSidebar()
{
    m_sidebar = new QListWidget(this);

    // Фиксированная ширина, чтобы меню не расползалось.
    m_sidebar->setFixedWidth(220);

    // Стилизуем через CSS-подобный QSS. Простой тёмный стиль — потом доработаем.
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
        "  background-color: #2d5a27;"   // Наш "сталкерский" зелёный
        "  color: #ffffff;"
        "}"
    );

    // Пункты меню — по одному на будущий раздел.
    // Пока это только ярлыки, но через 2 недели часть из них откроет настоящий функционал.
    const QStringList sections = {
        "🏠  Главная",
        "🧮  Калькуляторы",
        "📦  База предметов",
        "🔨  Убежище",
        "💰  Аукцион",
        "👤  Игроки"
    };
    m_sidebar->addItems(sections);

    // Выделяем первую строку по умолчанию.
    m_sidebar->setCurrentRow(0);
}

void MainWindow::createPages()
{
    m_pages = new QStackedWidget(this);

    // QStackedWidget — это "стопка карточек", видна только одна.
    // Для MVP на каждую секцию — простая заглушка с названием.
    // Позже заменим заглушки настоящими виджетами-разделами.
    const QStringList sections = {
        "Главная",
        "Калькуляторы",
        "База предметов",
        "Убежище",
        "Аукцион",
        "Игроки"
    };

    for (const QString& title : sections) {
        // Создаём простой виджет-страницу.
        auto* page = new QWidget(this);
        page->setStyleSheet("background-color: #1e1e1e;"); // Темный фон, как у сайдбара
        auto* layout = new QVBoxLayout(page);

        auto* label = new QLabel(
            QString("<h1>%1</h1><p>Раздел в разработке...</p>").arg(title),
            page
        );
        label->setAlignment(Qt::AlignCenter);

        label->setStyleSheet("color: #cccccc; background-color: transparent;"); // Светлый текст
        layout->addWidget(label);
        m_pages->addWidget(page);
    }
}