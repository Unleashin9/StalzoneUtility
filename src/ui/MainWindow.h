#pragma once

#include <QMainWindow>

// Предварительные объявления классов.
// Мы говорим компилятору: "Эти классы существуют, но их полные определения 
// мы подключим только в .cpp файле". Это ускоряет сборку.
class QListWidget;
class QStackedWidget;
class StalzoneAPI;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    // Приватные методы для построения интерфейса
    void setupUi();
    void createSidebar();
    void createPages();

    // Члены класса (виджеты и сервисы)
    QListWidget* m_sidebar = nullptr;
    QStackedWidget* m_pages = nullptr;

    // Наш API-менеджер. Он будет жить всё время работы окна.
    StalzoneAPI* m_api = nullptr;
};
