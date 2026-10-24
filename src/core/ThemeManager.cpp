#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager s_instance;
    return s_instance;
}

void ThemeManager::setTheme(ThemeType theme)
{
    if (m_currentTheme == theme) return;
    m_currentTheme = theme;
    applyStyleSheet();
    emit themeChanged();
}

QIcon ThemeManager::getIcon(const QString& iconName, const QColor& color, const QString& folder) const
{
    // Формируем путь к ресурсу 
    const QString path = QStringLiteral(":/resources/icons/%1/%2.svg").arg(folder, iconName);

    QSvgRenderer renderer(path);
    if (!renderer.isValid()) {
        qWarning() << "SVG not found or invalid:" << path;
        return QIcon();
    }

    QSize size = renderer.defaultSize();
    if (size.isEmpty()) {
        size = QSize(32, 32);
    }

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);

    // МАГИЯ: Заливаем непрозрачные пиксели нужным цветом
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return QIcon(pixmap);
}

void ThemeManager::applyStyleSheet()
{
    if (m_currentTheme == ThemeType::None) return;

    QString fileName = (m_currentTheme == ThemeType::Dark)
        ? ":/resources/dark_theme.qss"
        : ":/resources/light_theme.qss";

    QFile file(fileName);
    if (!file.exists()) {
        file.setFileName(fileName.mid(2));
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to load theme:" << fileName;
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString style = stream.readAll();
    file.close();

    if (style.startsWith(QChar(0xFEFF)))
        style.remove(0, 1);

    qApp->setStyleSheet(style);
}
