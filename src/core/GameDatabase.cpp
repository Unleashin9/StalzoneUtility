#include "GameDatabase.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

GameDatabase::GameDatabase(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

void GameDatabase::load()
{
    if (m_loaded) {
        emit loaded();
        return;
    }

    const QUrl listingUrl("https://raw.githubusercontent.com/EXBO-Studio/stalzone-database/main/ru/listing.json");
    const QUrl barterUrl("https://raw.githubusercontent.com/EXBO-Studio/stalzone-database/main/ru/barter_recipes.json");

    m_pendingRequests = 2;

    QNetworkReply* replyListing = m_networkManager->get(QNetworkRequest(listingUrl));
    connect(replyListing, &QNetworkReply::finished, this, [this, replyListing]() {
        if (replyListing->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(replyListing->readAll());
            if (doc.isArray()) {
                parseListing(doc.array());
            }
        }
        else {
            emit errorOccurred("Failed to load listing.json: " + replyListing->errorString());
        }
        replyListing->deleteLater();
        if (--m_pendingRequests == 0) { m_loaded = true; emit loaded(); }
        });

    QNetworkReply* replyBarter = m_networkManager->get(QNetworkRequest(barterUrl));
    connect(replyBarter, &QNetworkReply::finished, this, [this, replyBarter]() {
        if (replyBarter->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(replyBarter->readAll());
            if (doc.isArray()) {
                parseBarterRecipes(doc.array());
            }
        }
        else {
            emit errorOccurred("Failed to load barter_recipes.json: " + replyBarter->errorString());
        }
        replyBarter->deleteLater();
        if (--m_pendingRequests == 0) { m_loaded = true; emit loaded(); }
        });
}

void GameDatabase::reload()
{
    m_loaded = false;
    m_itemNames.clear();
    m_itemCategories.clear();
    m_itemIconPaths.clear();
    m_itemColors.clear();
    m_barterRecipes = QJsonArray();
    load();
}

void GameDatabase::parseListing(const QJsonArray& array)
{
    m_itemNames.clear();
    m_itemCategories.clear();
    m_itemIconPaths.clear();
    m_itemColors.clear();

    for (const QJsonValue& val : array) {
        QJsonObject obj = val.toObject();
        QString dataPath = obj["data"].toString(); // например, "/items/artefact/..."
        QString iconPath = obj["icon"].toString();   // например, "/icons/artefact/..."
        QString nameRu = obj["name"].toObject()["lines"].toObject()["ru"].toString();
        QString color = obj["color"].toString();     // например, "RANK_MASTER"

        QStringList parts = dataPath.split('/');
        if (parts.size() >= 3) {
            QString fileName = parts.last();
            QString id = fileName.split('.').first();
            if (!id.isEmpty() && !nameRu.isEmpty()) {
                m_itemNames[id] = nameRu;
                m_itemCategories[id] = parts[2]; // "artefact", "weapon" и т.д.
                m_itemIconPaths[id] = iconPath;
                m_itemColors[id] = color;
            }
        }
    }
    qDebug() << "GameDatabase: loaded" << m_itemNames.size() << "item names.";
}

void GameDatabase::parseBarterRecipes(const QJsonArray& array)
{
    m_barterRecipes = array;
    qDebug() << "GameDatabase: loaded" << m_barterRecipes.size() << "barter settlements.";
}

QString GameDatabase::itemName(const QString& id) const
{
    return m_itemNames.value(id, id);
}

QJsonArray GameDatabase::barterRecipes() const
{
    return m_barterRecipes;
}

QMap<QString, QString> GameDatabase::allItems() const
{
    QMap<QString, QString> sortedMap;
    for (auto it = m_itemNames.begin(); it != m_itemNames.end(); ++it) {
        sortedMap.insert(it.key(), it.value());
    }
    return sortedMap;
}

QVector<ItemData> GameDatabase::itemsByGroup(const QString& groupName) const
{
    QVector<ItemData> result;
    for (auto it = m_itemNames.begin(); it != m_itemNames.end(); ++it) {
        const QString& id = it.key();
        const QString& name = it.value();
        QString cat = m_itemCategories.value(id);

        bool matches = false;
        if (groupName == "all") matches = true;
        else if (groupName == "weapon") matches = (cat == "weapon" || cat == "device");
        else if (groupName == "armor") matches = (cat == "armor");
        else if (groupName == "containers") matches = (cat == "containers" || cat == "backpacks");
        else if (groupName == "artefact") matches = (cat == "artefact");
        else if (groupName == "attachment") matches = (cat == "attachment" || cat == "weapon_modules");
        else if (groupName == "bullet") matches = (cat == "bullet" || cat == "grenade");
        else if (groupName == "medicine") matches = (cat == "medicine" || cat == "supply");
        else if (groupName == "misc") matches = (cat == "misc");
        else if (groupName == "other") matches = (cat == "other");

        if (matches) {
            result.append({
                id,
                name,
                cat,
                m_itemIconPaths.value(id),
                m_itemColors.value(id)
                });
        }
    }
    return result;
}
