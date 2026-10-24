#pragma once

#include <QWidget>
#include <QJsonArray>

class QTableWidget;
class QPushButton;
class QComboBox;
class StalzoneAPI;

class AuctionWindow : public QWidget
{
    Q_OBJECT

public:
    // Передаем указатель на API, чтобы окно могло делать запросы
    explicit AuctionWindow(StalzoneAPI* api, QWidget* parent = nullptr);

private slots:
    void onRefreshClicked();
    void onLotsReceived(const QJsonArray& lots);
    void onApiError(const QString& error);

private:
    void setupUi();
    void populateTable(const QJsonArray& lots);

    StalzoneAPI* m_api;
    QTableWidget* m_table;
    QPushButton* m_refreshBtn;
    QComboBox* m_itemSelector;
};
