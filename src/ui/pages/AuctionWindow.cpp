#include "AuctionWindow.h"
#include "core/ThemeManager.h"
#include "ui/widgets/ItemCard.h"
#include "ui/widgets/Toast.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QStackedWidget>
#include <QListWidget>
#include <QButtonGroup>
#include <QNetworkAccessManager>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <algorithm>

AuctionWindow::AuctionWindow(StalzoneAPI* api, GameDatabase* db, QWidget* parent)
    : QWidget(parent), m_api(api), m_db(db)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setObjectName("auctionRoot");
    setAttribute(Qt::WA_StyledBackground, true);

    m_netManager = new QNetworkAccessManager(this);
    m_allowedItemIds = { "wj6l2", "qj1lk", "g35n", "wg53" };

    loadFavorites(); // <--- ЗАГРУЖАЕМ ИЗБРАННОЕ ДО СОЗДАНИЯ UI

    setupUi();

    if (m_db->isLoaded()) {
        onDatabaseLoaded();
    }
    else {
        connect(m_db, &GameDatabase::loaded, this, &AuctionWindow::onDatabaseLoaded);
    }
}

void AuctionWindow::loadFavorites()
{
    QSettings settings;
    QStringList favList = settings.value("auction/favorites").toStringList();
    m_favoriteIds = QSet<QString>(favList.begin(), favList.end());
}

void AuctionWindow::saveFavorites()
{
    QSettings settings;
    QStringList favList(m_favoriteIds.begin(), m_favoriteIds.end());
    settings.setValue("auction/favorites", favList);
}

void AuctionWindow::updateFavoritesCounter()
{
    if (m_favouritesBtn) {
        m_favouritesBtn->setText(QString(" Избранное  %1").arg(m_favoriteIds.size()));
    }
}

void AuctionWindow::onFavoriteToggled(const QString& itemId, bool isFavorite)
{
    if (isFavorite) {
        m_favoriteIds.insert(itemId);
    }
    else {
        m_favoriteIds.remove(itemId);
    }
    saveFavorites();
    updateFavoritesCounter();
}

void AuctionWindow::onCardClicked(const QString& itemId)
{
    Toast::showMessage(this, QString("%1 — детали в разработке").arg(m_db->itemName(itemId)), false);
}

void AuctionWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === ВЕРХНЯЯ ПАНЕЛЬ ===
    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(60);
    topBar->setAttribute(Qt::WA_StyledBackground, true);

    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(10, 0, 10, 0);
    topBarLayout->setSpacing(15);

    auto* logoBtn = new QPushButton("STALZONE Utility", topBar);
    logoBtn->setObjectName("logoBtn");
    logoBtn->setIcon(ThemeManager::instance().getIcon(
        "szutility", ThemeManager::instance().accentColor(), QString()));
    logoBtn->setIconSize(QSize(32, 32));
    logoBtn->setCursor(Qt::PointingHandCursor);
    logoBtn->setFlat(true);
    logoBtn->setFocusPolicy(Qt::NoFocus);
    logoBtn->setStyleSheet("background: transparent; border: none;");
    connect(logoBtn, &QPushButton::clicked, this, &AuctionWindow::backRequested);

    auto* navLayout = new QHBoxLayout();
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(10);

    auto createNavButton = [this, topBar](const QString& text, const QString& iconName) -> QPushButton* {
        auto* btn = new QPushButton(" " + text, topBar);
        btn->setObjectName("navBtn");
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(44);
        btn->setIconSize(QSize(20, 20));

        auto updateIcon = [btn, iconName](bool checked) {
            QColor color = checked ? ThemeManager::instance().accentColor() : QColor("#A8998A");
            btn->setIcon(ThemeManager::instance().getIcon(iconName, color, "AuctionWindow"));
            };
        updateIcon(false);
        connect(btn, &QPushButton::toggled, btn, updateIcon);
        return btn;
        };

    m_catalogNavBtn = createNavButton("Каталог", "catalog");
    m_monitoringNavBtn = createNavButton("Мониторинг", "monitoring");
    m_resaleNavBtn = createNavButton("Перепродажа", "resale");
    m_catalogNavBtn->setChecked(true);

    navLayout->addWidget(m_catalogNavBtn);
    navLayout->addWidget(m_monitoringNavBtn);
    navLayout->addWidget(m_resaleNavBtn);

    topBarLayout->addWidget(logoBtn, 0, Qt::AlignVCenter);
    topBarLayout->addLayout(navLayout);
    topBarLayout->addStretch();

    mainLayout->addWidget(topBar);

    // === КОНТЕНТ ===
    auto* contentStack = new QStackedWidget(this);

    auto* catalogPage = new QWidget(contentStack);
    auto* catalogLayout = new QHBoxLayout(catalogPage);
    catalogLayout->setContentsMargins(0, 0, 0, 0);
    catalogLayout->setSpacing(0);
    catalogLayout->addWidget(createQuickAccessSidebar());
    catalogLayout->addWidget(createRightArea(), 1);
    contentStack->addWidget(catalogPage);

    auto* monitoringPage = new QWidget(contentStack);
    auto* monLayout = new QVBoxLayout(monitoringPage);
    monLayout->addWidget(new QLabel("Мониторинг в разработке...", monitoringPage), 0, Qt::AlignCenter);
    contentStack->addWidget(monitoringPage);

    auto* resalePage = new QWidget(contentStack);
    auto* resLayout = new QVBoxLayout(resalePage);
    resLayout->addWidget(new QLabel("Перепродажа в разработке...", resalePage), 0, Qt::AlignCenter);
    contentStack->addWidget(resalePage);

    mainLayout->addWidget(contentStack, 1);

    connect(m_catalogNavBtn, &QPushButton::clicked, this, [contentStack]() { contentStack->setCurrentIndex(0); });
    connect(m_monitoringNavBtn, &QPushButton::clicked, this, [contentStack]() { contentStack->setCurrentIndex(1); });
    connect(m_resaleNavBtn, &QPushButton::clicked, this, [contentStack]() { contentStack->setCurrentIndex(2); });

    auto* bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    bottomBar->setFixedHeight(60);
    bottomBar->setAttribute(Qt::WA_StyledBackground, true);
    auto* bottomBarLayout = new QHBoxLayout(bottomBar);
    bottomBarLayout->setContentsMargins(50, 0, 50, 0);
    bottomBarLayout->addStretch();
    mainLayout->addWidget(bottomBar);

    m_rankPopup = createRankPopup();
    m_sortPopup = createSortPopup();
}

