#include "ItemCard.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QPointer>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QDebug>

// =========================================================
// === КАСТОМНАЯ ОБВОДКА ===
// =========================================================
class BorderOverlay : public QWidget {
public:
    BorderOverlay(QWidget* parent, const QColor& color) : QWidget(parent), m_color(color) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QLinearGradient grad(0, 0, 0, height());
        grad.setColorAt(0.0, QColor("#3A2F26"));
        grad.setColorAt(1.0, m_color);

        QPainterPath outer;
        outer.addRoundedRect(rect(), 8, 8);

        QPainterPath inner;
        inner.addRoundedRect(rect().adjusted(4, 1, -1, -4), 8, 8);

        QPainterPath border = outer.subtracted(inner);

        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawPath(border);
    }
private:
    QColor m_color;
};

// =========================================================
// === ITEM CARD ===
// =========================================================

ItemCard::ItemCard(const ItemData& item, QNetworkAccessManager* netManager,
    bool isFavorite, QWidget* parent)
    : QWidget(parent)
    , m_item(item)
    , m_netManager(netManager)
    , m_isFavorite(isFavorite)
{
    setFixedSize(170, 210);
    setObjectName("itemCard");
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);

    setupUi();

    if (m_netManager && !m_item.iconPath.isEmpty()) {
        QString url = "https://github.com/EXBO-Studio/stalzone-database/raw/main/ru" + m_item.iconPath;
        QNetworkRequest request{ QUrl(url) };
        m_reply = m_netManager->get(request);

        QPointer<ItemCard> self(this);
        connect(m_reply, &QNetworkReply::finished, this, [this, self]() {
            if (!self || !m_reply) return;

            if (m_reply->error() == QNetworkReply::NoError) {
                QByteArray data = m_reply->readAll();
                if (!data.isEmpty()) {
                    QPixmap pixmap;
                    if (pixmap.loadFromData(data)) {
                        setIcon(pixmap);
                    }
                }
            }
            m_reply->deleteLater();
            m_reply = nullptr;
            });
    }
}

ItemCard::~ItemCard()
{
    if (m_reply) {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

void ItemCard::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // === ИКОНКА ===
    m_iconContainer = new QWidget(this);
    m_iconContainer->setFixedSize(170, 150);
    m_iconContainer->setObjectName("itemCardIconContainer");
    m_iconContainer->setAttribute(Qt::WA_StyledBackground, true);
    m_iconContainer->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    auto* iconLayout = new QVBoxLayout(m_iconContainer);
    iconLayout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel(m_iconContainer);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    iconLayout->addWidget(m_iconLabel);

    layout->addWidget(m_iconContainer);

    // === НАЗВАНИЕ ===
    m_nameContainer = new QWidget(this);
    m_nameContainer->setFixedSize(170, 60);
    m_nameContainer->setObjectName("itemCardNameContainer");
    m_nameContainer->setAttribute(Qt::WA_StyledBackground, true);
    m_nameContainer->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    auto* nameLayout = new QHBoxLayout(m_nameContainer);
    nameLayout->setContentsMargins(5, 0, 5, 0);
    nameLayout->setAlignment(Qt::AlignCenter);

    m_nameLabel = new QLabel(m_item.name, m_nameContainer);
    m_nameLabel->setObjectName("itemNameLabel");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QColor color = colorForRank(m_item.color);
    m_nameLabel->setStyleSheet(QString("color: %1;").arg(color.name()));

    nameLayout->addWidget(m_nameLabel);
    layout->addWidget(m_nameContainer);

    // === ОБВОДКА ===
    m_overlay = new BorderOverlay(this, colorForRank(m_item.color));
    m_overlay->setGeometry(rect());
    m_overlay->raise();

    // === КНОПКА ИЗБРАННОГО ===
    m_favoriteBtn = new QPushButton(this);
    m_favoriteBtn->setObjectName("cardFavoriteBtn");
    m_favoriteBtn->setFixedSize(28, 28);
    m_favoriteBtn->setCursor(Qt::PointingHandCursor);
    m_favoriteBtn->setFlat(true);
    m_favoriteBtn->setFocusPolicy(Qt::NoFocus);
    m_favoriteBtn->move(width() - m_favoriteBtn->width() - 6, 6);

    updateFavoriteButton();
    connect(m_favoriteBtn, &QPushButton::clicked, this, &ItemCard::onFavoriteClicked);
    m_favoriteBtn->raise();
}

void ItemCard::updateFavoriteButton()
{
    QColor color = m_isFavorite
        ? ThemeManager::instance().accentColor()
        : QColor("#9CA3AF");
    m_favoriteBtn->setIcon(
        ThemeManager::instance().getIcon("favourite", color, "AuctionWindow"));
    m_favoriteBtn->setIconSize(QSize(18, 18));
}

void ItemCard::onFavoriteClicked()
{
    m_isFavorite = !m_isFavorite;
    updateFavoriteButton();
    emit favoriteToggled(m_item.id, m_isFavorite);
}

void ItemCard::setFavorite(bool isFavorite)
{
    if (m_isFavorite == isFavorite) return;
    m_isFavorite = isFavorite;
    updateFavoriteButton();
}

void ItemCard::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    setMask(QRegion(path.toFillPolygon().toPolygon()));

    if (m_overlay) m_overlay->setGeometry(rect());
    if (m_favoriteBtn) {
        m_favoriteBtn->move(width() - m_favoriteBtn->width() - 6, 6);
    }
}

void ItemCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
    }
    QWidget::mousePressEvent(event);
}

void ItemCard::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        if (rect().contains(event->pos())) {
            emit cardClicked(m_item.id);
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void ItemCard::enterEvent(QEnterEvent* event)
{
    // Подсвечиваем ОБЕ части карточки на тон светлее
    if (m_iconContainer) {
        m_iconContainer->setStyleSheet(
            "background-color: #252119;" // Чуть светлее #1F1A16
            "border-top-left-radius: 8px;"
            "border-top-right-radius: 8px;"
            "border: none;"
        );
    }
    if (m_nameContainer) {
        m_nameContainer->setStyleSheet(
            "background-color: #1F1A16;" // Чуть светлее #181411
            "border-bottom-left-radius: 8px;"
            "border-bottom-right-radius: 8px;"
            "border: none;"
        );
    }
    QWidget::enterEvent(event);
}

void ItemCard::leaveEvent(QEvent* event)
{
    // Сбрасываем стили — вернутся цвета по умолчанию из QSS
    if (m_iconContainer) m_iconContainer->setStyleSheet("");
    if (m_nameContainer) m_nameContainer->setStyleSheet("");
    QWidget::leaveEvent(event);
}

void ItemCard::setIcon(const QPixmap& pixmap)
{
    if (m_iconLabel) {
        m_iconLabel->setPixmap(
            pixmap.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

QColor ItemCard::colorForRank(const QString& colorName) const
{
    if (colorName == "RANK_LEGEND") return QColor("#ffdd66");
    if (colorName == "RANK_MASTER") return QColor("#ff5767");
    if (colorName == "RANK_VETERAN") return QColor("#d968c4");
    if (colorName == "RANK_STALKER") return QColor("#8d8dff");
    if (colorName == "RANK_NEWBIE") return QColor("#7def7d");
    if (colorName == "RANK_ROOKIE") return QColor("#eeeeee");
    return QColor("#A8998A");
}
