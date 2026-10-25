#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QDirIterator>
#include <QDebug>

#include "ui/MainWindow.h"
#include "core/ThemeManager.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setStyle("Fusion");

    QCoreApplication::setOrganizationName("STALZONE");
    QCoreApplication::setApplicationName("Utility");
    QCoreApplication::setApplicationVersion("0.1.0");

    // === ЗАГРУЗКА ВСЕХ ШРИФТОВ ИЗ resources/fonts/ ===
    QDirIterator it(":/resources/fonts", { "*.ttf", "*.otf" }, QDir::Files);
    while (it.hasNext()) {
        const QString fontPath = it.next();
        const int fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1) {
            const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
            for (const QString& family : families) {
                qDebug() << "Loaded font:" << family << "from" << fontPath;
            }
        }
        else {
            qWarning() << "Failed to load font:" << fontPath;
        }
    }

    // Проверка: какие веса Qt видит для IBM Plex Sans
    QFontDatabase db;
    qDebug() << "Available IBM Plex Sans styles:" << db.styles("IBM Plex Sans");

    // Базовый шрифт — только семейство и размер. Веса берём из QSS.
    QFont appFont("IBM Plex Sans");
    appFont.setPointSize(10);
    app.setFont(appFont);

    ThemeManager::instance().setTheme(ThemeManager::ThemeType::Dark);

    MainWindow window;
    window.showFullScreen();

    return app.exec();
}
