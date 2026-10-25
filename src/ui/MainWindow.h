#pragma once

#include <QMainWindow>
#include <memory>
#include <QString>
#include "core/DBManager.h"

class QPushButton;
class QGraphicsBlurEffect;

class StalzoneAPI;
class GameDatabase;
class AuctionWindow;
class BarterWindow;
class ItemsWindow;
class LoginWindow;
class RegisterWindow;
class SettingsWindow;
class ProfileMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onAuctionClicked();
    void onBarterClicked();
    void onItemsClicked();
    void onBackToMain();

    void onProfileButtonClicked();
    void onLoginSuccess(const QString& username);
    void onShowRegisterWindow();
    void onShowLoginWindow();
    void onLogout();
    void onOpenUserSettings();

private:
    void setupUi();
    void showSubWindow(QWidget* window);
    void updateProfileButton();
    void applyBlurEffect();
    void removeBlurEffect();
    void positionDialog(QWidget* dialog);
    void blockMainWindow();
    void unblockMainWindow();
    void updateIcons();

    void showProfileMenu();
    void hideProfileMenu();

    StalzoneAPI* m_api = nullptr;
    GameDatabase* m_db = nullptr;

    std::unique_ptr<AuctionWindow> m_auctionWindow;
    std::unique_ptr<BarterWindow>  m_barterWindow;
    std::unique_ptr<ItemsWindow>   m_itemsWindow;
    std::unique_ptr<SettingsWindow> m_settingsWindow;

    LoginWindow* m_loginWindow = nullptr;
    RegisterWindow* m_registerWindow = nullptr;
    ProfileMenu* m_profileMenu = nullptr;

    QPushButton* m_profileBtn = nullptr;
    QString m_currentUser;
    UserInfo m_currentUserInfo;
};
