#pragma once
#include <QWidget>
#include <QSet>
#include "core/GameDatabase.h"

class QLabel;
class FiltersBar;

class ItemDetailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ItemDetailWidget(QWidget* parent = nullptr);
    void setItem(const ItemData& item);

    // НОВОЕ: управление панелью "Лучшая цена"
    void setBestPrice(int pricePerItem);
    void setBestPriceLoading();
    void setBestPriceEmpty();

signals:
    void raritiesChanged(const QSet<int>& rarities);
    void enchantChanged(int minValue, int maxValue);

private:
    void setupUi();
    QColor colorForRank(const QString& colorName) const;

    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    FiltersBar* m_filtersBar = nullptr;

    // НОВОЕ: поля панели "Лучшая цена"
    QLabel* m_bestPriceIcon = nullptr;
    QLabel* m_bestPriceTitle = nullptr;
    QLabel* m_bestPriceValue = nullptr;
};
