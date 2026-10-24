#pragma once  // Защита от двойного включения заголовка — заменяет #ifndef guards.

#include <QMainWindow>

// ВАЖНО: forward declaration вместо #include.
// Мы не знаем, понадобится ли нам QListWidget и QStackedWidget в других файлах,
// а включать лишние заголовки = замедление сборки. Объявляем "они есть",
// подключаем по-настоящему — только в .cpp.
class QListWidget;
class QStackedWidget;

// MainWindow наследуется от QMainWindow — окно с меню, статус-баром, тулбаром.
// Можно было бы использовать QWidget, но QMainWindow удобнее для будущего.
class MainWindow : public QMainWindow
{
    // Q_OBJECT — МАКРОС, без которого не работают сигналы/слоты, свойства, moc.
    // Это "маркер" для CMake (AUTOMOC), что этот класс нужно обработать.
    // Без Q_OBJECT класс компилируется, но connect() с ним не сработает!
    Q_OBJECT

public:
    // explicit — запрещает неявное преобразование из QWidget* в MainWindow.
    // parent = nullptr — значит "это окно верхнего уровня, без родителя".
    explicit MainWindow(QWidget* parent = nullptr);

    // Виртуальный деструктор. Мы пока не владеем голыми указателями (всё через Qt-parent),
    // но правило хорошего тона: если класс наследуется — деструктор virtual.
    // = default означает "пусть компилятор сгенерирует пустой".
    ~MainWindow() override = default;

private:
    // Приватные методы: строят интерфейс. Разбиваем на части, чтобы код был читаемым.
    void setupUi();         // Общий каркас: центральный виджет + layout
    void createSidebar();   // Левая панель с кнопками разделов
    void createPages();     // Правая часть — стопка страниц-заглушек

    // Члены класса. Префикс m_ — стандартное соглашение в Qt-проектах
    // (m = member), чтобы сразу отличать поля от локальных переменных.
    QListWidget* m_sidebar = nullptr;  // Список разделов слева
    QStackedWidget* m_pages = nullptr;  // Стопка страниц справа
};