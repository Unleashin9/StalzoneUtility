#include <QApplication>
#include "ui/MainWindow.h"
#include "core/ThemeManager.h"
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 1. Принудительно используем Fusion (лучше дружит с QSS)
    QApplication::setStyle("Fusion");

    QCoreApplication::setOrganizationName("STALZONE");
    QCoreApplication::setApplicationName("Utility");
    QCoreApplication::setApplicationVersion("0.1.0");

    // 2. Устанавливаем темную тему через наш ThemeManager
    qDebug() << "Applying theme...";
    ThemeManager::instance().setTheme(ThemeManager::ThemeType::Dark);

    MainWindow window;
    window.show(); // Пока оставляем show(), а не showFullScreen(), чтобы проверить

    return app.exec();
}
