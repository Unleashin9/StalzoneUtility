#pragma once
#include <QWidget>
#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QPainter>
#include <QPainterPath>
#include <QFrame>
#include <QLineEdit>
#include <QRegularExpression>
#include "core/DBManager.h"   // <--- ВОТ ТАК

class QLabel;
class QPushButton;
class QToolButton;
class QStackedWidget;
class QShowEvent;

class SettingsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWindow(QWidget* parent = nullptr);
    void setUserInfo(const UserInfo& info);

signals:
    void backRequested();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onChangeAvatarClicked();
    void onSavePasswordClicked();

private:
    void setupUi();
    void loadAvatar();
    QPixmap getCircularPixmap(const QPixmap& source, int size);

    UserInfo m_userInfo;
    QLabel* m_userInfoLabel = nullptr;
    QLabel* m_usernameLabel = nullptr;
    QLabel* m_accountNameLabel = nullptr;
    QLabel* m_accountDetailsLabel = nullptr;
    QLabel* m_avatarLabel = nullptr;
    QLabel* m_sidebarAvatarLabel = nullptr;
    QLabel* m_pageTitleLabel = nullptr;
    QLineEdit* m_loginField = nullptr;
    QLineEdit* m_emailField = nullptr;
    QLineEdit* m_currentPassField = nullptr;
    QLineEdit* m_newPassField = nullptr;
    QLineEdit* m_confirmPassField = nullptr;
    QPushButton* m_settingsSidebarBtn = nullptr;
    QStackedWidget* m_pages = nullptr;
};
