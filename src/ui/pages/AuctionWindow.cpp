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
    setupUi();
    connect(m_api, &StalzoneAPI::lotsReceived, this, &AuctionWindow::onLotsReceived);
    connect(m_api, &StalzoneAPI::errorOccurred, this, &AuctionWindow::onApiError);
}

void AuctionWindow::setupUi()
{
    setStyleSheet("background-color: #1e1e1e; color: #cccccc;");
    auto* layout = new QVBoxLayout(this);

    auto* controlsLayout = new QHBoxLayout();

    m_itemSelector = new QComboBox(this);
    m_itemSelector->addItem("Предмет: zyv9 (тестовый)", "zyv9");
    m_itemSelector->setStyleSheet("padding: 5px; background: #2d2d2d; border: 1px solid #444; color: #fff;");

    m_refreshBtn = new QPushButton("Обновить", this);
    m_refreshBtn->setStyleSheet("padding: 5px 15px; background: #2d5a27; color: white; border: none; border-radius: 3px;");
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
    m_table->setStyleSheet(
        "QTableWidget { background-color: #252525; gridline-color: #444; color: #ccc; }"
        "QHeaderView::section { background-color: #1e1e1e; padding: 4px; border: 1px solid #444; color: #fff; }"
        "QTableWidget::item:selected { background-color: #2d5a27; }"
    );

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
        QString itemName = m_db->itemName(itemId); // <--- ПОЛУЧАЕМ НАЗВАНИЕ
        int amount = lot["amount"].toInt();
        int price = lot["buyoutPrice"].toInt();
        QString endTime = lot["endTime"].toString();

        m_table->setItem(i, 0, new QTableWidgetItem(itemName)); // <--- ПИШЕМ НАЗВАНИЕ
        m_table->setItem(i, 1, new QTableWidgetItem(QString::number(amount)));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(price) + " руб."));

        QString formattedTime = endTime.replace("T", " ").remove("Z");
        m_table->setItem(i, 3, new QTableWidgetItem(formattedTime));
    }
}
