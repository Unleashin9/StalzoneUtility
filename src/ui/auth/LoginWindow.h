#pragma once
#include <QDialog>

class QLineEdit;
class QCheckBox;
class QAction;
class QVBoxLayout;
class QWidget;
class QPushButton;
class QResizeEvent;

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
    void resizeEvent(QResizeEvent* event) override; // <--- ОБЪЯВЛЕНО

private slots:
    void onLoginClicked();
    void togglePasswordVisibility();

private:
    void setupUi();
    void addDivider(QVBoxLayout* layout, QWidget* parent);

    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_rememberCheck;
    QAction* m_togglePasswordAction;
    QPushButton* m_closeBtn; // <--- ОБЪЯВЛЕНО
};
