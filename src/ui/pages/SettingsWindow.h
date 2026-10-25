#pragma once
#include <QWidget>
#include "core/DBManager.h"

class QLabel;
class QPushButton;

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

private:
    void setupUi();

    QLabel* m_userInfoLabel = nullptr;
    QLabel* m_usernameLabel = nullptr; // <--- НОВОЕ
    UserInfo m_userInfo;
};
