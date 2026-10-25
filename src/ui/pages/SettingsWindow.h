#pragma once
#include <QWidget>
#include <QPixmap>
#include "core/DBManager.h"

class QLabel;
class QPushButton;
class QStackedWidget;

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

private:
    void setupUi();
    void loadAvatar();
    QPixmap getCircularPixmap(const QPixmap& source, int size); // <--- НОВЫЙ ХЕЛПЕР

    QLabel* m_userInfoLabel = nullptr;
    QLabel* m_usernameLabel = nullptr;
    QLabel* m_pageTitleLabel = nullptr;
    QStackedWidget* m_pages = nullptr;

    QLabel* m_avatarLabel = nullptr;
    QLabel* m_sidebarAvatarLabel = nullptr; // <--- Аватар в сайдбаре
    QLabel* m_accountNameLabel = nullptr;
    QLabel* m_accountDetailsLabel = nullptr;

    UserInfo m_userInfo;
};
