#pragma once
#include <QWidget>
#include "core/GameDatabase.h"

class QLabel;
class QPushButton;

class ItemDetailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ItemDetailWidget(QWidget* parent = nullptr);
    void setItem(const ItemData& item);

signals:
    void backRequested();

private:
    void setupUi();
    QColor colorForRank(const QString& colorName) const;

    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QPushButton* m_backBtn = nullptr;
};
