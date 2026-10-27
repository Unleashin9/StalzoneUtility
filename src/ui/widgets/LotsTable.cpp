#include "LotsTable.h"
#include "core/IconCache.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QTimer>
#include <QJsonObject>

LotsTable::LotsTable(QWidget* parent) : QWidget(parent)
{
    setObjectName("lotsTableRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
}

void LotsTable::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_table = new QTableWidget(this);
    m_table->setObjectName("lotsTable");
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "ЛОТ", "ЗАТОЧКА", "КОЛИЧЕСТВО", "СТАВКА", "ВЫКУП", "ЗА ЕДИНИЦУ", "ОСТАЛОСЬ"
        });
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int i = 1; i < 7; ++i) {
        m_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);

    layout->addWidget(m_table);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &LotsTable::updateTimers);
}

void LotsTable::setLots(const QJsonArray& lots, const QString& itemId,
    const QString& itemName, const QString& itemIconPath)
{
    m_table->setRowCount(0);
    m_endTimes.clear();
    m_table->setRowCount(lots.size());

    QDateTime now = QDateTime::currentDateTimeUtc();

    for (int i = 0; i < lots.size(); ++i) {
        QJsonObject lot = lots[i].toObject();
        int amount = lot["amount"].toInt();
        int startPrice = lot["startPrice"].toInt();
        int buyoutPrice = lot["buyoutPrice"].toInt();
        QString endTimeStr = lot["endTime"].toString();

        QDateTime endTime = QDateTime::fromString(endTimeStr, Qt::ISODate);
        if (!endTime.isValid()) {
            endTime = QDateTime::fromString(endTimeStr, Qt::ISODateWithMs);
        }
        m_endTimes.append(endTime);

        m_table->setItem(i, 0, new QTableWidgetItem(itemName));

        int ptn = lot["additional"].toObject()["ptn"].toInt(-1);
        QString ptnStr = (ptn < 0) ? "—" : QString::number(ptn);
        m_table->setItem(i, 1, new QTableWidgetItem(ptnStr));

        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(amount)));
        m_table->setItem(i, 3, new QTableWidgetItem(formatPrice(startPrice) + " ₽"));
        m_table->setItem(i, 4, new QTableWidgetItem(formatPrice(buyoutPrice) + " ₽"));

        int perItem = (amount > 0) ? buyoutPrice / amount : buyoutPrice;
        m_table->setItem(i, 5, new QTableWidgetItem(formatPrice(perItem) + " ₽"));

        qint64 secsLeft = now.secsTo(endTime);
        m_table->setItem(i, 6, new QTableWidgetItem(formatTimeLeft(secsLeft)));
    }

    if (!itemIconPath.isEmpty() && m_table->rowCount() > 0) {
        IconCache::instance().getIcon(itemId, itemIconPath,
            [this](const QPixmap& pixmap) {
                if (pixmap.isNull()) return;
                QIcon icon(pixmap);
                for (int i = 0; i < m_table->rowCount(); ++i) {
                    QTableWidgetItem* item = m_table->item(i, 0);
                    if (item) item->setIcon(icon);
                }
            }, this);
    }

    if (!m_timer->isActive()) m_timer->start();
}

void LotsTable::clear()
{
    m_table->setRowCount(0);
    m_endTimes.clear();
    m_timer->stop();
}

void LotsTable::updateTimers()
{
    if (m_endTimes.isEmpty()) return;
    QDateTime now = QDateTime::currentDateTimeUtc();
    for (int i = 0; i < m_table->rowCount() && i < m_endTimes.size(); ++i) {
        QTableWidgetItem* item = m_table->item(i, 6);
        if (!item) continue;
        qint64 secsLeft = now.secsTo(m_endTimes[i]);
        item->setText(formatTimeLeft(secsLeft));
    }
}

QString LotsTable::formatPrice(int value) const
{
    QString s = QString::number(value);
    if (value < 0) s = s.mid(1);

    QString result;
    int count = 0;
    for (int i = s.length() - 1; i >= 0; --i) {
        result.prepend(s[i]);
        ++count;
        if (count % 3 == 0 && i > 0) result.prepend(' ');
    }
    return (value < 0 ? "-" : "") + result;
}

QString LotsTable::formatTimeLeft(qint64 secsLeft) const
{
    if (secsLeft <= 0) return "00:00:00";
    int h = static_cast<int>(secsLeft / 3600);
    int m = static_cast<int>((secsLeft % 3600) / 60);
    int s = static_cast<int>(secsLeft % 60);
    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}
