#pragma once
#include <QWidget>
#include <QSet>
#include <QVector>
#include <QJsonArray>
#include "core/GameDatabase.h"

class QPushButton;
class QLineEdit;
class QLabel;
class QStackedWidget;
class QVBoxLayout;
class QListWidget;
class QButtonGroup;
class QNetworkAccessManager;
class SidebarButton;
class SidebarItemInfo;
class SidebarAuctionStats;
class ItemDetailWidget;
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
    void onFavoriteToggled(const QString& itemId, bool isFavorite);
    void onCardClicked(const ItemData& item);
    void onDetailLotsReceived(const QJsonArray& lots);
    void onDetailApiError(const QString& error);
    void onRaritiesChanged(const QSet<int>& selectedRarities);
    void onEnchantChanged(int minValue, int maxValue);

private:
    void setupUi();
    QWidget* createQuickAccessSidebar();
    QWidget* createEmptySidebar();
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
    SidebarButton* m_favouritesBtn = nullptr;
    QButtonGroup* m_quickAccessGroup = nullptr;

    QVBoxLayout* m_categoryLayout = nullptr;
    QVector<SidebarButton*> m_categoryButtons;
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
    QSet<QString> m_favoriteIds;

    // Сайдбары (только по одному экземпляру!)
    QWidget* m_normalSidebar = nullptr;
    QWidget* m_emptySidebar = nullptr;

    // Правая часть
    QStackedWidget* m_rightStack = nullptr;
    ItemDetailWidget* m_itemDetailWidget = nullptr;

    // Виджеты в пустом сайдбаре
    SidebarItemInfo* m_sidebarItemInfo = nullptr;
    SidebarAuctionStats* m_auctionStats = nullptr;

    // Фильтры (заглушки)
    QSet<int> m_selectedRarities;
    int m_enchantMin = 0;
    int m_enchantMax = 0;

    QString m_currentDetailItemId;
};
