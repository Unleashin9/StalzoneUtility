#pragma once

#include <QObject>
#include <QHash>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class QNetworkAccessManager;

class GameDatabase : public QObject
{
    Q_OBJECT

public:
    explicit GameDatabase(QObject* parent = nullptr);

    void load();

    // Получить русское название предмета по ID
    QString itemName(const QString& id) const;

    // Получить все рецепты бартера
    QJsonArray barterRecipes() const;

    // НОВОЕ: Получить список всех предметов (ID -> Название)
    QMap<QString, QString> allItems() const;

    bool isLoaded() const { return m_loaded; }

signals:
    void loaded();
    void errorOccurred(const QString& msg);

private:
    void parseListing(const QJsonArray& array);
    void parseBarterRecipes(const QJsonArray& array);

    QNetworkAccessManager* m_networkManager;
    QHash<QString, QString> m_itemNames;
    QJsonArray m_barterRecipes;
    bool m_loaded = false;
    int m_pendingRequests = 0;
};
