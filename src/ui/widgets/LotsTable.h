#pragma once
#include <QWidget>
#include <QJsonArray>
#include <QDateTime>
#include <QVector>

class QTableWidget;
class QTimer;

class LotsTable : public QWidget
{
    Q_OBJECT
public:
    explicit LotsTable(QWidget* parent = nullptr);

    void setLots(const QJsonArray& lots, const QString& itemId,
        const QString& itemName, const QString& itemIconPath);
    void clear();

private slots:
    void updateTimers();

private:
    void setupUi();
    QString formatPrice(int value) const;
    QString formatTimeLeft(qint64 secsLeft) const;

    QTableWidget* m_table = nullptr;
    QTimer* m_timer = nullptr;
    QVector<QDateTime> m_endTimes;
};
