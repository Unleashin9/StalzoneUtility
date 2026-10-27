#include "IconCache.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

IconCache& IconCache::instance()
{
    static IconCache s_instance;
    return s_instance;
}

IconCache::IconCache(QObject* parent)
    : QObject(parent)
    , m_netManager(new QNetworkAccessManager(this))
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString roaming = QFileInfo(appData).dir().absolutePath();
    m_cacheDir = roaming + "/SZUtility/icons";
    QDir().mkpath(m_cacheDir);
    qDebug() << "[IconCache] Cache dir:" << m_cacheDir;
}

QString IconCache::cachePathForItem(const QString& itemId) const
{
    return m_cacheDir + "/" + itemId + ".png";
}

void IconCache::getIcon(const QString& itemId,
    const QString& iconPath,
    std::function<void(const QPixmap&)> callback,
    QObject* context)
{
    if (itemId.isEmpty() || iconPath.isEmpty()) {
        if (callback) callback(QPixmap());
        return;
    }

    // 1. Кэш в памяти
    if (m_memoryCache.contains(itemId)) {
        if (callback) callback(m_memoryCache.value(itemId));
        return;
    }

    // 2. Кэш на диске
    QString path = cachePathForItem(itemId);
    if (QFile::exists(path)) {
        QPixmap pixmap;
        if (pixmap.load(path)) {
            m_memoryCache.insert(itemId, pixmap);
            if (callback) callback(pixmap);
            return;
        }
    }

    // 3. Уже качается — добавляем в очередь ожидания
    PendingRequest req;
    req.callback = callback;
    req.context = context;
    req.hasContext = (context != nullptr);

    if (m_pending.contains(itemId)) {
        m_pending[itemId].append(req);
        return;
    }
    m_pending[itemId].append(req);

    // 4. Качаем с GitHub
    downloadIcon(itemId, iconPath);
}

void IconCache::downloadIcon(const QString& itemId, const QString& iconPath)
{
    QString url = "https://github.com/EXBO-Studio/stalzone-database/raw/main/ru" + iconPath;
    QNetworkRequest request{ QUrl(url) };
    auto* reply = m_netManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, itemId]() {
        QPixmap pixmap;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            if (!data.isEmpty()) {
                pixmap.loadFromData(data);
            }
        }
        reply->deleteLater();

        if (!pixmap.isNull()) {
            m_memoryCache.insert(itemId, pixmap);
            pixmap.save(cachePathForItem(itemId), "PNG");
        }

        // Уведомляем всех ждущих (пропускаем тех, чей контекст удалён)
        QList<PendingRequest> waiters = m_pending.take(itemId);
        for (const auto& w : waiters) {
            if (w.hasContext && w.context.isNull()) continue; // объект-владелец удалён
            if (w.callback) w.callback(pixmap);
        }
        });
}
