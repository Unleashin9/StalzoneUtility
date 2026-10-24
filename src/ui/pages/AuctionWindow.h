#pragma once
#include <QWidget>
#include <QJsonArray>

class QTableWidget;
class QPushButton;
class QComboBox;
class StalzoneAPI;
class GameDatabase; // <--- ДОБАВИЛИ

class AuctionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AuctionWindow(StalzoneAPI* api, GameDatabase* db, QWidget* parent = nullptr);

private slots:
    void onRefreshClicked();
    void onLotsReceived(const QJsonArray& lots);
    void onApiError(const QString& error);

private:
    void setupUi();
    void populateTable(const QJsonArray& lots);

    StalzoneAPI* m_api;
    GameDatabase* m_db; // <--- ДОБАВИЛИ
    QTableWidget* m_table;
    QPushButton* m_refreshBtn;
    QComboBox* m_itemSelector;
};
