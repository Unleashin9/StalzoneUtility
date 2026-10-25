#pragma once
#include <QString>
#include <QSqlDatabase>

struct UserInfo {
    int id = 0;
    QString username;
    QString createdAt;
};

class DBManager
{
public:
    static DBManager& instance();

    bool init();
    bool registerUser(const QString& username, const QString& password);
    bool authenticateUser(const QString& username, const QString& password);
    bool userExists(const QString& username);
    UserInfo getUserInfo(const QString& username);
    bool updatePassword(const QString& username, const QString& newPassword);

private:
    DBManager() = default;
    ~DBManager() = default;
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase m_db;
};