QWidget* AuctionWindow::createQuickAccessSidebar()
{
    auto* sidebar = new QWidget(this);
    sidebar->setObjectName("quickAccessSidebar");
    sidebar->setFixedWidth(300);
    sidebar->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(15, 20, 15, 20);
    layout->setSpacing(6);

    auto* quickTitle = new QLabel("БЫСТРЫЙ ДОСТУП", sidebar);
    quickTitle->setObjectName("quickAccessTitle");
    layout->addWidget(quickTitle);
    layout->addSpacing(8);

    m_quickAccessGroup = new QButtonGroup(this);
    m_quickAccessGroup->setExclusive(true);

    m_recentBtn = new QPushButton(" Недавно просмотренное", sidebar);
    m_recentBtn->setObjectName("quickAccessBtn");
    m_recentBtn->setCheckable(true);
    m_recentBtn->setIconSize(QSize(18, 18));
    m_recentBtn->setCursor(Qt::PointingHandCursor);
    m_recentBtn->setMinimumHeight(38);
    m_recentBtn->setIcon(ThemeManager::instance().getIcon("recently", QColor("#A8998A"), "AuctionWindow"));
    m_quickAccessGroup->addButton(m_recentBtn, 0);

    m_favouritesBtn = new QPushButton(QString(" Избранное  %1").arg(m_favoriteIds.size()), sidebar);
    m_favouritesBtn->setObjectName("quickAccessBtn");
    m_favouritesBtn->setCheckable(true);
    m_favouritesBtn->setIconSize(QSize(18, 18));
    m_favouritesBtn->setCursor(Qt::PointingHandCursor);
    m_favouritesBtn->setMinimumHeight(38);
    m_favouritesBtn->setIcon(ThemeManager::instance().getIcon("favourite", QColor("#A8998A"), "AuctionWindow"));
    m_quickAccessGroup->addButton(m_favouritesBtn, 1);

    connect(m_quickAccessGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &AuctionWindow::onQuickAccessClicked);

    layout->addWidget(m_recentBtn);
    layout->addWidget(m_favouritesBtn);
    layout->addSpacing(15);

    auto* catTitle = new QLabel("КАТЕГОРИИ", sidebar);
    catTitle->setObjectName("quickAccessTitle");
    layout->addWidget(catTitle);
    layout->addSpacing(8);

    m_categoryGroup = new QButtonGroup(this);
    m_categoryGroup->setExclusive(true);

    m_categoryLayout = new QVBoxLayout();
    m_categoryLayout->setSpacing(4);

    QList<QPair<QString, QString>> categories = {
        {"all", "Все предметы"}, {"weapon", "Оружие"}, {"armor", "Костюмы"},
        {"containers", "Рюкзаки и контейнеры"}, {"artefact", "Артефакты"},
        {"attachment", "Обвесы и модули"}, {"bullet", "Патроны и гранаты"},
        {"medicine", "Медикаменты и еда"}, {"misc", "Разное"}, {"other", "Прочее"}
    };

    for (int i = 0; i < categories.size(); ++i) {
        auto& cat = categories[i];
        auto* btn = new QPushButton(sidebar);
        btn->setObjectName("categoryBtn");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(38);
        btn->setProperty("groupName", cat.first);
        btn->setText(cat.second);
        m_categoryGroup->addButton(btn, i);
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            onCategoryClicked(btn->property("groupName").toString(), btn->text());
            });
        m_categoryLayout->addWidget(btn);
        m_categoryButtons.append(btn);
    }

    layout->addLayout(m_categoryLayout);
    layout->addStretch();

    return sidebar;
}

void AuctionWindow::onQuickAccessClicked(int id)
{
    Q_UNUSED(id);
    if (m_categoryGroup) {
        m_categoryGroup->setExclusive(false);
        for (auto* btn : m_categoryButtons) btn->setChecked(false);
        m_categoryGroup->setExclusive(true);
    }
    showPlaceholder("В разработке");
}

void AuctionWindow::onCategoryClicked(const QString& groupName, const QString& displayName)
{
    Q_UNUSED(displayName);
    m_currentGroup = groupName;

    if (m_quickAccessGroup) {
        m_quickAccessGroup->setExclusive(false);
        m_recentBtn->setChecked(false);
        m_favouritesBtn->setChecked(false);
        m_quickAccessGroup->setExclusive(true);
    }

    populateItems(groupName);
    if (m_searchEdit) m_searchEdit->clear();
}

void AuctionWindow::showPlaceholder(const QString& text)
{
    if (!m_itemsList) return;
    m_itemsList->clear();
    auto* item = new QListWidgetItem(text, m_itemsList);
    item->setFlags(Qt::NoItemFlags);
    item->setTextAlignment(Qt::AlignCenter);
}

