#include "DBManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDateTime>
#include <QPasswordDigestor>

DBManager& DBManager::instance()
{
    static DBManager s_instance;
    return s_instance;
}

bool DBManager::init()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("users.db");

    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    // Проверяем актуальность схемы: должна быть колонка created_at
    // и id должен быть INTEGER (а не TEXT/UUID из прошлой версии).
    QSqlQuery check;
    check.exec("PRAGMA table_info(users)");
    bool schemaOk = false;
    bool hasCreatedAt = false;
    bool idIsInteger = false;
    while (check.next()) {
        QString colName = check.value(1).toString();
        QString colType = check.value(2).toString().toUpper();
        if (colName == "created_at") hasCreatedAt = true;
        if (colName == "id" && colType.contains("INT")) idIsInteger = true;
    }

    // Если таблица пустая (нет колонок) — тоже норм (создадим ниже)
    QSqlQuery countCheck;
    countCheck.exec("SELECT count(*) FROM pragma_table_info('users')");
    int colCount = 0;
    if (countCheck.next()) colCount = countCheck.value(0).toInt();

    schemaOk = (colCount == 0) || (hasCreatedAt && idIsInteger);

    if (!schemaOk) {
        QSqlQuery drop;
        drop.exec("DROP TABLE IF EXISTS users");
        qDebug() << "Old database schema detected. Table dropped for regeneration.";
    }

    QSqlQuery query;
    bool success = query.exec("CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "salt BLOB NOT NULL, "
        "password_hash BLOB NOT NULL, "
        "created_at TEXT NOT NULL)");

    if (!success) {
        qDebug() << "Failed to create table:" << query.lastError().text();
    }
    return success;
}

bool DBManager::userExists(const QString& username)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username");
    query.bindValue(":username", username);
    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}

bool DBManager::registerUser(const QString& username, const QString& password)
{
    if (userExists(username)) return false;

    QByteArray salt(16, 0);
    for (int i = 0; i < salt.size(); ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->generate() % 256);
    }

    QByteArray hash = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        salt,
        10000,
        32);

    QString createdAt = QDateTime::currentDateTime().toString("dd/MM/yyyy");

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, salt, password_hash, created_at) "
        "VALUES (:username, :salt, :hash, :created_at)");
    query.bindValue(":username", username);
    query.bindValue(":salt", salt);
    query.bindValue(":hash", hash);
    query.bindValue(":created_at", createdAt);

    if (!query.exec()) {
        qDebug() << "Register SQL Error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::authenticateUser(const QString& username, const QString& password)
{
    QSqlQuery query;
    query.prepare("SELECT salt, password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        QByteArray salt = query.value(0).toByteArray();
        QByteArray storedHash = query.value(1).toByteArray();

        QByteArray inputHash = QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256,
            password.toUtf8(),
            salt,
            10000,
            32);

        return (inputHash == storedHash);
    }
    return false;
}

UserInfo DBManager::getUserInfo(const QString& username)
{
    UserInfo info;
    QSqlQuery query;
    query.prepare("SELECT id, username, created_at FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        info.id = query.value(0).toInt();
        info.username = query.value(1).toString();
        info.createdAt = query.value(2).toString();
    }
    return info;
}
