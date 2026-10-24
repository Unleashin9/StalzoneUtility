#pragma once
#include <QDialog>

class QLineEdit;
class QCheckBox;
class QAction;
class QVBoxLayout;
class QWidget;
class QPushButton;

class LoginWindow : public QDialog
{
    Q_OBJECT
public:
    explicit LoginWindow(QWidget* parent = nullptr);

signals:
    void loginSuccessful(const QString& username);
    void registerRequested();
    void windowClosed();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onLoginClicked();
    void togglePasswordVisibility();

private:
    void setupUi();
    void addDivider(QVBoxLayout* layout, QWidget* parent);

    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QCheckBox* m_rememberCheck = nullptr;
    QAction* m_togglePasswordAction = nullptr;
};
