#pragma once
#include <QString>
#include <QSqlDatabase>

class DBManager
{
public:
    static DBManager& instance();

    bool init(); // Инициализация БД и создание таблицы
    bool registerUser(const QString& username, const QString& password);
    bool authenticateUser(const QString& username, const QString& password);
    bool userExists(const QString& username);

private:
    DBManager() = default;
    ~DBManager() = default;
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase m_db;
};
