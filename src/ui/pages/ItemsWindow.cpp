#include "ItemsWindow.h"
#include "core/GameDatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHeaderView>
#include <QMap>
#include <QDebug>

ItemsWindow::ItemsWindow(GameDatabase* db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
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
    setStyleSheet("background-color: #1e1e1e; color: #cccccc;");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);

    auto* title = new QLabel("<h2>База предметов</h2>", this);
    mainLayout->addWidget(title);

    auto* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Поиск:", this));

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Введите название или ID предмета...");
    m_searchEdit->setStyleSheet("background: #2d2d2d; border: 1px solid #444; padding: 6px; color: white; border-radius: 3px;");
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
    m_table->setStyleSheet(
        "QTableWidget { background-color: #252525; gridline-color: #444; color: #ccc; margin-top: 10px; }"
        "QHeaderView::section { background-color: #1e1e1e; padding: 4px; border: 1px solid #444; color: #fff; }"
        "QTableWidget::item:selected { background-color: #2d5a27; }"
    );

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
        QString id = it.key();
        QString name = it.value();

        m_table->setItem(row, 0, new QTableWidgetItem(id));
        m_table->setItem(row, 1, new QTableWidgetItem(name));

        row++;
    }

    qDebug() << "ItemsWindow: populated" << items.size() << "items.";
}

void ItemsWindow::filterItems(const QString& text)
{
    if (text.isEmpty()) {
        for (int i = 0; i < m_table->rowCount(); ++i) {
            m_table->setRowHidden(i, false);
        }
        return;
    }

    for (int i = 0; i < m_table->rowCount(); ++i) {
        QTableWidgetItem* idItem = m_table->item(i, 0);
        QTableWidgetItem* nameItem = m_table->item(i, 1);

        if (!idItem || !nameItem) continue;

        QString id = idItem->text();
        QString name = nameItem->text();

        bool matches = id.contains(text, Qt::CaseInsensitive) ||
            name.contains(text, Qt::CaseInsensitive);

        m_table->setRowHidden(i, !matches);
    }
}
