#include "ItemsWindow.h"
#include "core/GameDatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMap>

ItemsWindow::ItemsWindow(GameDatabase* db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setObjectName("itemsRoot");
    setupUi();

    if (m_db->isLoaded()) {
        onDatabaseLoaded();
    }
    else {
        connect(m_db, &GameDatabase::loaded, this, &ItemsWindow::onDatabaseLoaded);
    }
}

void ItemsWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setAlignment(Qt::AlignTop);

    // --- Кнопка Назад ---
    auto* topBarLayout = new QHBoxLayout();
    auto* btnBack = new QPushButton("← На главную", this);
    btnBack->setObjectName("btnBack");
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, this, &ItemsWindow::backRequested);
    topBarLayout->addWidget(btnBack);
    topBarLayout->addStretch();
    mainLayout->addLayout(topBarLayout);

    auto* title = new QLabel("<h2>База предметов</h2>", this);
    mainLayout->addWidget(title);

    auto* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Поиск:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Введите название или ID...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ItemsWindow::filterItems);
    searchLayout->addWidget(m_searchEdit);
    mainLayout->addLayout(searchLayout);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({ "ID предмета", "Название" });
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 150);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    mainLayout->addWidget(m_table);
}

void ItemsWindow::onDatabaseLoaded()
{
    populateTable();
}

void ItemsWindow::populateTable()
{
    QMap<QString, QString> items = m_db->allItems();
    m_table->setRowCount(items.size());
    int row = 0;
    for (auto it = items.begin(); it != items.end(); ++it) {
        m_table->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_table->setItem(row, 1, new QTableWidgetItem(it.value()));
        row++;
    }
}

void ItemsWindow::filterItems(const QString& text)
{
    if (text.isEmpty()) {
        for (int i = 0; i < m_table->rowCount(); ++i) m_table->setRowHidden(i, false);
        return;
    }
    for (int i = 0; i < m_table->rowCount(); ++i) {
        QTableWidgetItem* idItem = m_table->item(i, 0);
        QTableWidgetItem* nameItem = m_table->item(i, 1);
        if (!idItem || !nameItem) continue;
        bool matches = idItem->text().contains(text, Qt::CaseInsensitive) ||
            nameItem->text().contains(text, Qt::CaseInsensitive);
        m_table->setRowHidden(i, !matches);
    }
}