void AuctionWindow::populateItems(const QString& groupName)
{
    if (!m_itemsList) return;
    m_itemsList->clear();

    bool isAllowedCategory = (groupName == "all" || groupName == "weapon" ||
        groupName == "armor" || groupName == "containers" ||
        groupName == "artefact");
    if (!isAllowedCategory) {
        showPlaceholder("В разработке");
        return;
    }

    QVector<ItemData> items = m_db->itemsByGroup(groupName);
    QVector<ItemData> filteredItems;

    const QStringList rankColors = { "RANK_LEGEND", "RANK_MASTER", "RANK_VETERAN",
                                     "RANK_STALKER", "RANK_NEWBIE", "RANK_ROOKIE" };
    QSet<QString> selectedColors;
    for (int idx : m_selectedRanks) {
        if (idx >= 0 && idx < rankColors.size()) {
            selectedColors.insert(rankColors[idx]);
        }
    }

    // Хелпер: относится ли предмет к "Обычному" качеству
    // (RANK_ROOKIE + артефакты с неизвестной/пустой редкостью)
    auto isCommonItem = [](const ItemData& item) -> bool {
        if (item.color == "RANK_ROOKIE") return true;
        if (item.color == "DEFAULT" || item.color == "COMMON" || item.color == "RANK_DEFAULT") return true;

        // Артефакты обычного качества: цвет не относится к одной из 5 высших редкостей
        if (item.category == "artefact") {
            static const QStringList highRanks = {
                "RANK_LEGEND", "RANK_MASTER", "RANK_VETERAN",
                "RANK_STALKER", "RANK_NEWBIE"
            };
            if (item.color.isEmpty() || !highRanks.contains(item.color)) {
                return true;
            }
        }
        return false;
        };

    // Хелпер для сортировки: индекс редкости (неизвестные = Обычный)
    auto rankIndexOf = [&rankColors, &isCommonItem](const ItemData& item) -> int {
        int idx = rankColors.indexOf(item.color);
        if (idx < 0 && isCommonItem(item)) {
            return rankColors.size() - 1; // позиция RANK_ROOKIE
        }
        return idx;
        };

    // 1. Фильтрация
    for (const auto& item : items) {
        if (!m_allowedItemIds.contains(item.id)) continue;

        if (!selectedColors.isEmpty()) {
            bool matches = selectedColors.contains(item.color);

            // Если выбран «Обычный» — также пропускаем артефакты обычного качества
            if (!matches && selectedColors.contains("RANK_ROOKIE") && isCommonItem(item)) {
                matches = true;
            }

            if (!matches) continue;
        }

        filteredItems.append(item);
    }

    // 2. Сортировка
    if (m_currentSort == 0) {
        std::sort(filteredItems.begin(), filteredItems.end(),
            [&rankIndexOf](const ItemData& a, const ItemData& b) {
                return rankIndexOf(a) < rankIndexOf(b);
            });
    }
    else if (m_currentSort == 1) {
        std::sort(filteredItems.begin(), filteredItems.end(), [](const ItemData& a, const ItemData& b) {
            return a.name.localeAwareCompare(b.name) < 0;
            });
    }
    else if (m_currentSort == 2) {
        std::sort(filteredItems.begin(), filteredItems.end(), [](const ItemData& a, const ItemData& b) {
            return a.name.localeAwareCompare(b.name) > 0;
            });
    }

    if (filteredItems.isEmpty()) {
        showPlaceholder("Ничего не найдено");
        return;
    }

    // 3. Отрисовка
    for (const auto& item : filteredItems) {
        auto* listItem = new QListWidgetItem(m_itemsList);
        listItem->setText(item.name);
        listItem->setSizeHint(QSize(190, 230));

        auto* wrapper = new QWidget();
        auto* wrapperLayout = new QVBoxLayout(wrapper);
        wrapperLayout->setContentsMargins(10, 10, 10, 10);

        bool isFav = m_favoriteIds.contains(item.id);
        auto* card = new ItemCard(item, m_netManager, isFav, wrapper);

        connect(card, &ItemCard::favoriteToggled, this, &AuctionWindow::onFavoriteToggled);
        connect(card, &ItemCard::cardClicked, this, &AuctionWindow::onCardClicked);

        wrapperLayout->addWidget(card);
        m_itemsList->setItemWidget(listItem, wrapper);
    }
}

