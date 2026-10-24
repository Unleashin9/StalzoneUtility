#pragma once
#include <QObject>
#include <QString>
#include <QIcon>
#include <QColor>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum class ThemeType { None, Dark, Light };

    static ThemeManager& instance();

    void setTheme(ThemeType theme);
    ThemeType currentTheme() const { return m_currentTheme; }

    QIcon getIcon(const QString& iconName, const QColor& color, const QString& folder = QStringLiteral("MainWindow")) const;

    QColor accentColor() const;

    // Генерирует PNG во временную папку и возвращает путь к файлу
    QString iconAsTempFile(const QString& iconName, const QColor& color, const QString& folder = QStringLiteral("LoginWindow")) const;

signals:
    void themeChanged();

private:
    ThemeManager() = default;
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void applyStyleSheet();

    ThemeType m_currentTheme = ThemeType::None;
};
