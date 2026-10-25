#include "AuctionWindow.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

AuctionWindow::AuctionWindow(StalzoneAPI* api, GameDatabase* db, QWidget* parent)
    : QWidget(parent), m_api(api), m_db(db)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setObjectName("auctionRoot");
    setAttribute(Qt::WA_StyledBackground, true);

    setupUi();
}

void AuctionWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // =========================================================
    // === ВЕРХНЯЯ ПАНЕЛЬ ===
    // =========================================================
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

    // Навигация
    auto* navLayout = new QHBoxLayout();
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(10);

    auto createNavButton = [this, topBar](const QString& text, const QString& iconName) -> QPushButton* {
        auto* btn = new QPushButton(" " + text, topBar);
        btn->setObjectName("navBtn");
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(36);
        btn->setIconSize(QSize(18, 18));

        auto updateIcon = [btn, iconName](bool checked) {
            QColor color = checked ? ThemeManager::instance().accentColor() : QColor("#A8998A");
            btn->setIcon(ThemeManager::instance().getIcon(iconName, color, "AuctionWindow"));
            };

        updateIcon(false);
        connect(btn, &QPushButton::toggled, btn, updateIcon);

        return btn;
        };

    m_catalogNavBtn = createNavButton("Каталог", "catalog");
    auto* monitoringBtn = createNavButton("Мониторинг", "monitoring");
    auto* resaleBtn = createNavButton("Перепродажа", "resale");

    m_catalogNavBtn->setChecked(true);

    navLayout->addWidget(m_catalogNavBtn);
    navLayout->addWidget(monitoringBtn);
    navLayout->addWidget(resaleBtn);

    topBarLayout->addWidget(logoBtn, 0, Qt::AlignVCenter);
    topBarLayout->addLayout(navLayout);
    topBarLayout->addStretch();

    mainLayout->addWidget(topBar);

    // =========================================================
    // === КОНТЕНТ: САЙДБАР + ПРАВАЯ ЧАСТЬ ===
    // =========================================================
    auto* contentArea = new QWidget(this);
    contentArea->setObjectName("auctionContent");
    auto* contentAreaLayout = new QHBoxLayout(contentArea);
    contentAreaLayout->setContentsMargins(0, 0, 0, 0);
    contentAreaLayout->setSpacing(0);

    contentAreaLayout->addWidget(createQuickAccessSidebar());

    auto* rightArea = new QWidget(contentArea);
    auto* rightLayout = new QVBoxLayout(rightArea);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    rightLayout->addWidget(createRightTopBar());

    // Пустая зона контента (сюда потом таблица лотов)
    auto* mainContent = new QWidget(rightArea);
    mainContent->setObjectName("catalogContent");
    auto* mainContentLayout = new QVBoxLayout(mainContent);
    mainContentLayout->setContentsMargins(20, 20, 20, 20);
    mainContentLayout->setAlignment(Qt::AlignCenter);

    auto* placeholder = new QLabel("Здесь будет таблица предметов", mainContent);
    placeholder->setStyleSheet("color: #A8998A; font-size: 16px;");
    mainContentLayout->addWidget(placeholder);

    rightLayout->addWidget(mainContent, 1);
    contentAreaLayout->addWidget(rightArea, 1);

    mainLayout->addWidget(contentArea, 1);

    // =========================================================
    // === НИЖНЯЯ ПАНЕЛЬ ===
    // =========================================================
    auto* bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    bottomBar->setFixedHeight(60);
    bottomBar->setAttribute(Qt::WA_StyledBackground, true);

    auto* bottomBarLayout = new QHBoxLayout(bottomBar);
    bottomBarLayout->setContentsMargins(50, 0, 50, 0);
    bottomBarLayout->addStretch();

    mainLayout->addWidget(bottomBar);

    // Создаём попапы (скрытые)
    m_rankPopup = createRankPopup();
    m_sortPopup = createSortPopup();
}

