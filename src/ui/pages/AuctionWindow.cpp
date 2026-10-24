#include "AuctionWindow.h"
#include "api/StalzoneAPI.h"
#include "core/GameDatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QHeaderView>
#include <QJsonObject>
#include <QDebug>

AuctionWindow::AuctionWindow(StalzoneAPI* api, GameDatabase* db, QWidget* parent)
    : QWidget(parent), m_api(api), m_db(db)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setObjectName("auctionRoot");
    setupUi();
    connect(m_api, &StalzoneAPI::lotsReceived, this, &AuctionWindow::onLotsReceived);
    connect(m_api, &StalzoneAPI::errorOccurred, this, &AuctionWindow::onApiError);
}

void AuctionWindow::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);

    // --- Кнопка Назад ---
    auto* topBarLayout = new QHBoxLayout();
    auto* btnBack = new QPushButton("← На главную", this);
    btnBack->setObjectName("btnBack");
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, this, &AuctionWindow::backRequested);
    topBarLayout->addWidget(btnBack);
    topBarLayout->addStretch();
    layout->addLayout(topBarLayout);

    auto* title = new QLabel("<h2>Аукцион</h2>", this);
    layout->addWidget(title);

    auto* controlsLayout = new QHBoxLayout();
    m_itemSelector = new QComboBox(this);
    m_itemSelector->addItem("Предмет: zyv9 (тестовый)", "zyv9");

    m_refreshBtn = new QPushButton("Обновить", this);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AuctionWindow::onRefreshClicked);

    controlsLayout->addWidget(m_itemSelector);
    controlsLayout->addWidget(m_refreshBtn);
    controlsLayout->addStretch();

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({ "Предмет", "Количество", "Цена выкупа", "Время окончания" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);

    layout->addLayout(controlsLayout);
    layout->addWidget(m_table);
}

void AuctionWindow::onRefreshClicked()
{
    m_refreshBtn->setEnabled(false);
    m_refreshBtn->setText("Загрузка...");
    m_table->setRowCount(0);
    m_api->fetchAuctionLots("ru", m_itemSelector->currentData().toString());
}

void AuctionWindow::onLotsReceived(const QJsonArray& lots)
{
    m_refreshBtn->setEnabled(true);
    m_refreshBtn->setText("Обновить");
    populateTable(lots);
}

void AuctionWindow::onApiError(const QString& error)
{
    m_refreshBtn->setEnabled(true);
    m_refreshBtn->setText("Обновить");
    qDebug() << "AuctionWindow error:" << error;
}

void AuctionWindow::populateTable(const QJsonArray& lots)
{
    m_table->setRowCount(lots.size());
    for (int i = 0; i < lots.size(); ++i) {
        QJsonObject lot = lots[i].toObject();
        QString itemId = lot["itemId"].toString();
        QString itemName = m_db->itemName(itemId);
        int amount = lot["amount"].toInt();
        int price = lot["buyoutPrice"].toInt();
        QString endTime = lot["endTime"].toString();

        m_table->setItem(i, 0, new QTableWidgetItem(itemName));
        m_table->setItem(i, 1, new QTableWidgetItem(QString::number(amount)));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(price) + " руб."));
        QString formattedTime = endTime.replace("T", " ").remove("Z");
        m_table->setItem(i, 3, new QTableWidgetItem(formattedTime));
    }
}
