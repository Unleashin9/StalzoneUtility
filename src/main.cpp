#include <QApplication>
#include "ui/MainWindow.h"
#include "core/ThemeManager.h"
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Принудительно используем Fusion (лучше дружит с QSS)
    QApplication::setStyle("Fusion");

    QCoreApplication::setOrganizationName("STALZONE");
    QCoreApplication::setApplicationName("Utility");
    QCoreApplication::setApplicationVersion("0.1.0");

    // Устанавливаем темную тему
    ThemeManager::instance().setTheme(ThemeManager::ThemeType::Dark);

    MainWindow window;
    window.showFullScreen(); // <--- ВКЛЮЧАЕМ ПОЛНЫЙ ЭКРАН

    return app.exec();
}