void AuctionWindow::onDatabaseLoaded()
{
    for (auto* btn : m_categoryButtons) {
        QString group = btn->property("groupName").toString();
        int count = m_db->itemsByGroup(group).size();
        QString displayName;
        if (group == "all") displayName = "Все предметы";
        else if (group == "weapon") displayName = "Оружие";
        else if (group == "armor") displayName = "Костюмы";
        else if (group == "containers") displayName = "Рюкзаки и контейнеры";
        else if (group == "artefact") displayName = "Артефакты";
        else if (group == "attachment") displayName = "Обвесы и модули";
        else if (group == "bullet") displayName = "Патроны и гранаты";
        else if (group == "medicine") displayName = "Медикаменты и еда";
        else if (group == "misc") displayName = "Разное";
        else if (group == "other") displayName = "Прочее";
        btn->setText(QString("%1 (%2)").arg(displayName).arg(count));
    }

    for (auto* btn : m_categoryButtons) {
        if (btn->property("groupName").toString() == "weapon") {
            btn->setChecked(true);
            m_currentGroup = "weapon";
            populateItems("weapon");
            break;
        }
    }
}

void AuctionWindow::onSearchTextChanged(const QString& text)
{
    if (!m_itemsList) return;
    for (int i = 0; i < m_itemsList->count(); ++i) {
        QListWidgetItem* item = m_itemsList->item(i);
        if (!(item->flags() & Qt::ItemIsEnabled)) continue;
        bool matches = text.isEmpty() || item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

QWidget* AuctionWindow::createRightArea()
{
    auto* rightArea = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightArea);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    rightLayout->addWidget(createRightTopBar());

    m_itemsList = new QListWidget(rightArea);
    m_itemsList->setObjectName("itemsList");
    m_itemsList->setViewMode(QListView::IconMode);
    m_itemsList->setResizeMode(QListView::Adjust);
    m_itemsList->setGridSize(QSize(190, 230));
    m_itemsList->setMovement(QListView::Static);
    m_itemsList->setWordWrap(true);
    m_itemsList->setSelectionMode(QAbstractItemView::NoSelection);
    m_itemsList->setFocusPolicy(Qt::NoFocus);

    rightLayout->addWidget(m_itemsList, 1);

    return rightArea;
}

QWidget* AuctionWindow::createRightTopBar()
{
    auto* bar = new QWidget(this);
    bar->setObjectName("rightTopBar");
    bar->setFixedHeight(60);
    bar->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(20, 10, 20, 10);
    layout->setSpacing(12);

    m_searchEdit = new QLineEdit(bar);
    m_searchEdit->setObjectName("searchEdit");
    m_searchEdit->setPlaceholderText("Поиск предмета по имени...");
    m_searchEdit->addAction(ThemeManager::instance().getIcon("search", QColor("#A8998A"), "AuctionWindow"), QLineEdit::LeadingPosition);
    m_searchEdit->setMinimumHeight(40);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AuctionWindow::onSearchTextChanged);

    m_rankFilterBtn = new QPushButton(" Ранг: Все", bar);
    m_rankFilterBtn->setObjectName("filterBtn");
    m_rankFilterBtn->setIcon(ThemeManager::instance().getIcon("filter", QColor("#A8998A"), "AuctionWindow"));
    m_rankFilterBtn->setIconSize(QSize(18, 18));
    m_rankFilterBtn->setCursor(Qt::PointingHandCursor);
    m_rankFilterBtn->setMinimumHeight(40);
    m_rankFilterBtn->setMinimumWidth(180);
    connect(m_rankFilterBtn, &QPushButton::clicked, this, &AuctionWindow::onRankFilterClicked);

    m_sortFilterBtn = new QPushButton(" Сортировка: По рангу", bar);
    m_sortFilterBtn->setObjectName("filterBtn");
    m_sortFilterBtn->setIcon(ThemeManager::instance().getIcon("filter", QColor("#A8998A"), "AuctionWindow"));
    m_sortFilterBtn->setIconSize(QSize(18, 18));
    m_sortFilterBtn->setCursor(Qt::PointingHandCursor);
    m_sortFilterBtn->setMinimumHeight(40);
    m_sortFilterBtn->setMinimumWidth(230);
    connect(m_sortFilterBtn, &QPushButton::clicked, this, &AuctionWindow::onSortFilterClicked);

    layout->addWidget(m_searchEdit, 1);
    layout->addWidget(m_rankFilterBtn);
    layout->addWidget(m_sortFilterBtn);

    return bar;
}

