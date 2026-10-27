#include "LotsModel.h"

#include <QJsonValue>
#include <QFont>
#include <QBrush>
#include <algorithm>

LotsModel::LotsModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_nowUtc(QDateTime::currentDateTimeUtc())
{
}

int LotsModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

int LotsModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant LotsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};

    const Row& r = m_rows.at(index.row());

    // Обработка заглушки (если лотов нет)
    if (r.isPlaceholder) {
        if (role == Qt::DisplayRole && index.column() == LotName) {
            return r.lotName;
        }
        if (role == Qt::TextAlignmentRole) {
            return int(Qt::AlignCenter); // По центру
        }
        if (role == Qt::ForegroundRole) {
            return QBrush(QColor("#F0A93B")); // Янтарный
        }
        if (role == Qt::FontRole) {
            QFont f;
            f.setBold(true);
            f.setPointSize(11);
            return f;
        }
        return {};
    }

    if (role == Qt::FontRole) {
        QFont f;
        f.setBold(true);
        return f;
    }

    if (role == Qt::ForegroundRole) {
        if (index.column() == LotName) {
            const QColor c = rarityColor(r.rarity);
            if (c.isValid())
                return QBrush(c);
        }
        return {};
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case LotName:      return r.lotName;
        case Enchant:      return r.enchantStr;
        case Quantity:     return r.quantityStr;
        case StartPrice:   return r.startPriceStr;
        case BuyoutPrice:  return r.buyoutPriceStr;
        case PerItemPrice: return r.perItemPriceStr;
        case TimeLeft: {
            const qint64 secs = m_nowUtc.secsTo(r.endTime);
            return formatTimeLeft(secs);
        }
        default: break;
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == LotName) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        return int(Qt::AlignRight | Qt::AlignVCenter);
    }

    return {};
}

QVariant LotsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};

    switch (section) {
    case LotName:      return QStringLiteral("ПРЕДМЕТ");
    case Enchant:      return QStringLiteral("ЗАТОЧКА");
    case Quantity:     return QStringLiteral("КОЛИЧЕСТВО");
    case StartPrice:   return QStringLiteral("СТАВКА");
    case BuyoutPrice:  return QStringLiteral("ВЫКУП");
    case PerItemPrice: return QStringLiteral("ЗА ЕДИНИЦУ");
    case TimeLeft:     return QStringLiteral("ОСТАВШЕЕСЯ ВРЕМЯ");
    default:           return {};
    }
}

void LotsModel::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= ColumnCount || (m_rows.size() == 1 && m_rows.first().isPlaceholder))
        return;

    beginResetModel();
    std::sort(m_rows.begin(), m_rows.end(), [=](const Row& a, const Row& b) {
        int cmp = 0;
        switch (column) {
        case LotName:      cmp = QString::compare(a.lotName, b.lotName, Qt::CaseInsensitive); break;
        case Enchant:      cmp = (a.enchant > b.enchant) - (a.enchant < b.enchant); break;
        case Quantity:     cmp = (a.quantity > b.quantity) - (a.quantity < b.quantity); break;
        case StartPrice:   cmp = (a.startPrice > b.startPrice) - (a.startPrice < b.startPrice); break;
        case BuyoutPrice:  cmp = (a.buyoutPrice > b.buyoutPrice) - (a.buyoutPrice < b.buyoutPrice); break;
        case PerItemPrice: cmp = (a.perItemPrice > b.perItemPrice) - (a.perItemPrice < b.perItemPrice); break;
        case TimeLeft:     cmp = (a.endTime > b.endTime) - (a.endTime < b.endTime); break;
        default: break;
        }
        return order == Qt::AscendingOrder ? cmp < 0 : cmp > 0;
        });
    m_sortColumn = column;
    m_sortOrder = order;
    endResetModel();
}

