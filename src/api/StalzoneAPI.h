#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QJsonArray>

class QNetworkReply;

class StalzoneAPI : public QObject
{
    Q_OBJECT
public:
    explicit StalzoneAPI(QObject* parent = nullptr);

    void fetchAuctionLots(const QString& region, const QString& itemId);
    void fetchAllAuctionLots(const QString& region, const QString& itemId);

signals:
    void lotsReceived(const QJsonArray& lots);
    void allLotsReceived(const QJsonArray& allLots);
    void errorOccurred(const QString& errorString);

private:
    bool loadTokenFromFile(const QString& filePath);
    void requestNextPage(const QString& url);
    void performRequest(const QString& url);

    QNetworkAccessManager* m_networkManager;
    QString m_accessToken;

    QJsonArray m_accumulatedLots;
    QString m_currentItemId;
    int m_currentOffset = 0;
    const int m_pageLimit = 200;
    const int m_maxTotalLots = 4000;

    QNetworkReply* m_currentReply = nullptr; // <--- НОВОЕ
};
