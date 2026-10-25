#include "SidebarButton.h"
#include <QLabel>
#include <QResizeEvent>

SidebarButton::SidebarButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    m_counter = new QLabel(this);
    m_counter->setObjectName("sidebarCounter");
    m_counter->setAlignment(Qt::AlignCenter);
    m_counter->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_counter->setFixedHeight(22);
    m_counter->setMinimumWidth(28);
    m_counter->hide();
}

void SidebarButton::setCounter(int count)
{
    m_counter->setText(QString::number(count));
    m_counter->setMinimumWidth(28);
    m_counter->setFixedHeight(22);
    m_counter->adjustSize();
    m_counter->show();
    repositionCounter();
}

void SidebarButton::hideCounter()
{
    m_counter->hide();
}

void SidebarButton::resizeEvent(QResizeEvent* event)
{
    QPushButton::resizeEvent(event);
    repositionCounter();
}

void SidebarButton::repositionCounter()
{
    if (!m_counter || m_counter->isHidden()) return;
    int x = width() - m_counter->width() - 8;
    int y = (height() - m_counter->height()) / 2;
    m_counter->move(x, y);
}
