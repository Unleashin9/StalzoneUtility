#pragma once
#include <QDialog>

class QLineEdit;
class QPushButton;
class QAction;
class QWidget;
class QVBoxLayout;
class QShowEvent;
class QHideEvent;
class RequirementsPanel;

class RegisterWindow : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterWindow(QWidget* parent = nullptr);
    void clearFields();

signals:
    void registrationSuccessful(const QString& username);
    void loginRequested();
    void windowClosed();

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void onRegisterClicked();
    void validateUsername(const QString& text);
    void validatePassword(const QString& text);

private:
    void setupUi();
    void addDivider(QVBoxLayout* layout, QWidget* parent);

    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QLineEdit* m_confirmEdit = nullptr;
    QLineEdit* m_referralEdit = nullptr;

    QAction* m_togglePassAction = nullptr;
    QAction* m_toggleConfirmAction = nullptr;
    RequirementsPanel* m_reqPanel = nullptr;
};
