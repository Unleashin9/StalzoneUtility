#pragma once
#include <QWidget>
#include "core/GameDatabase.h"

class QLabel;
class QNetworkAccessManager;

class SidebarItemInfo : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarItemInfo(QWidget* parent = nullptr);
    void setItem(const ItemData& item);

private:
    void setupUi();
    QColor colorForRank(const QString& colorName) const;
    QString rankNameForColor(const QString& colorName) const;
    QString categoryName(const QString& cat) const;

    QWidget* m_iconFrame = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_infoLabel = nullptr;
    QNetworkAccessManager* m_netManager = nullptr;
};
