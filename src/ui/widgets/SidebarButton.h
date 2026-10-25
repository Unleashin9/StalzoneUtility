#pragma once
#include <QPushButton>

class QLabel;
class QResizeEvent;

class SidebarButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SidebarButton(const QString& text, QWidget* parent = nullptr);

    void setCounter(int count);
    void hideCounter();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void repositionCounter();

    QLabel* m_counter = nullptr;
};
