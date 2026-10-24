#include "BarterWindow.h"
#include "core/GameDatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

BarterWindow::BarterWindow(GameDatabase* db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
    setupUi();

    // Если база уже загружена (например, окно открыли позже) - заполняем сразу
    if (m_db->isLoaded()) {
        onDatabaseLoaded();
    }
    else {
        // Иначе ждем сигнала
        connect(m_db, &GameDatabase::loaded, this, &BarterWindow::onDatabaseLoaded);
    }
}

void BarterWindow::setupUi()
{
    setStyleSheet("background-color: #1e1e1e; color: #cccccc;");
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);

    auto* title = new QLabel("<h2>Калькулятор бартера (по рецептам)</h2>", this);
    mainLayout->addWidget(title);

    m_statusLabel = new QLabel("Загрузка базы данных...", this);
    m_statusLabel->setStyleSheet("color: #ffff55;");
    mainLayout->addWidget(m_statusLabel);

    auto* controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel("Что хотим получить:", this));

    m_recipeSelector = new QComboBox(this);
    m_recipeSelector->setStyleSheet("padding: 5px; background: #2d2d2d; border: 1px solid #444; color: white; min-width: 200px;");
    m_recipeSelector->setEnabled(false); // Заблокирован до загрузки
    connect(m_recipeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarterWindow::onRecipeSelected);

    controlsLayout->addWidget(m_recipeSelector);
    controlsLayout->addStretch();
    mainLayout->addLayout(controlsLayout);

    // Таблица того, что нужно для обмена
    m_requirementsTable = new QTableWidget(this);
    m_requirementsTable->setColumnCount(2);
    m_requirementsTable->setHorizontalHeaderLabels({ "Необходимый предмет", "Количество" });
    m_requirementsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_requirementsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_requirementsTable->setStyleSheet(
        "QTableWidget { background-color: #252525; gridline-color: #444; color: #ccc; margin-top: 10px; }"
        "QHeaderView::section { background-color: #1e1e1e; padding: 4px; border: 1px solid #444; color: #fff; }"
    );
    mainLayout->addWidget(m_requirementsTable);
}

void BarterWindow::onDatabaseLoaded()
{
    m_statusLabel->setText("База данных загружена. Выберите предмет.");
    m_statusLabel->setStyleSheet("color: #55ff55;");

    QJsonArray settlements = m_db->barterRecipes();

    // Собираем все рецепты из всех поселений (баров) в один список
    for (const QJsonValue& settlementVal : settlements) {
        QJsonObject settlement = settlementVal.toObject();
        QJsonArray recipes = settlement["recipes"].toArray();

        for (const QJsonValue& recipeVal : recipes) {
            QJsonObject recipe = recipeVal.toObject();
            QString resultItemId = recipe["item"].toString();
            QString displayName = m_db->itemName(resultItemId);

            // Добавляем в список. В userData (второй параметр) сохраняем сам JSON рецепта,
            // чтобы потом легко достать его требования.
            m_recipeSelector->addItem(displayName, recipeVal.toVariant());
        }
    }

    m_recipeSelector->setEnabled(true);
    if (m_recipeSelector->count() > 0) {
        onRecipeSelected(0); // Автоматически показываем первый рецепт
    }
}

void BarterWindow::onRecipeSelected(int index)
{
    if (index < 0) return;

    m_requirementsTable->setRowCount(0); // Очищаем таблицу

    // Достаем сохраненный JSON рецепта
    QVariant data = m_recipeSelector->itemData(index);
    QJsonObject recipe = QJsonObject::fromVariantMap(data.toMap());

    // В barter_recipes.json у рецепта есть массив "offers" (предложения).
    // Обычно там одно предложение с массивом "requiredItems".
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
