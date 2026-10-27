#pragma once
#include <QObject>
#include <QString>
#include <QPixmap>
#include <QHash>
#include <QList>
#include <QPointer>
#include <functional>

class QNetworkAccessManager;

class IconCache : public QObject
{
    Q_OBJECT
public:
    static IconCache& instance();

    void getIcon(const QString& itemId,
        const QString& iconPath,
        std::function<void(const QPixmap&)> callback,
        QObject* context = nullptr);

    QString cacheDirectory() const { return m_cacheDir; }

private:
    IconCache(QObject* parent = nullptr);
    ~IconCache() = default;
    IconCache(const IconCache&) = delete;
    IconCache& operator=(const IconCache&) = delete;

    struct PendingRequest {
        std::function<void(const QPixmap&)> callback;
        QPointer<QObject> context;
        bool hasContext = false;
    };

    QString cachePathForItem(const QString& itemId) const;
    void downloadIcon(const QString& itemId, const QString& iconPath);

    QNetworkAccessManager* m_netManager;
    QHash<QString, QPixmap> m_memoryCache;
    QHash<QString, QList<PendingRequest>> m_pending;
    QString m_cacheDir;
};
