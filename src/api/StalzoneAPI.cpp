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

    if (m_currentReply) {
        m_currentReply->disconnect(this);
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_accumulatedLots = QJsonArray();
    m_currentRegion = region;
    m_currentItemId = itemId;
    m_currentOffset = 0;

    QString url = QString("https://eapi.stalcraft.net/%1/auction/%2/lots?additional=true&limit=%3&offset=0")
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
        if (reply != m_currentReply) {
            reply->deleteLater();
            return;
        }

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonArray lots;
            int total = 0;

            if (doc.isArray()) {
                lots = doc.array();
                total = lots.size();
            }
            else if (doc.isObject()) {
                lots = doc.object().value("lots").toArray();
                total = doc.object().value("total").toInt();
            }

            for (const QJsonValue& v : lots) {
                m_accumulatedLots.append(v);
            }
            int received = lots.size();

            // Отправляем прогресс загрузки
            emit loadingProgress(m_accumulatedLots.size(), total);

            m_currentOffset += received;

            if (received < m_pageLimit || m_accumulatedLots.size() >= m_maxTotalLots || (total > 0 && m_accumulatedLots.size() >= total)) {
                emit allLotsReceived(m_accumulatedLots);
                reply->deleteLater();
                m_currentReply = nullptr;
            }
            else {
                QString baseUrl = QString("https://eapi.stalcraft.net/%1/auction/%2/lots?additional=true&limit=%3&offset=%4")
                    .arg(m_currentRegion, m_currentItemId).arg(m_pageLimit).arg(m_currentOffset);
                reply->deleteLater();
                m_currentReply = nullptr;
                requestNextPage(baseUrl);
            }
        }
        else {
            emit errorOccurred(QString("API Error: %1").arg(reply->errorString()));
            reply->deleteLater();
            m_currentReply = nullptr;
        }
        });
}

void StalzoneAPI::performRequest(const QString& url)
{
    Q_UNUSED(url);
}
