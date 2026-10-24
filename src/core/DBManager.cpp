#include "DBManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QRandomGenerator>

// В Qt 6 для PBKDF2 используется этот заголовок
#include <QPasswordDigestor>

DBManager& DBManager::instance()
{
    static DBManager s_instance;
    return s_instance;
}

bool DBManager::init()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("users.db"); // Файл БД создастся рядом с .exe

    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;
    // Таблица пользователей: логин, соль, хэш пароля
    bool success = query.exec("CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "salt BLOB NOT NULL, "
        "password_hash BLOB NOT NULL)");

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

    // Генерируем случайную соль
    QByteArray salt(16, 0);
    for (int i = 0; i < salt.size(); ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->generate() % 256);
    }

    // Хэшируем пароль с помощью PBKDF2 (10000 итераций, SHA-256)
    QByteArray hash = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        salt,
        10000,
        32);

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, salt, password_hash) VALUES (:username, :salt, :hash)");
    query.bindValue(":username", username);
    query.bindValue(":salt", salt);
    query.bindValue(":hash", hash);

    return query.exec();
}

bool DBManager::authenticateUser(const QString& username, const QString& password)
{
    QSqlQuery query;
    query.prepare("SELECT salt, password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        QByteArray salt = query.value(0).toByteArray();
        QByteArray storedHash = query.value(1).toByteArray();

        // Вычисляем хэш введенного пароля с той же солью
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
