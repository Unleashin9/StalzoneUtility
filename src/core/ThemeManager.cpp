#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QStandardPaths>
#include <QDir>

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

QColor ThemeManager::accentColor() const
{
    switch (m_currentTheme) {
    case ThemeType::Dark:  return QColor("#A3E635");
    case ThemeType::Light: return QColor("#84CC16");
    default: return QColor("#A3E635");
    }
}

QIcon ThemeManager::getIcon(const QString& iconName, const QColor& color, const QString& folder) const
{
    const QString path = QStringLiteral(":/resources/icons/%1/%2.svg").arg(folder, iconName);

    QSvgRenderer renderer(path);
    if (!renderer.isValid()) {
        qWarning() << "SVG not found or invalid:" << path;
        return QIcon();
    }

    QSize size = renderer.defaultSize();
    if (size.isEmpty()) size = QSize(32, 32);

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return QIcon(pixmap);
}

QString ThemeManager::iconAsTempFile(const QString& iconName, const QColor& color, const QString& folder) const
{
    // Папка кэша (создается один раз)
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/stalzone_icons";
    QDir().mkpath(cacheDir);

    QString fileName = QString("%1_%2_%3.png").arg(folder, iconName, color.name().mid(1));
    QString filePath = cacheDir + "/" + fileName;

    // Если файл уже есть — переиспользуем
    if (QFile::exists(filePath)) {
        // Возвращаем путь с forward slashes (Qt QSS этого ожидает)
        return QString(filePath).replace('\\', '/');
    }

    const QString resourcePath = QStringLiteral(":/resources/icons/%1/%2.svg").arg(folder, iconName);
    QSvgRenderer renderer(resourcePath);
    if (!renderer.isValid()) {
        qWarning() << "SVG not found or invalid:" << resourcePath;
        return QString();
    }

    QSize size = renderer.defaultSize();
    if (size.isEmpty()) size = QSize(32, 32);

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    if (!pixmap.save(filePath, "PNG")) {
        qWarning() << "Failed to save temp icon:" << filePath;
        return QString();
    }

    return QString(filePath).replace('\\', '/');
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

    // === ПОДСТАНОВКА ПЕРЕМЕННЫХ ТЕМЫ ===
    QColor accent = accentColor();
    style.replace("@accent@", accent.name());

    QString rememberMeIcon = iconAsTempFile("rememberMe", accent, "LoginWindow");
    if (!rememberMeIcon.isEmpty()) {
        style.replace("@rememberMeIcon@", rememberMeIcon);
    }
    else {
        // Fallback: если иконка не сгенерировалась, ставим акцентный цвет на фон
        // (визуально индикатор будет залит цветом, что тоже читается)
        style.replace("@rememberMeIcon@", QStringLiteral(":/resources/icons/LoginWindow/rememberMe.svg"));
    }

    qApp->setStyleSheet(style);
}
