#include "StalzoneAPI.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>

StalzoneAPI::StalzoneAPI(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
    QString tokenPath = QCoreApplication::applicationDirPath() + "/token.txt";

    if (!QFile::exists(tokenPath)) {
        tokenPath = "../../../token.txt";
    }

    if (!loadTokenFromFile(tokenPath)) {
        qDebug() << "WARNING: Token not loaded from token.txt!";
    }
}

bool StalzoneAPI::loadTokenFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    m_accessToken = in.readAll().trimmed();
    file.close();

    return !m_accessToken.isEmpty();
}

void StalzoneAPI::fetchAuctionLots(const QString& region, const QString& itemId)
{
    if (m_accessToken.isEmpty()) {
        emit errorOccurred("Token is empty. Check token.txt");
        return;
    }

    QString url = QString("https://eapi.stalzone.net/%1/auction/%2/lots?limit=10")
        .arg(region, itemId);
    performRequest(url);
}

void StalzoneAPI::performRequest(const QString& url)
{
    QNetworkRequest request{ QUrl(url) };
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());

    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);

            if (doc.isArray()) {
                emit lotsReceived(doc.array());
            }
            else if (doc.isObject() && doc.object().contains("lots")) {
                emit lotsReceived(doc.object().value("lots").toArray());
            }
            else {
                emit errorOccurred("Unknown API response format");
            }
        }
        else {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            emit errorOccurred(QString("API Error (HTTP %1): %2").arg(statusCode).arg(reply->errorString()));
        }
        reply->deleteLater();
        });
}
