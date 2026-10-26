#include "ItemDetailWidget.h"
#include "core/ThemeManager.h"
#include "ui/widgets/FiltersBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QtMath>

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

    // === ФИЛЬТРЫ ===
    auto* filtersContainer = new QWidget(this);
    auto* filtersContainerLayout = new QVBoxLayout(filtersContainer);
    filtersContainerLayout->setContentsMargins(10, 10, 10, 0);
    filtersContainerLayout->setSpacing(0);

    m_filtersBar = new FiltersBar(filtersContainer);
    connect(m_filtersBar, &FiltersBar::raritiesChanged, this, &ItemDetailWidget::raritiesChanged);
    connect(m_filtersBar, &FiltersBar::enchantChanged, this, &ItemDetailWidget::enchantChanged);
    filtersContainerLayout->addWidget(m_filtersBar);
    mainLayout->addWidget(filtersContainer);

    // === ЛУЧШАЯ ЦЕНА ===
    auto* bestPriceContainer = new QWidget(this);
    auto* bestPriceContainerLayout = new QVBoxLayout(bestPriceContainer);
    bestPriceContainerLayout->setContentsMargins(10, 10, 10, 0);
    bestPriceContainerLayout->setSpacing(0);

    auto* bestPricePanel = new QWidget(bestPriceContainer);
    bestPricePanel->setObjectName("bestPricePanel");
    bestPricePanel->setAttribute(Qt::WA_StyledBackground, true);

    auto* panelLayout = new QHBoxLayout(bestPricePanel);
    panelLayout->setContentsMargins(20, 16, 20, 16);
    panelLayout->setSpacing(16);

    m_bestPriceIcon = new QLabel(bestPricePanel);
    m_bestPriceIcon->setFixedSize(56, 56);
    m_bestPriceIcon->setStyleSheet("background: transparent;");
    QIcon bestIcon = ThemeManager::instance().getIcon(
        "bestprice", ThemeManager::instance().accentColor(), "AuctionWindow");
    if (!bestIcon.isNull()) {
        m_bestPriceIcon->setPixmap(bestIcon.pixmap(56, 56));
    }
    panelLayout->addWidget(m_bestPriceIcon, 0, Qt::AlignVCenter);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(4);

    m_bestPriceTitle = new QLabel("Лучшая цена", bestPricePanel);
    m_bestPriceTitle->setObjectName("bestPriceTitle");
    textLayout->addWidget(m_bestPriceTitle);

    auto* priceRow = new QHBoxLayout();
    priceRow->setSpacing(8);

    m_bestPriceValue = new QLabel("—", bestPricePanel);
    m_bestPriceValue->setObjectName("bestPriceValue");
    priceRow->addWidget(m_bestPriceValue, 0, Qt::AlignBaseline);

    m_bestPriceUnit = new QLabel("за единицу", bestPricePanel);
    m_bestPriceUnit->setObjectName("bestPriceUnit");
    m_bestPriceUnit->hide();
    priceRow->addWidget(m_bestPriceUnit, 0, Qt::AlignBaseline);

    m_bestPriceBadge = new QLabel(bestPricePanel);
    m_bestPriceBadge->setObjectName("bestPriceBadge");
    m_bestPriceBadge->setAlignment(Qt::AlignCenter);
    m_bestPriceBadge->hide();
    priceRow->addWidget(m_bestPriceBadge, 0, Qt::AlignVCenter);

    priceRow->addStretch();
    textLayout->addLayout(priceRow);

    panelLayout->addLayout(textLayout);

    // === Разделитель ===
    auto* sep = new QFrame(bestPricePanel);
    sep->setFrameShape(QFrame::VLine);
    sep->setFixedWidth(1);
    sep->setStyleSheet("background-color: #3A2F26; border: none;");
    panelLayout->addWidget(sep);

    // === Колонка: ЛОТ ЦЕЛИКОМ ===
    auto* lotPriceColWidget = new QWidget(bestPricePanel);
    lotPriceColWidget->setStyleSheet("background: transparent;");
    auto* lotPriceCol = new QVBoxLayout(lotPriceColWidget);
    lotPriceCol->setContentsMargins(0, 0, 0, 0);
    lotPriceCol->setSpacing(10);
    lotPriceCol->setAlignment(Qt::AlignCenter);
    auto* lotPriceTitle = new QLabel("ЛОТ ЦЕЛИКОМ", lotPriceColWidget);
    lotPriceTitle->setObjectName("bestPriceColTitle");
    m_lotPriceValue = new QLabel("—", lotPriceColWidget);
    m_lotPriceValue->setObjectName("bestPriceColValue");
    lotPriceCol->addWidget(lotPriceTitle, 0, Qt::AlignHCenter);
    lotPriceCol->addWidget(m_lotPriceValue, 0, Qt::AlignHCenter);
    panelLayout->addWidget(lotPriceColWidget, 0, Qt::AlignVCenter);

    // === Колонка: ШТУК В ЛОТЕ ===
    auto* amountColWidget = new QWidget(bestPricePanel);
    amountColWidget->setStyleSheet("background: transparent;");
    auto* amountCol = new QVBoxLayout(amountColWidget);
    amountCol->setContentsMargins(0, 0, 0, 0);
    amountCol->setSpacing(10);
    amountCol->setAlignment(Qt::AlignCenter);
    auto* amountTitle = new QLabel("ШТУК В ЛОТЕ", amountColWidget);
    amountTitle->setObjectName("bestPriceColTitle");
    m_lotAmountValue = new QLabel("—", amountColWidget);
    m_lotAmountValue->setObjectName("bestPriceColValue");
    amountCol->addWidget(amountTitle, 0, Qt::AlignHCenter);
    amountCol->addWidget(m_lotAmountValue, 0, Qt::AlignHCenter);
    panelLayout->addWidget(amountColWidget, 0, Qt::AlignVCenter);

    // === Колонка: ОСТАЛОСЬ ===
    auto* timeColWidget = new QWidget(bestPricePanel);
    timeColWidget->setStyleSheet("background: transparent;");
    auto* timeCol = new QVBoxLayout(timeColWidget);
    timeCol->setContentsMargins(0, 0, 0, 0);
    timeCol->setSpacing(10);
    timeCol->setAlignment(Qt::AlignCenter);
    auto* timeTitle = new QLabel("ОСТАЛОСЬ", timeColWidget);
    timeTitle->setObjectName("bestPriceColTitle");
    m_lotTimeValue = new QLabel("—", timeColWidget);
    m_lotTimeValue->setObjectName("bestPriceColValueGreen");
    timeCol->addWidget(timeTitle, 0, Qt::AlignHCenter);
    timeCol->addWidget(m_lotTimeValue, 0, Qt::AlignHCenter);
    panelLayout->addWidget(timeColWidget, 0, Qt::AlignVCenter);

    panelLayout->addStretch();

    m_loadedLotsLabel = new QLabel("Загружено 0 лотов", bestPricePanel);
    m_loadedLotsLabel->setObjectName("bestPriceLoadedLots");
    m_loadedLotsLabel->setStyleSheet("background: transparent;");
    panelLayout->addWidget(m_loadedLotsLabel, 0, Qt::AlignVCenter);

    bestPriceContainerLayout->addWidget(bestPricePanel);
    mainLayout->addWidget(bestPriceContainer);

    // === ЦЕНТР: ИКОНКА + НАЗВАНИЕ ===
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

    // === ТАЙМЕР ===
    m_timeUpdateTimer = new QTimer(this);
    m_timeUpdateTimer->setInterval(1000);
    connect(m_timeUpdateTimer, &QTimer::timeout, this, &ItemDetailWidget::updateTimeLeft);
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

