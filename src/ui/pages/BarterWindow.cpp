#include "BarterWindow.h"
#include "core/GameDatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>

BarterWindow::BarterWindow(GameDatabase* db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setObjectName("barterRoot");
    setupUi();

    if (m_db->isLoaded()) {
        onDatabaseLoaded();
    }
    else {
        connect(m_db, &GameDatabase::loaded, this, &BarterWindow::onDatabaseLoaded);
    }
}

void BarterWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setAlignment(Qt::AlignTop);

    // --- Кнопка Назад ---
    auto* topBarLayout = new QHBoxLayout();
    auto* btnBack = new QPushButton("← На главную", this);
    btnBack->setObjectName("btnBack");
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, this, &BarterWindow::backRequested);
    topBarLayout->addWidget(btnBack);
    topBarLayout->addStretch();
    mainLayout->addLayout(topBarLayout);

    auto* title = new QLabel("<h2>Калькулятор бартера (по рецептам)</h2>", this);
    mainLayout->addWidget(title);

    m_statusLabel = new QLabel("Загрузка базы данных...", this);
    mainLayout->addWidget(m_statusLabel);

    auto* controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel("Что хотим получить:", this));
    m_recipeSelector = new QComboBox(this);
    m_recipeSelector->setEnabled(false);
    connect(m_recipeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarterWindow::onRecipeSelected);
    controlsLayout->addWidget(m_recipeSelector);
    controlsLayout->addStretch();
    mainLayout->addLayout(controlsLayout);

    m_requirementsTable = new QTableWidget(this);
    m_requirementsTable->setColumnCount(2);
    m_requirementsTable->setHorizontalHeaderLabels({ "Необходимый предмет", "Количество" });
    m_requirementsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_requirementsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_requirementsTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_requirementsTable);
}

void BarterWindow::onDatabaseLoaded()
{
    m_statusLabel->setText("База данных загружена. Выберите предмет.");
    QJsonArray settlements = m_db->barterRecipes();

    for (const QJsonValue& settlementVal : settlements) {
        QJsonObject settlement = settlementVal.toObject();
        QJsonArray recipes = settlement["recipes"].toArray();
        for (const QJsonValue& recipeVal : recipes) {
            QJsonObject recipe = recipeVal.toObject();
            QString resultItemId = recipe["item"].toString();
            QString displayName = m_db->itemName(resultItemId);
            m_recipeSelector->addItem(displayName, recipeVal.toVariant());
        }
    }
    m_recipeSelector->setEnabled(true);
    if (m_recipeSelector->count() > 0) {
        onRecipeSelected(0);
    }
}

void BarterWindow::onRecipeSelected(int index)
{
    if (index < 0) return;
    m_requirementsTable->setRowCount(0);

    QVariant data = m_recipeSelector->itemData(index);
    QJsonObject recipe = QJsonObject::fromVariantMap(data.toMap());
    QJsonArray offers = recipe["offers"].toArray();
    if (offers.isEmpty()) return;

    QJsonObject firstOffer = offers[0].toObject();
    QJsonArray requiredItems = firstOffer["requiredItems"].toArray();
    m_requirementsTable->setRowCount(requiredItems.size());

    for (int i = 0; i < requiredItems.size(); ++i) {
        QJsonObject req = requiredItems[i].toObject();
        QString reqId = req["item"].toString();
        int amount = req["amount"].toInt();
        QString reqName = m_db->itemName(reqId);
        m_requirementsTable->setItem(i, 0, new QTableWidgetItem(reqName));
        m_requirementsTable->setItem(i, 1, new QTableWidgetItem(QString::number(amount)));
    }
}
