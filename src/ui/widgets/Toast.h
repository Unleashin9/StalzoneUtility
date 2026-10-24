#pragma once
#include <QWidget>

class QLabel;

class Toast : public QWidget
{
    Q_OBJECT
public:
    // source — любой виджет, от которого мы найдем MainWindow (по цепочке родителей)
    static void showMessage(QWidget* source, const QString& text, bool isError = true);

private:
    Toast();
    void setupUi(const QString& text, bool isError);
    void showAnimated();

    QLabel* m_label;
};
