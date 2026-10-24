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

    // Метод для перекраски SVG-иконок
    QIcon getIcon(const QString& iconName, const QColor& color, const QString& folder = QStringLiteral("MainWindow")) const;

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