QWidget* AuctionWindow::createRankPopup()
{
    auto* popup = new QWidget(this, Qt::Popup);
    popup->setObjectName("dropdownPopup");
    popup->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(popup);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(2);

    QStringList ranks = { "Легенда", "Мастер", "Ветеран", "Сталкер", "Новичок", "Обычный" };

    for (int i = 0; i < ranks.size(); ++i) {
        auto* btn = new QPushButton(" " + ranks[i], popup);
        btn->setObjectName("dropdownItem");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(34);
        btn->setMinimumWidth(220);
        updateRankItemStyle(i);
        connect(btn, &QPushButton::clicked, this, [this, i]() { onRankClicked(i); });
        layout->addWidget(btn);
        m_rankItems.append(btn);
    }

    auto* sep = new QFrame(popup);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background-color: #3A2F26; margin: 6px 0;");
    layout->addWidget(sep);

    auto* resetBtn = new QPushButton(" Сбросить фильтр", popup);
    resetBtn->setObjectName("dropdownItem");
    resetBtn->setIcon(ThemeManager::instance().getIcon("filterremove", QColor("#A8998A"), "AuctionWindow"));
    resetBtn->setIconSize(QSize(16, 16));
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setMinimumHeight(34);
    resetBtn->setMinimumWidth(220);
    connect(resetBtn, &QPushButton::clicked, this, &AuctionWindow::onResetRanksClicked);
    layout->addWidget(resetBtn);

    popup->hide();
    return popup;
}

QWidget* AuctionWindow::createSortPopup()
{
    auto* popup = new QWidget(this, Qt::Popup);
    popup->setObjectName("dropdownPopup");
    popup->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(popup);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(2);

    QStringList sorts = { "По рангу", "От А до Я", "От Я до А" };

    for (int i = 0; i < sorts.size(); ++i) {
        auto* btn = new QPushButton(" " + sorts[i], popup);
        btn->setObjectName("dropdownItem");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(34);
        btn->setMinimumWidth(200);
        updateSortItemStyle(i);
        connect(btn, &QPushButton::clicked, this, [this, i]() { onSortClicked(i); });
        layout->addWidget(btn);
        m_sortItems.append(btn);
    }

    popup->hide();
    return popup;
}

void AuctionWindow::updateRankItemStyle(int index)
{
    if (index < 0 || index >= m_rankItems.size()) return;
    // НОВЫЕ ЦВЕТА:
    const QStringList rankColors = { "#ffdd66", "#ff5767", "#d968c4", "#8d8dff", "#7def7d", "#eeeeee" };
    QString color = rankColors[index];
    bool active = m_selectedRanks.contains(index);
    QPushButton* btn = m_rankItems[index];
    btn->setChecked(active);

    if (active) {
        QColor c(color);
        btn->setStyleSheet(QString(
            "QPushButton { background-color: rgba(%1, %2, %3, 0.15); color: %4; border: 1px solid %4; border-left: 3px solid %4; border-radius: 6px; font-weight: bold; text-align: left; padding: 0px 10px; font-size: 13px; }"
        ).arg(c.red()).arg(c.green()).arg(c.blue()).arg(color));
    }
    else {
        btn->setStyleSheet(
            "QPushButton { background-color: transparent; color: #A8998A; border: 1px solid transparent; border-left: 3px solid transparent; border-radius: 6px; text-align: left; padding: 0px 10px; font-size: 13px; }"
            "QPushButton:hover { background-color: rgba(255, 255, 255, 0.05); }"
        );
    }
}

