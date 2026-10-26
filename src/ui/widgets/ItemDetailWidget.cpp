#include "ItemDetailWidget.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
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
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    m_backBtn = new QPushButton("← Назад", this);
    m_backBtn->setObjectName("btnBack");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    connect(m_backBtn, &QPushButton::clicked, this, &ItemDetailWidget::backRequested);
    mainLayout->addWidget(m_backBtn, 0, Qt::AlignLeft);

    mainLayout->addStretch();

    auto* centerLayout = new QVBoxLayout();
    centerLayout->setSpacing(20);
    centerLayout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(200, 200);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("background-color: #1F1A16; border-radius: 12px; border: 1px solid #3A2F26;");
    centerLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("itemDetailName");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    centerLayout->addWidget(m_nameLabel, 0, Qt::AlignCenter);

    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
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
