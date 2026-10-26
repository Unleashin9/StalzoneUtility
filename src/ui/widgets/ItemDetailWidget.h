#pragma once
#include <QWidget>
#include <QSet>
#include <QDateTime>
#include "core/GameDatabase.h"

class QLabel;
class QTimer;
class FiltersBar;

class ItemDetailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ItemDetailWidget(QWidget* parent = nullptr);
    void setItem(const ItemData& item);

    void setBestPrice(int bestPrice, int avgPrice);
    void setBestPriceLoading();
    void setBestPriceEmpty();

    void setBestLot(int lotPrice, int amount, const QDateTime& endTime, int totalLoadedLots);

private slots:
    void updateTimeLeft();

signals:
    void raritiesChanged(const QSet<int>& rarities);
    void enchantChanged(int minValue, int maxValue);

private:
    void setupUi();
    QColor colorForRank(const QString& colorName) const;
    QString formatPrice(int value) const;
    void updateBestPriceBadge(int bestPrice, int avgPrice);

    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    FiltersBar* m_filtersBar = nullptr;

    QLabel* m_bestPriceIcon = nullptr;
    QLabel* m_bestPriceTitle = nullptr;
    QLabel* m_bestPriceValue = nullptr;
    QLabel* m_bestPriceUnit = nullptr;
    QLabel* m_bestPriceBadge = nullptr;

    QLabel* m_lotPriceValue = nullptr;
    QLabel* m_lotAmountValue = nullptr;
    QLabel* m_lotTimeValue = nullptr;
    QLabel* m_loadedLotsLabel = nullptr;

    QTimer* m_timeUpdateTimer = nullptr;
    QDateTime m_bestLotEndTime;
};