void AuctionWindow::updateSortItemStyle(int index)
{
    if (index < 0 || index >= m_sortItems.size()) return;
    QColor accent = ThemeManager::instance().accentColor();
    bool active = (m_currentSort == index);
    QPushButton* btn = m_sortItems[index];
    btn->setChecked(active);

    if (active) {
        btn->setStyleSheet(QString(
            "QPushButton { background-color: rgba(%1, %2, %3, 0.15); color: %4; border: 1px solid %4; border-left: 3px solid %4; border-radius: 6px; font-weight: bold; text-align: left; padding: 0px 10px; font-size: 13px; }"
        ).arg(accent.red()).arg(accent.green()).arg(accent.blue()).arg(accent.name()));
    }
    else {
        btn->setStyleSheet(
            "QPushButton { background-color: transparent; color: #A8998A; border: 1px solid transparent; border-left: 3px solid transparent; border-radius: 6px; text-align: left; padding: 0px 10px; font-size: 13px; }"
            "QPushButton:hover { background-color: rgba(255, 255, 255, 0.05); }"
        );
    }
}

void AuctionWindow::onRankFilterClicked()
{
    if (m_rankPopup->isVisible()) { m_rankPopup->hide(); return; }
    m_rankPopup->adjustSize();
    m_rankPopup->move(m_rankFilterBtn->mapToGlobal(QPoint(0, m_rankFilterBtn->height() + 4)));
    auto* effect = new QGraphicsOpacityEffect(m_rankPopup);
    effect->setOpacity(0.0);
    m_rankPopup->setGraphicsEffect(effect);
    m_rankPopup->show();
    auto* anim = new QPropertyAnimation(effect, "opacity", m_rankPopup);
    anim->setDuration(150);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AuctionWindow::onSortFilterClicked()
{
    if (m_sortPopup->isVisible()) { m_sortPopup->hide(); return; }
    m_sortPopup->adjustSize();
    m_sortPopup->move(m_sortFilterBtn->mapToGlobal(QPoint(0, m_sortFilterBtn->height() + 4)));
    auto* effect = new QGraphicsOpacityEffect(m_sortPopup);
    effect->setOpacity(0.0);
    m_sortPopup->setGraphicsEffect(effect);
    m_sortPopup->show();
    auto* anim = new QPropertyAnimation(effect, "opacity", m_sortPopup);
    anim->setDuration(150);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AuctionWindow::onRankClicked(int rankIndex)
{
    if (m_selectedRanks.contains(rankIndex)) m_selectedRanks.remove(rankIndex);
    else m_selectedRanks.insert(rankIndex);
    updateRankItemStyle(rankIndex);
    updateRankButtonText();
    populateItems(m_currentGroup);
}

void AuctionWindow::onSortClicked(int sortIndex)
{
    m_currentSort = sortIndex;
    for (int i = 0; i < m_sortItems.size(); ++i) updateSortItemStyle(i);
    updateSortButtonText();
    m_sortPopup->hide();
    populateItems(m_currentGroup);
}

void AuctionWindow::onResetRanksClicked()
{
    m_selectedRanks.clear();
    for (int i = 0; i < m_rankItems.size(); ++i) updateRankItemStyle(i);
    updateRankButtonText();
    m_rankPopup->hide();
    populateItems(m_currentGroup);
}

void AuctionWindow::updateRankButtonText()
{
    if (m_selectedRanks.isEmpty()) m_rankFilterBtn->setText(" Ранг: Все");
    else if (m_selectedRanks.size() == 1) {
        int idx = *m_selectedRanks.begin();
        const QStringList ranks = { "Легенда", "Мастер", "Ветеран", "Сталкер", "Новичок", "Обычный" };
        m_rankFilterBtn->setText(" Ранг: " + ranks[idx]);
    }
    else m_rankFilterBtn->setText(QString(" Ранг: %1").arg(m_selectedRanks.size()));
}

void AuctionWindow::updateSortButtonText()
{
    const QStringList sorts = { "По рангу", "От А до Я", "От Я до А" };
    m_sortFilterBtn->setText(" Сортировка: " + sorts[m_currentSort]);
}