void LotsModel::setLots(const QJsonArray& lots, const QString& itemName)
{
    m_nowUtc = QDateTime::currentDateTimeUtc();

    beginResetModel();
    m_rows.clear();
    m_rows.reserve(lots.size());
    for (const QJsonValue& v : lots) {
        if (v.isObject())
            m_rows.append(parseLot(v.toObject(), itemName));
    }
    endResetModel();

    if (m_sortColumn >= 0)
        sort(m_sortColumn, m_sortOrder);
}

void LotsModel::showEmptyState(const QString& itemName)
{
    beginResetModel();
    m_rows.clear();
    Row r;
    r.isPlaceholder = true;
    r.lotName = QStringLiteral("ПРЕДМЕТА %1 НЕТ НА АУКЦИОНЕ").arg(itemName.toUpper());
    m_rows.append(r);
    endResetModel();
}

void LotsModel::clear()
{
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

void LotsModel::refreshTimeLeft()
{
    if (m_rows.isEmpty() || (m_rows.size() == 1 && m_rows.first().isPlaceholder))
        return;
    m_nowUtc = QDateTime::currentDateTimeUtc();
    emit dataChanged(index(0, TimeLeft), index(m_rows.size() - 1, TimeLeft), { Qt::DisplayRole });
}

LotsModel::Row LotsModel::parseLot(const QJsonObject& obj, const QString& itemName)
{
    Row r;
    r.raw = obj;
    r.lotName = itemName;
    r.quantity = obj.value("amount").toInt();
    r.startPrice = obj.value("startPrice").toInt();
    r.buyoutPrice = obj.value("buyoutPrice").toInt();
    r.perItemPrice = (r.quantity > 0) ? r.buyoutPrice / r.quantity : r.buyoutPrice;

    const QJsonObject additional = obj.value("additional").toObject();
    r.enchant = additional.value("ptn").toInt(-1);
    r.rarity = additional.value("qlt").toInt(-1);

    const QString endStr = obj.value("endTime").toString();
    r.endTime = QDateTime::fromString(endStr, Qt::ISODateWithMs);
    if (!r.endTime.isValid())
        r.endTime = QDateTime::fromString(endStr, Qt::ISODate);

    r.enchantStr = (r.enchant < 0) ? QStringLiteral("—") : QStringLiteral("+%1").arg(r.enchant);
    r.quantityStr = QString::number(r.quantity);
    r.startPriceStr = formatPrice(r.startPrice) + QStringLiteral(" ₽");
    r.buyoutPriceStr = formatPrice(r.buyoutPrice) + QStringLiteral(" ₽");
    r.perItemPriceStr = formatPrice(r.perItemPrice) + QStringLiteral(" ₽");

    return r;
}

QColor LotsModel::rarityColor(int rarity)
{
    switch (rarity) {
    case 0: return QColor(0xB0, 0xB0, 0xB0);
    case 1: return QColor(0x4C, 0xAF, 0x50);
    case 2: return QColor(0x21, 0x96, 0xF3);
    case 3: return QColor(0x9C, 0x27, 0xB0);
    case 4: return QColor(0xFF, 0x98, 0x00);
    case 5: return QColor(0xFF, 0xD7, 0x00);
    case 6: return QColor(0xFF, 0x40, 0x81);
    default: return QColor();
    }
}

QString LotsModel::formatPrice(int value)
{
    QString s = QString::number(value);
    if (value < 0) s = s.mid(1);
    QString result;
    int count = 0;
    for (int i = s.length() - 1; i >= 0; --i) {
        result.prepend(s[i]);
        ++count;
        if (count % 3 == 0 && i > 0)
            result.prepend(' ');
    }
    return (value < 0 ? "-" : "") + result;
}

QString LotsModel::formatTimeLeft(qint64 secsLeft)
{
    if (secsLeft <= 0)
        return QStringLiteral("00:00:00");

    const int h = static_cast<int>(secsLeft / 3600);
    const int m = static_cast<int>((secsLeft % 3600) / 60);
    const int s = static_cast<int>(secsLeft % 60);
    return QStringLiteral("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}
