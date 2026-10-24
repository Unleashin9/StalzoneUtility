#pragma once

#include <QObject>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class QNetworkAccessManager;

class GameDatabase : public QObject
{
    Q_OBJECT

public:
    explicit GameDatabase(QObject* parent = nullptr);

    // Запускает загрузку данных с GitHub
    void load();

    // Получить русское название предмета по ID (например, "zyv9" -> "Вспышка")
    QString itemName(const QString& id) const;

    // Получить все рецепты бартера
    QJsonArray barterRecipes() const;

    bool isLoaded() const { return m_loaded; }

signals:
    void loaded();                          // Данные успешно загружены
    void errorOccurred(const QString& msg); // Ошибка при загрузке

private:
    void parseListing(const QJsonArray& array);
    void parseBarterRecipes(const QJsonArray& array);

    QNetworkAccessManager* m_networkManager;
    QHash<QString, QString> m_itemNames; // Словарь: ID -> Название
    QJsonArray m_barterRecipes;
    bool m_loaded = false;
    int m_pendingRequests = 0; // Счетчик, чтобы понять, когда загрузились ОБА файла
};
