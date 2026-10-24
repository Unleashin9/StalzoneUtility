#pragma once
#include <QDialog>

class QLineEdit;
class QCheckBox;

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

    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_rememberCheck;
};
