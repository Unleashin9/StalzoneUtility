#include "Toast.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>

Toast::Toast() : QWidget(nullptr)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);  // Клики проходят сквозь
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);      // Не крадет фокус
    // Tool + WindowStaysOnTopHint, чтобы быть поверх LoginWindow/RegisterWindow,
    // но не создаваться как дочернее окно, которое замыливается blur-эффектом
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

void Toast::showMessage(QWidget* source, const QString& text, bool isError)
{
    Toast* toast = new Toast();
    toast->setupUi(text, isError);

    // Ищем MainWindow, поднимаясь по цепочке родителей
    QWidget* anchor = source;
    while (anchor && anchor->parentWidget()) {
        anchor = anchor->parentWidget();
    }

    if (anchor && anchor->isVisible()) {
        // Позиционируем сверху по центру относительно MainWindow
        QPoint anchorTopCenter = anchor->mapToGlobal(QPoint(anchor->width() / 2, 0));
        int x = anchorTopCenter.x() - toast->width() / 2;
        int y = anchorTopCenter.y() + 20;
        toast->move(x, y);
    }
    else {
        // Fallback: центр экрана сверху
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect sg = screen->geometry();
            int x = sg.center().x() - toast->width() / 2;
            int y = sg.top() + 20;
            toast->move(x, y);
        }
    }

    toast->showAnimated();
}

void Toast::setupUi(const QString& text, bool isError)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_label = new QLabel(text, this);
    m_label->setObjectName(isError ? "toastError" : "toastSuccess");
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true);
    m_label->setMinimumWidth(280);

    QString color = isError ? "#F87171" : "#A3E635";
    m_label->setStyleSheet(QString(
        "background-color: #2D2D2D; "
        "color: %1; "
        "border: 1px solid %1; "
        "border-radius: 8px; "
        "padding: 12px 24px; "
        "font-weight: bold; "
        "font-size: 13px;"
    ).arg(color));

    layout->addWidget(m_label);
    adjustSize();
}

void Toast::showAnimated()
{
    show();
    raise();

    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);

    auto* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(300);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(3000, this, [this]() {
        auto* fadeOut = new QPropertyAnimation(graphicsEffect(), "opacity");
        fadeOut->setDuration(300);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        connect(fadeOut, &QPropertyAnimation::finished, this, &QWidget::deleteLater);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });
}
