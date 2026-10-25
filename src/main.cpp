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

    // Отключаем системные анимации окон (устраняет мерцание при переключении)
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false);
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);
    QApplication::setEffectEnabled(Qt::UI_AnimateTooltip, false);
    QApplication::setEffectEnabled(Qt::UI_FadeMenu, false);
    QApplication::setEffectEnabled(Qt::UI_FadeTooltip, false);

    QApplication::setStyle("Fusion");

    QCoreApplication::setOrganizationName("STALZONE");
    QCoreApplication::setApplicationName("Utility");
    QCoreApplication::setApplicationVersion("0.1.0");

    // === ЗАГРУЗКА ВСЕХ ШРИФТОВ ИЗ resources/fonts/ ===
    int loadedCount = 0;
    QDirIterator it(":/resources/fonts", { "*.ttf", "*.otf" }, QDir::Files);
    while (it.hasNext()) {
        const QString fontPath = it.next();
        const int fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1) {
            const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
            for (const QString& family : families) {
                qDebug() << "Loaded font:" << family << "from" << fontPath;
            }
            ++loadedCount;
        }
        else {
            qWarning() << "Failed to load font:" << fontPath;
        }
    }
    qDebug() << "Total font files loaded:" << loadedCount;

    // === УСТАНАВЛИВАЕМ IBM PLEX SANS ГЛОБАЛЬНО ===
    QFont appFont("IBM Plex Sans");
    appFont.setPointSize(10);
    app.setFont(appFont);

    // Проверка: реально ли шрифт применён
    qDebug() << "App font family:" << app.font().family()
        << "exact match:" << QFontInfo(app.font()).family();

    // === ТЕМА ===
    ThemeManager::instance().setTheme(ThemeManager::ThemeType::Dark);

    MainWindow window;
    window.showFullScreen();

    return app.exec();
}
