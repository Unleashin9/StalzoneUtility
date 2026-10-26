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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    m_accessToken = in.readAll().trimmed();
    file.close();
    return !m_accessToken.isEmpty();
}

void StalzoneAPI::fetchAuctionLots(const QString& region, const QString& itemId)
{
    fetchAllAuctionLots(region, itemId);
}

void StalzoneAPI::fetchAllAuctionLots(const QString& region, const QString& itemId)
{
    if (m_accessToken.isEmpty()) {
        emit errorOccurred("Token is empty. Check token.txt");
        return;
    }

    // ОТМЕНЯЕМ предыдущий запрос, если он ещё в процессе
    if (m_currentReply) {
        m_currentReply->disconnect(this);
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_accumulatedLots = QJsonArray();
    m_currentItemId = itemId;
    m_currentOffset = 0;

    QString url = QString("https://eapi.stalzone.net/%1/auction/%2/lots?limit=%3&offset=0")
        .arg(region, itemId).arg(m_pageLimit);
    requestNextPage(url);
}

void StalzoneAPI::requestNextPage(const QString& url)
{
    QNetworkRequest request{ QUrl(url) };
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());

    QNetworkReply* reply = m_networkManager->get(request);
    m_currentReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Если этот ответ больше не актуален (был отменен или заменен) — игнорируем
        if (reply != m_currentReply) {
            reply->deleteLater();
            return;
        }

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonArray lots;

            if (doc.isArray()) {
                lots = doc.array();
            }
            else if (doc.isObject() && doc.object().contains("lots")) {
                lots = doc.object().value("lots").toArray();
            }

            m_accumulatedLots.append(lots);
            int received = lots.size();
            m_currentOffset += received;

            if (received < m_pageLimit || m_accumulatedLots.size() >= m_maxTotalLots) {
                emit allLotsReceived(m_accumulatedLots);
                reply->deleteLater();
                m_currentReply = nullptr;
            }
            else {
                QString baseUrl = QString("https://eapi.stalzone.net/ru/auction/%1/lots?limit=%2&offset=%3")
                    .arg(m_currentItemId).arg(m_pageLimit).arg(m_currentOffset);
                reply->deleteLater();
                m_currentReply = nullptr;
                requestNextPage(baseUrl);
            }
        }
        else {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            emit errorOccurred(QString("API Error (HTTP %1): %2").arg(statusCode).arg(reply->errorString()));
            reply->deleteLater();
            m_currentReply = nullptr;
        }
        });
}

void StalzoneAPI::performRequest(const QString& url)
{
    Q_UNUSED(url);
}
