#pragma once
#include <QWidget>
#include <QPixmap>
#include "core/DBManager.h"

class QLabel;
class QPushButton;
class QStackedWidget;
class QLineEdit;

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

    QLabel* m_userInfoLabel = nullptr;
    QLabel* m_usernameLabel = nullptr;
    QLabel* m_pageTitleLabel = nullptr;
    QStackedWidget* m_pages = nullptr;

    QLabel* m_avatarLabel = nullptr;
    QLabel* m_sidebarAvatarLabel = nullptr;
    QLabel* m_accountNameLabel = nullptr;
    QLabel* m_accountDetailsLabel = nullptr;

    QLineEdit* m_loginField = nullptr;
    QLineEdit* m_emailField = nullptr;
    QLineEdit* m_currentPassField = nullptr;
    QLineEdit* m_newPassField = nullptr;
    QLineEdit* m_confirmPassField = nullptr;

    QPushButton* m_settingsSidebarBtn = nullptr;

    UserInfo m_userInfo;
};
