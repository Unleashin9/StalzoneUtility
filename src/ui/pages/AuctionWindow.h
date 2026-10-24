#pragma once
#include <QWidget>
#include <QJsonArray>

class QTableWidget;
class QPushButton;
class QComboBox;
class StalzoneAPI;
class GameDatabase;

class AuctionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AuctionWindow(StalzoneAPI* api, GameDatabase* db, QWidget* parent = nullptr);

signals:
    void backRequested(); // <--- СИГНАЛ ДЛЯ ВОЗВРАТА

private slots:
    void onRefreshClicked();
    void onLotsReceived(const QJsonArray& lots);
    void onApiError(const QString& error);

private:
    void setupUi();
    void populateTable(const QJsonArray& lots);

    StalzoneAPI* m_api;
    GameDatabase* m_db;
    QTableWidget* m_table;
    QPushButton* m_refreshBtn;
    QComboBox* m_itemSelector;
};
