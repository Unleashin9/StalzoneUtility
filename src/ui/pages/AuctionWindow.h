#pragma once
#include <QWidget>
#include <QSet>

class QPushButton;
class QLineEdit;
class QLabel;
class StalzoneAPI;
class GameDatabase;

class AuctionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AuctionWindow(StalzoneAPI* api, GameDatabase* db, QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void onRankFilterClicked();
    void onSortFilterClicked();
    void onRankClicked(int rankIndex);
    void onSortClicked(int sortIndex);
    void onResetRanksClicked();

private:
    void setupUi();
    QWidget* createQuickAccessSidebar();
    QWidget* createRightTopBar();
    QWidget* createRankPopup();
    QWidget* createSortPopup();
    void updateRankButtonText();
    void updateSortButtonText();
    void updateRankItemStyle(int index);
    void updateSortItemStyle(int index);

    StalzoneAPI* m_api;
    GameDatabase* m_db;

    // Навигация (каталог, мониторинг, перепродажа)
    QPushButton* m_catalogNavBtn = nullptr;

    // Быстрый доступ
    QPushButton* m_recentBtn = nullptr;
    QPushButton* m_favouritesBtn = nullptr;
    int m_favouritesCount = 0;

    // Фильтры
    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_rankFilterBtn = nullptr;
    QPushButton* m_sortFilterBtn = nullptr;

    // Выпадающие списки
    QWidget* m_rankPopup = nullptr;
    QWidget* m_sortPopup = nullptr;
    QList<QPushButton*> m_rankItems;
    QList<QPushButton*> m_sortItems;

    // Состояние
    QSet<int> m_selectedRanks;
    int m_currentSort = 0;
};