// =========================================================
// === SIDEBAR: БЫСТРЫЙ ДОСТУП ===
// =========================================================
QWidget* AuctionWindow::createQuickAccessSidebar()
{
    auto* sidebar = new QWidget(this);
    sidebar->setObjectName("quickAccessSidebar");
    sidebar->setFixedWidth(240);
    sidebar->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(15, 20, 15, 20);
    layout->setSpacing(6);

    auto* title = new QLabel("БЫСТРЫЙ ДОСТУП", sidebar);
    title->setObjectName("quickAccessTitle");
    layout->addWidget(title);
    layout->addSpacing(8);

    m_recentBtn = new QPushButton(" Недавно просмотренное", sidebar);
    m_recentBtn->setObjectName("quickAccessBtn");
    m_recentBtn->setIcon(ThemeManager::instance().getIcon("recently", QColor("#A8998A"), "AuctionWindow"));
    m_recentBtn->setIconSize(QSize(18, 18));
    m_recentBtn->setCursor(Qt::PointingHandCursor);
    m_recentBtn->setMinimumHeight(38);

    m_favouritesBtn = new QPushButton(QString(" Избранное  %1").arg(m_favouritesCount), sidebar);
    m_favouritesBtn->setObjectName("quickAccessBtn");
    m_favouritesBtn->setIcon(ThemeManager::instance().getIcon("favourite", QColor("#A8998A"), "AuctionWindow"));
    m_favouritesBtn->setIconSize(QSize(18, 18));
    m_favouritesBtn->setCursor(Qt::PointingHandCursor);
    m_favouritesBtn->setMinimumHeight(38);

    layout->addWidget(m_recentBtn);
    layout->addWidget(m_favouritesBtn);
    layout->addStretch();

    return sidebar;
}

// =========================================================
// === ПРАВАЯ ВЕРХНЯЯ ПАНЕЛЬ (ПОИСК + ФИЛЬТРЫ) ===
// =========================================================
QWidget* AuctionWindow::createRightTopBar()
{
    auto* bar = new QWidget(this);
    bar->setObjectName("rightTopBar");
    bar->setFixedHeight(60);
    bar->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(20, 10, 20, 10);
    layout->setSpacing(12);

    // Поиск
    m_searchEdit = new QLineEdit(bar);
    m_searchEdit->setObjectName("searchEdit");
    m_searchEdit->setPlaceholderText("Поиск предмета по имени...");
    m_searchEdit->addAction(ThemeManager::instance().getIcon("search", QColor("#A8998A"), "AuctionWindow"), QLineEdit::LeadingPosition);
    m_searchEdit->setMinimumHeight(40);

    // Кнопка "Ранг"
    m_rankFilterBtn = new QPushButton(" Ранг: Все", bar);
    m_rankFilterBtn->setObjectName("filterBtn");
    m_rankFilterBtn->setIcon(ThemeManager::instance().getIcon("filter", QColor("#A8998A"), "AuctionWindow"));
    m_rankFilterBtn->setIconSize(QSize(18, 18));
    m_rankFilterBtn->setCursor(Qt::PointingHandCursor);
    m_rankFilterBtn->setMinimumHeight(40);
    m_rankFilterBtn->setMinimumWidth(180);
    connect(m_rankFilterBtn, &QPushButton::clicked, this, &AuctionWindow::onRankFilterClicked);

    // Кнопка "Сортировка"
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

// =========================================================
// === ПОПАП: РАНГИ ===
// =========================================================
QWidget* AuctionWindow::createRankPopup()
{
    auto* popup = new QWidget(this, Qt::Popup);
    popup->setObjectName("dropdownPopup");
    popup->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(popup);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(2);

    QStringList ranks = { "Легенда", "Мастер", "Ветеран", "Сталкер", "Новичок", "Отмычка" };

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

    // Разделитель
    auto* sep = new QFrame(popup);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background-color: #3A2F26; margin: 6px 0;");
    layout->addWidget(sep);

    // Кнопка "Сбросить фильтр"
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

// =========================================================
// === ПОПАП: СОРТИРОВКА ===
// =========================================================
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

// =========================================================
// === ОБНОВЛЕНИЕ СТИЛЕЙ ПУНКТОВ ===
// =========================================================
void AuctionWindow::updateRankItemStyle(int index)
{
    if (index < 0 || index >= m_rankItems.size()) return;

    const QStringList rankColors = {
        "#FFC700", "#C84BFF", "#4A9EFF", "#4CD964", "#FFFFFF", "#808080"
    };
    QString color = rankColors[index];
    bool active = m_selectedRanks.contains(index);

    QPushButton* btn = m_rankItems[index];
    if (active) {
        QColor c(color);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background-color: rgba(%1, %2, %3, 0.15);"
            "  color: %4;"
            "  border: 1px solid %4;"
            "  border-left: 3px solid %4;"
            "  border-radius: 6px;"
            "  font-weight: bold;"
            "  text-align: left;"
            "  padding: 0px 10px;"
            "  font-size: 13px;"
            "}"
        ).arg(c.red()).arg(c.green()).arg(c.blue()).arg(color));
    }
    else {
        btn->setStyleSheet(
            "QPushButton {"
            "  background: transparent;"
            "  color: #A8998A;"
            "  border: 1px solid transparent;"
            "  border-left: 3px solid transparent;"
            "  border-radius: 6px;"
            "  text-align: left;"
            "  padding: 0px 10px;"
            "  font-size: 13px;"
            "}"
        );
    }
}

