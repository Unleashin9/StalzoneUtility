#pragma once
#include <QMainWindow>

class QListWidget;
class QStackedWidget;
class StalzoneAPI;
class GameDatabase; // <--- ДОБАВИЛИ

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    void setupUi();
    void createSidebar();
    void createPages();

    QListWidget* m_sidebar = nullptr;
    QStackedWidget* m_pages = nullptr;
    StalzoneAPI* m_api = nullptr;
    GameDatabase* m_db = nullptr; // <--- ДОБАВИЛИ
};
