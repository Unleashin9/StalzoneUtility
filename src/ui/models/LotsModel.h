#pragma once

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QColor>
#include <QVector>

class LotsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        LotName = 0,
        Enchant,
        Quantity,
        StartPrice,
        BuyoutPrice,
        PerItemPrice,
        TimeLeft,
        ColumnCount
    };

    explicit LotsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setLots(const QJsonArray& lots, const QString& itemName);
    void showEmptyState(const QString& itemName); // <--- НОВОЕ
    void clear();
    void refreshTimeLeft();

private:
    struct Row {
        QJsonObject raw;
        QString lotName;
        int enchant = -1;
        int rarity = -1;
        int quantity = 0;
        int startPrice = 0;
        int buyoutPrice = 0;
        int perItemPrice = 0;
        QDateTime endTime;
        bool isPlaceholder = false; // <--- НОВОЕ

        QString enchantStr;
        QString quantityStr;
        QString startPriceStr;
        QString buyoutPriceStr;
        QString perItemPriceStr;
    };

    static Row parseLot(const QJsonObject& obj, const QString& itemName);
    static QString formatPrice(int value);
    static QString formatTimeLeft(qint64 secsLeft);
    static QColor rarityColor(int rarity);

    QVector<Row> m_rows;
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
    mutable QDateTime m_nowUtc;
};
