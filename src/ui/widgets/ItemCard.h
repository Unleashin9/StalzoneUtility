#pragma once
#include <QWidget>
#include <QColor>
#include "core/GameDatabase.h"

class QLabel;
class QPushButton;
class QNetworkAccessManager;
class QNetworkReply;
class QResizeEvent;
class QMouseEvent;
class QEnterEvent;
class QEvent;

class ItemCard : public QWidget
{
    Q_OBJECT
public:
    explicit ItemCard(const ItemData& item, QNetworkAccessManager* netManager,
        bool isFavorite, QWidget* parent = nullptr);
    ~ItemCard() override;

    void setIcon(const QPixmap& pixmap);
    void setFavorite(bool isFavorite);
    bool isFavorite() const { return m_isFavorite; }
    QString itemId() const { return m_item.id; }

signals:
    void favoriteToggled(const QString& itemId, bool isFavorite);
    void cardClicked(const ItemData& item); // <--- ИЗМЕНЕНО

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onFavoriteClicked();

private:
    void setupUi();
    void updateFavoriteButton();
    QColor colorForRank(const QString& colorName) const;

    ItemData m_item;
    QNetworkAccessManager* m_netManager;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QWidget* m_iconContainer = nullptr;
    QWidget* m_nameContainer = nullptr;
    QWidget* m_overlay = nullptr;
    QPushButton* m_favoriteBtn = nullptr;
    QNetworkReply* m_reply = nullptr;
    bool m_isFavorite = false;
    bool m_pressed = false;
};
