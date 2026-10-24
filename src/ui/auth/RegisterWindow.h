#pragma once
#include <QDialog>

class QLineEdit;

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

private slots:
    void onRegisterClicked();

private:
    void setupUi();

    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmEdit;
};