void ItemDetailWidget::setBestPrice(int bestPrice, int avgPrice)
{
    if (!m_bestPriceValue) return;
    if (bestPrice <= 0) {
        m_bestPriceValue->setText("—");
        m_bestPriceUnit->hide();
        m_bestPriceBadge->hide();
        return;
    }
    m_bestPriceValue->setText(formatPrice(bestPrice) + " ₽");
    m_bestPriceUnit->show();
    updateBestPriceBadge(bestPrice, avgPrice);
}

void ItemDetailWidget::setBestPriceLoading()
{
    if (m_bestPriceValue) m_bestPriceValue->setText("Загрузка...");
    if (m_bestPriceUnit)  m_bestPriceUnit->hide();
    if (m_bestPriceBadge) m_bestPriceBadge->hide();
}

void ItemDetailWidget::setBestPriceEmpty()
{
    if (m_bestPriceValue) m_bestPriceValue->setText("—");
    if (m_bestPriceUnit)  m_bestPriceUnit->hide();
    if (m_bestPriceBadge) m_bestPriceBadge->hide();
}

void ItemDetailWidget::setBestLot(int lotPrice, int amount, const QDateTime& endTime, int totalLoadedLots)
{
    if (!m_lotPriceValue || !m_lotAmountValue || !m_lotTimeValue || !m_loadedLotsLabel) return;

    if (lotPrice <= 0) {
        m_lotPriceValue->setText("—");
        m_lotAmountValue->setText("—");
        m_lotTimeValue->setText("—");
        if (m_timeUpdateTimer) m_timeUpdateTimer->stop();
    }
    else {
        m_lotPriceValue->setText(formatPrice(lotPrice) + " ₽");
        m_lotAmountValue->setText(QString::number(amount));

        m_bestLotEndTime = endTime;
        updateTimeLeft();
        if (m_timeUpdateTimer) m_timeUpdateTimer->start();
    }
    m_loadedLotsLabel->setText(QString("Загружено %1 лотов").arg(totalLoadedLots));
}

