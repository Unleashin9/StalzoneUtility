#pragma once
#include <QWidget>
#include <QSet>
#include <QVector>
#include "core/GameDatabase.h"

class QPushButton;
class QLineEdit;
class QLabel;
class QStackedWidget;
class QVBoxLayout;
class QListWidget;
class QButtonGroup;
class QNetworkAccessManager;
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
    void onDatabaseLoaded();
    void onCategoryClicked(const QString& groupName, const QString& displayName);
    void onQuickAccessClicked(int id);
    void onSearchTextChanged(const QString& text);

    // НОВЫЕ СЛОТЫ
    void onFavoriteToggled(const QString& itemId, bool isFavorite);
    void onCardClicked(const QString& itemId);

private:
    void setupUi();
    QWidget* createQuickAccessSidebar();
    QWidget* createRightArea();
    QWidget* createRightTopBar();
    QWidget* createRankPopup();
    QWidget* createSortPopup();
    void updateRankButtonText();
    void updateSortButtonText();
    void updateRankItemStyle(int index);
    void updateSortItemStyle(int index);
    void populateItems(const QString& groupName);
    void showPlaceholder(const QString& text);

    // НОВЫЕ МЕТОДЫ ДЛЯ ИЗБРАННОГО
    void loadFavorites();
    void saveFavorites();
    void updateFavoritesCounter();

    StalzoneAPI* m_api;
    GameDatabase* m_db;
    QNetworkAccessManager* m_netManager;

    QPushButton* m_catalogNavBtn = nullptr;
    QPushButton* m_monitoringNavBtn = nullptr;
    QPushButton* m_resaleNavBtn = nullptr;

    QPushButton* m_recentBtn = nullptr;
    QPushButton* m_favouritesBtn = nullptr;
    QButtonGroup* m_quickAccessGroup = nullptr;

    QVBoxLayout* m_categoryLayout = nullptr;
    QVector<QPushButton*> m_categoryButtons;
    QString m_currentGroup = "artefact";
    QButtonGroup* m_categoryGroup = nullptr;

    QLineEdit* m_searchEdit = nullptr;
    QListWidget* m_itemsList = nullptr;

    QPushButton* m_rankFilterBtn = nullptr;
    QPushButton* m_sortFilterBtn = nullptr;

    QWidget* m_rankPopup = nullptr;
    QWidget* m_sortPopup = nullptr;
    QList<QPushButton*> m_rankItems;
    QList<QPushButton*> m_sortItems;

    QSet<int> m_selectedRanks;
    int m_currentSort = 0;

    QSet<QString> m_allowedItemIds;
    QSet<QString> m_favoriteIds;  // НОВОЕ: ID избранных предметов
};
