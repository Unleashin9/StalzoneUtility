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

signals:
    void raritiesChanged(const QSet<int>& rarities);
    void enchantChanged(int minValue, int maxValue);

private:
    void setupUi();
    QColor colorForRank(const QString& colorName) const;

    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    FiltersBar* m_filtersBar = nullptr;
};