void ItemDetailWidget::updateTimeLeft()
{
    if (!m_bestLotEndTime.isValid() || !m_lotTimeValue) return;

    QDateTime now = QDateTime::currentDateTimeUtc();
    qint64 secsLeft = now.secsTo(m_bestLotEndTime);

    if (secsLeft > 0) {
        int h = static_cast<int>(secsLeft / 3600);
        int m = static_cast<int>((secsLeft % 3600) / 60);
        int s = static_cast<int>(secsLeft % 60);
        m_lotTimeValue->setText(QString("%1:%2:%3")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0')));
    }
    else {
        m_lotTimeValue->setText("00:00:00");
        if (m_timeUpdateTimer) m_timeUpdateTimer->stop();
    }
}

void ItemDetailWidget::updateBestPriceBadge(int bestPrice, int avgPrice)
{
    if (!m_bestPriceBadge) return;

    bool isBelowAverage = false;
    int percent = 0;

    if (avgPrice > 0 && bestPrice < avgPrice) {
        percent = qRound((1.0 - static_cast<double>(bestPrice) / avgPrice) * 100.0);
        if (percent >= 1) isBelowAverage = true;
    }

    if (isBelowAverage) {
        m_bestPriceBadge->setText(QString("на %1% ниже аукциона").arg(percent));
        m_bestPriceBadge->setStyleSheet(
            "background: transparent;"
            "color: #7def7d;"
            "border: 1px solid #7def7d;"
            "border-radius: 6px;"
            "padding: 4px 10px;"
            "font-size: 12px;"
            "font-weight: 600;"
        );
    }
    else {
        m_bestPriceBadge->setText("средняя по аукциону");
        m_bestPriceBadge->setStyleSheet(
            "background: transparent;"
            "color: #A8998A;"
            "border: 1px solid #3A2F26;"
            "border-radius: 6px;"
            "padding: 4px 10px;"
            "font-size: 12px;"
            "font-weight: 600;"
        );
    }
    m_bestPriceBadge->show();
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

QString ItemDetailWidget::formatPrice(int value) const
{
    QString s = QString::number(value);
    if (value < 0) s = s.mid(1);

    QString result;
    int count = 0;
    for (int i = s.length() - 1; i >= 0; --i) {
        result.prepend(s[i]);
        ++count;
        if (count % 3 == 0 && i > 0) {
            result.prepend(' ');
        }
    }
    return (value < 0 ? "-" : "") + result;
}
