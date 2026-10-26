#include "ItemDetailWidget.h"
#include "core/ThemeManager.h"
#include "ui/widgets/FiltersBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>

ItemDetailWidget::ItemDetailWidget(QWidget* parent) : QWidget(parent)
{
    setObjectName("itemDetailRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
}

void ItemDetailWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === КОНТЕЙНЕР: ФИЛЬТРЫ ===
    auto* filtersContainer = new QWidget(this);
    auto* filtersContainerLayout = new QVBoxLayout(filtersContainer);
    filtersContainerLayout->setContentsMargins(10, 10, 10, 0);
    filtersContainerLayout->setSpacing(0);

    m_filtersBar = new FiltersBar(filtersContainer);
    connect(m_filtersBar, &FiltersBar::raritiesChanged, this, &ItemDetailWidget::raritiesChanged);
    connect(m_filtersBar, &FiltersBar::enchantChanged, this, &ItemDetailWidget::enchantChanged);
    filtersContainerLayout->addWidget(m_filtersBar);

    mainLayout->addWidget(filtersContainer);

    // === КОНТЕЙНЕР: ЛУЧШАЯ ЦЕНА ===
    auto* bestPriceContainer = new QWidget(this);
    auto* bestPriceContainerLayout = new QVBoxLayout(bestPriceContainer);
    bestPriceContainerLayout->setContentsMargins(10, 10, 10, 0);
    bestPriceContainerLayout->setSpacing(0);

    auto* bestPricePanel = new QWidget(bestPriceContainer);
    bestPricePanel->setObjectName("bestPricePanel");
    bestPricePanel->setAttribute(Qt::WA_StyledBackground, true);

    auto* panelLayout = new QHBoxLayout(bestPricePanel);
    panelLayout->setContentsMargins(15, 10, 15, 10);
    panelLayout->setSpacing(12);

    // Иконка
    m_bestPriceIcon = new QLabel(bestPricePanel);
    m_bestPriceIcon->setFixedSize(28, 28);
    m_bestPriceIcon->setStyleSheet("background: transparent;");
    QIcon bestIcon = ThemeManager::instance().getIcon(
        "bestprice", ThemeManager::instance().accentColor(), "AuctionWindow");
    if (!bestIcon.isNull()) {
        m_bestPriceIcon->setPixmap(bestIcon.pixmap(28, 28));
    }
    panelLayout->addWidget(m_bestPriceIcon, 0, Qt::AlignVCenter);

    // Текст: заголовок + значение
    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);

    m_bestPriceTitle = new QLabel("Лучшая цена", bestPricePanel);
    m_bestPriceTitle->setObjectName("bestPriceTitle");
    textLayout->addWidget(m_bestPriceTitle);

    m_bestPriceValue = new QLabel("—", bestPricePanel);
    m_bestPriceValue->setObjectName("bestPriceValue");
    textLayout->addWidget(m_bestPriceValue);

    panelLayout->addLayout(textLayout);
    panelLayout->addStretch();

    bestPriceContainerLayout->addWidget(bestPricePanel);
    mainLayout->addWidget(bestPriceContainer);

    // === ЦЕНТРАЛЬНЫЙ БЛОК: ИКОНКА + НАЗВАНИЕ ===
    auto* contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 20, 30, 20);
    contentLayout->setSpacing(20);
    contentLayout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel(contentWidget);
    m_iconLabel->setFixedSize(200, 200);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("background-color: #1F1A16; border-radius: 12px; border: 1px solid #3A2F26;");
    contentLayout->addStretch();
    contentLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);

    m_nameLabel = new QLabel(contentWidget);
    m_nameLabel->setObjectName("itemDetailName");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    contentLayout->addWidget(m_nameLabel, 0, Qt::AlignCenter);
    contentLayout->addStretch();

    mainLayout->addWidget(contentWidget, 1);
}

void ItemDetailWidget::setItem(const ItemData& item)
{
    m_nameLabel->setText(item.name);
    m_nameLabel->setStyleSheet(QString("color: %1;").arg(colorForRank(item.color).name()));

    m_iconLabel->clear();
    if (!item.iconPath.isEmpty()) {
        auto* netManager = new QNetworkAccessManager(this);
        QString url = "https://github.com/EXBO-Studio/stalzone-database/raw/main/ru" + item.iconPath;
        QNetworkRequest request{ QUrl(url) };
        auto* reply = netManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply, netManager]() {
            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pixmap;
                if (pixmap.loadFromData(reply->readAll())) {
                    m_iconLabel->setPixmap(pixmap.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
            reply->deleteLater();
            netManager->deleteLater();
            });
    }
}

void ItemDetailWidget::setBestPrice(int pricePerItem)
{
    if (!m_bestPriceValue) return;
    if (pricePerItem <= 0) {
        m_bestPriceValue->setText("—");
        return;
    }
    m_bestPriceValue->setText(QString::number(pricePerItem) + " ₽");
}

void ItemDetailWidget::setBestPriceLoading()
{
    if (m_bestPriceValue) m_bestPriceValue->setText("...");
}

void ItemDetailWidget::setBestPriceEmpty()
{
    if (m_bestPriceValue) m_bestPriceValue->setText("—");
}

QColor ItemDetailWidget::colorForRank(const QString& colorName) const
{
    if (colorName == "RANK_LEGEND") return QColor("#ffdd66");
    if (colorName == "RANK_MASTER") return QColor("#ff5767");
    if (colorName == "RANK_VETERAN") return QColor("#d968c4");
    if (colorName == "RANK_STALKER") return QColor("#8d8dff");
    if (colorName == "RANK_NEWBIE") return QColor("#7def7d");
    if (colorName == "RANK_ROOKIE") return QColor("#eeeeee");
    return QColor("#A8998A");
}
