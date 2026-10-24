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

    // Ссылки на официальный репозиторий игры (ветка main, папка ru)
    const QUrl listingUrl("https://raw.githubusercontent.com/EXBO-Studio/stalzone-database/main/ru/listing.json");
    const QUrl barterUrl("https://raw.githubusercontent.com/EXBO-Studio/stalzone-database/main/ru/barter_recipes.json");

    m_pendingRequests = 2;

    // 1. Загружаем список всех предметов
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

    // 2. Загружаем рецепты бартера
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

void GameDatabase::parseListing(const QJsonArray& array)
{
    m_itemNames.clear();
    for (const QJsonValue& val : array) {
        QJsonObject obj = val.toObject();
        QString dataPath = obj["data"].toString(); // например, "/items/other/qyvk.json"

        // Извлекаем ID из пути (берем имя файла без расширения)
        QStringList parts = dataPath.split('/');
        if (parts.size() >= 2) {
            QString fileName = parts.last(); // "qyvk.json"
            QString id = fileName.split('.').first(); // "qyvk"

            // Достаем русское название
            QString nameRu = obj["name"].toObject()["lines"].toObject()["ru"].toString();
            if (!id.isEmpty() && !nameRu.isEmpty()) {
                m_itemNames[id] = nameRu;
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
    return m_itemNames.value(id, id); // Если не нашли, возвращаем сам ID
}

QJsonArray GameDatabase::barterRecipes() const
{
    return m_barterRecipes;
}