void AuctionWindow::updateSortItemStyle(int index)
{
    if (index < 0 || index >= m_sortItems.size()) return;

    QColor accent = ThemeManager::instance().accentColor();
    bool active = (m_currentSort == index);

    QPushButton* btn = m_sortItems[index];
    if (active) {
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background-color: rgba(%1, %2, %3, 0.15);"
            "  color: %4;"
            "  border: 1px solid %4;"
            "  border-left: 3px solid %4;"
            "  border-radius: 6px;"
            "  font-weight: bold;"
            "  text-align: left;"
            "  padding: 0px 10px;"
            "  font-size: 13px;"
            "}"
        ).arg(accent.red()).arg(accent.green()).arg(accent.blue()).arg(accent.name()));
    }
    else {
        btn->setStyleSheet(
            "QPushButton {"
            "  background: transparent;"
            "  color: #A8998A;"
            "  border: 1px solid transparent;"
            "  border-left: 3px solid transparent;"
            "  border-radius: 6px;"
            "  text-align: left;"
            "  padding: 0px 10px;"
            "  font-size: 13px;"
            "}"
        );
    }
}

// =========================================================
// === ОТКРЫТИЕ / ЗАКРЫТИЕ ПОПАПОВ ===
// =========================================================
void AuctionWindow::onRankFilterClicked()
{
    if (m_rankPopup->isVisible()) {
        m_rankPopup->hide();
        return;
    }

    m_rankPopup->adjustSize();
    QPoint pos = m_rankFilterBtn->mapToGlobal(QPoint(0, m_rankFilterBtn->height() + 4));
    m_rankPopup->move(pos);

    auto* effect = new QGraphicsOpacityEffect(m_rankPopup);
    effect->setOpacity(0.0);
    m_rankPopup->setGraphicsEffect(effect);
    m_rankPopup->show();

    auto* anim = new QPropertyAnimation(effect, "opacity", m_rankPopup);
    anim->setDuration(150);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, m_rankPopup, [this]() {
        if (m_rankPopup) m_rankPopup->setGraphicsEffect(nullptr);
        });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AuctionWindow::onSortFilterClicked()
{
    if (m_sortPopup->isVisible()) {
        m_sortPopup->hide();
        return;
    }

    m_sortPopup->adjustSize();
    QPoint pos = m_sortFilterBtn->mapToGlobal(QPoint(0, m_sortFilterBtn->height() + 4));
    m_sortPopup->move(pos);

    auto* effect = new QGraphicsOpacityEffect(m_sortPopup);
    effect->setOpacity(0.0);
    m_sortPopup->setGraphicsEffect(effect);
    m_sortPopup->show();

    auto* anim = new QPropertyAnimation(effect, "opacity", m_sortPopup);
    anim->setDuration(150);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, m_sortPopup, [this]() {
        if (m_sortPopup) m_sortPopup->setGraphicsEffect(nullptr);
        });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// =========================================================
// === ОБРАБОТКА ВЫБОРА ===
// =========================================================
void AuctionWindow::onRankClicked(int rankIndex)
{
    if (m_selectedRanks.contains(rankIndex)) {
        m_selectedRanks.remove(rankIndex);
    }
    else {
        m_selectedRanks.insert(rankIndex);
    }
    updateRankItemStyle(rankIndex);
    updateRankButtonText();
}

void AuctionWindow::onSortClicked(int sortIndex)
{
    m_currentSort = sortIndex;
    for (int i = 0; i < m_sortItems.size(); ++i) {
        updateSortItemStyle(i);
    }
    updateSortButtonText();
    m_sortPopup->hide();
}

void AuctionWindow::onResetRanksClicked()
{
    m_selectedRanks.clear();
    for (int i = 0; i < m_rankItems.size(); ++i) {
        updateRankItemStyle(i);
    }
    updateRankButtonText();
    m_rankPopup->hide();
}

// =========================================================
// === ОБНОВЛЕНИЕ ТЕКСТА КНОПОК ===
// =========================================================
void AuctionWindow::updateRankButtonText()
{
    if (m_selectedRanks.isEmpty()) {
        m_rankFilterBtn->setText(" Ранг: Все");
    }
    else if (m_selectedRanks.size() == 1) {
        int idx = *m_selectedRanks.begin();
        const QStringList ranks = { "Легенда", "Мастер", "Ветеран", "Сталкер", "Новичок", "Отмычка" };
        m_rankFilterBtn->setText(" Ранг: " + ranks[idx]);
    }
    else {
        m_rankFilterBtn->setText(QString(" Ранг: %1").arg(m_selectedRanks.size()));
    }
}

void AuctionWindow::updateSortButtonText()
{
    const QStringList sorts = { "По рангу", "От А до Я", "От Я до А" };
    m_sortFilterBtn->setText(" Сортировка: " + sorts[m_currentSort]);
}
