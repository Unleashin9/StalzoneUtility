#pragma once
#include <QWidget>

class QVBoxLayout;
class QPushButton;

class ProfileMenu : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileMenu(QWidget* parent = nullptr);
    void setUserName(const QString& name);

signals:
    void settingsUserClicked();
    void settingsAppClicked();
    void logoutClicked();

private:
    void setupUi();
    QPushButton* m_settingsUserBtn = nullptr;
    QPushButton* m_settingsAppBtn = nullptr;
    QPushButton* m_logoutBtn = nullptr;
    QVBoxLayout* m_layout = nullptr;
};
