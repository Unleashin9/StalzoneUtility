#pragma once
#include <QObject>
#include <QHash>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class QNetworkAccessManager;

struct ItemData {
    QString id;
    QString name;
    QString category;
    QString iconPath; // <--- НОВОЕ: путь к иконке (например, "/icons/artefact/...")
    QString color;    // <--- НОВОЕ: цвет редкости (например, "RANK_MASTER")
};

class GameDatabase : public QObject
{
    Q_OBJECT

public:
    explicit GameDatabase(QObject* parent = nullptr);

    void load();
    void reload();

    QString itemName(const QString& id) const;
    QJsonArray barterRecipes() const;
    QMap<QString, QString> allItems() const;
    bool isLoaded() const { return m_loaded; }

    QVector<ItemData> itemsByGroup(const QString& groupName) const;

signals:
    void loaded();
    void errorOccurred(const QString& msg);

private:
    void parseListing(const QJsonArray& array);
    void parseBarterRecipes(const QJsonArray& array);

    QNetworkAccessManager* m_networkManager;
    QHash<QString, QString> m_itemNames;
    QHash<QString, QString> m_itemCategories;
    QHash<QString, QString> m_itemIconPaths; // <--- НОВОЕ
    QHash<QString, QString> m_itemColors;    // <--- НОВОЕ
    QJsonArray m_barterRecipes;
    bool m_loaded = false;
    int m_pendingRequests = 0;
};
