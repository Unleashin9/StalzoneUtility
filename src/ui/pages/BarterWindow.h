#pragma once
#include <QWidget>

class QComboBox;
class QTableWidget;
class QLabel;
class GameDatabase;

class BarterWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BarterWindow(GameDatabase* db, QWidget* parent = nullptr);

signals:
    void backRequested(); // <--- СИГНАЛ ДЛЯ ВОЗВРАТА

private slots:
    void onRecipeSelected(int index);
    void onDatabaseLoaded();

private:
    void setupUi();

    GameDatabase* m_db;
    QComboBox* m_recipeSelector;
    QTableWidget* m_requirementsTable;
    QLabel* m_statusLabel;
};
