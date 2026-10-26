#include "SidebarItemInfo.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>

SidebarItemInfo::SidebarItemInfo(QWidget* parent) : QWidget(parent)
{
    setObjectName("sidebarItemInfo");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(210, 150);
    m_netManager = new QNetworkAccessManager(this);
    setupUi();
}

void SidebarItemInfo::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignCenter);

    // Иконка с рамкой в цвет ранга
    m_iconFrame = new QWidget(this);
    m_iconFrame->setObjectName("itemIconFrame");
    m_iconFrame->setFixedSize(80, 80);
    m_iconFrame->setAttribute(Qt::WA_StyledBackground, true);
    m_iconFrame->setStyleSheet(
        "QWidget#itemIconFrame {"
        "  background-color: #181411;"
        "  border: 2px solid #3A2F26;"
        "  border-radius: 12px;"
        "}");

    auto* iconLayout = new QVBoxLayout(m_iconFrame);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel(m_iconFrame);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    iconLayout->addWidget(m_iconLabel);

    layout->addWidget(m_iconFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(4);

    // Название
    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("sidebarItemName");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(m_nameLabel);

    // Ранг · Категория
    m_infoLabel = new QLabel(this);
    m_infoLabel->setObjectName("sidebarItemInfoLine");
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(m_infoLabel);

    layout->addStretch();
}

void SidebarItemInfo::setItem(const ItemData& item)
{
    QColor color = colorForRank(item.color);

    // Перекрашиваем ТОЛЬКО внутреннюю рамку иконки
    m_iconFrame->setStyleSheet(QString(
        "QWidget#itemIconFrame {"
        "  background-color: #181411;"
        "  border: 2px solid %1;"
        "  border-radius: 12px;"
        "}").arg(color.name()));

    m_nameLabel->setText(item.name);

    QString rankName = rankNameForColor(item.color);
    QString catName = categoryName(item.category);

    // Разделитель |, и весь текст красим в цвет ранга
    m_infoLabel->setText(rankName + "  |  " + catName);
    m_infoLabel->setStyleSheet(QString("color: %1;").arg(color.name()));

    m_iconLabel->clear();
    if (!item.iconPath.isEmpty()) {
        QString url = "https://github.com/EXBO-Studio/stalzone-database/raw/main/ru" + item.iconPath;
        QNetworkRequest request{ QUrl(url) };
        auto* reply = m_netManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pixmap;
                if (pixmap.loadFromData(reply->readAll())) {
                    m_iconLabel->setPixmap(pixmap.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
            reply->deleteLater();
            });
    }
}

QColor SidebarItemInfo::colorForRank(const QString& colorName) const
{
    if (colorName == "RANK_LEGEND")  return QColor("#ffdd66");
    if (colorName == "RANK_MASTER")  return QColor("#ff5767");
    if (colorName == "RANK_VETERAN") return QColor("#d968c4");
    if (colorName == "RANK_STALKER") return QColor("#8d8dff");
    if (colorName == "RANK_NEWBIE")  return QColor("#7def7d");
    if (colorName == "RANK_ROOKIE")  return QColor("#eeeeee");
    return QColor("#A8998A");
}

QString SidebarItemInfo::rankNameForColor(const QString& colorName) const
{
    if (colorName == "RANK_LEGEND")  return "ЛЕГЕНДА";
    if (colorName == "RANK_MASTER")  return "МАСТЕР";
    if (colorName == "RANK_VETERAN") return "ВЕТЕРАН";
    if (colorName == "RANK_STALKER") return "СТАЛКЕР";
    if (colorName == "RANK_NEWBIE")  return "НОВИЧОК";
    if (colorName == "RANK_ROOKIE")  return "ОБЫЧНЫЙ";
    return "ОБЫЧНЫЙ";
}

QString SidebarItemInfo::categoryName(const QString& cat) const
{
    if (cat == "weapon")          return "ОРУЖИЕ";
    if (cat == "armor")           return "КОСТЮМЫ";
    if (cat == "containers")      return "КОНТЕЙНЕРЫ";
    if (cat == "backpacks")       return "РЮКЗАКИ";
    if (cat == "artefact")        return "АРТЕФАКТЫ";
    if (cat == "attachment")      return "ОБВЕСЫ";
    if (cat == "weapon_modules")  return "МОДУЛИ";
    if (cat == "bullet")          return "ПАТРОНЫ";
    if (cat == "grenade")         return "ГРАНАТЫ";
    if (cat == "medicine")        return "МЕДИКАМЕНТЫ";
    if (cat == "supply")          return "РАСХОДНИКИ";
    if (cat == "misc")            return "РАЗНОЕ";
    if (cat == "device")          return "УСТРОЙСТВА";
    if (cat == "other")           return "ПРОЧЕЕ";
    return cat.toUpper();
}
