#pragma once
#include <QWidget>

class QLabel;

class SidebarAuctionStats : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarAuctionStats(QWidget* parent = nullptr);

    void reset();
    void setLoading();
    void setStats(int lotsCount, double avgPrice, double minPrice, double maxPrice);

private:
    void setupUi();
    QString formatPrice(int value) const; // <--- НОВОЕ

    QLabel* m_lotsValue = nullptr;
    QLabel* m_avgValue = nullptr;
    QLabel* m_minValue = nullptr;
    QLabel* m_maxValue = nullptr;
};
