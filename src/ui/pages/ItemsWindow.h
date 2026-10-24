#pragma once

#include <QWidget>

class QTableWidget;
class QLineEdit;
class GameDatabase;

class ItemsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ItemsWindow(GameDatabase* db, QWidget* parent = nullptr);

private slots:
    void onDatabaseLoaded();
    void filterItems(const QString& text);

private:
    void setupUi();
    void populateTable();

    GameDatabase* m_db;
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
};
