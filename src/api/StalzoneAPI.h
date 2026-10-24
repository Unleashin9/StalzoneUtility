#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>

class QNetworkReply;

class StalzoneAPI : public QObject
{
    Q_OBJECT

public:
    explicit StalzoneAPI(QObject* parent = nullptr);

    void fetchAuctionLots(const QString& region, const QString& itemId);

signals:
    void lotsReceived(const QJsonArray& lots);
    void errorOccurred(const QString& errorString);

private:
    bool loadTokenFromFile(const QString& filePath);
    void performRequest(const QString& url);

    QNetworkAccessManager* m_networkManager;
    QString m_accessToken;
};
