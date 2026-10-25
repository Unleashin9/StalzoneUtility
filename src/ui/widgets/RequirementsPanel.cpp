#include "RequirementsPanel.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

RequirementsPanel::RequirementsPanel(QWidget* parent) : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setupUi();
    resetAll();
}

void RequirementsPanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* container = new QWidget(this);
    container->setObjectName("requirementsContainer");
    mainLayout->addWidget(container);

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 180));
    shadow->setOffset(0, 0);
    container->setGraphicsEffect(shadow);

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    auto* userTitle = new QLabel("Требования к имени", container);
    userTitle->setObjectName("rulesTitle");
    layout->addWidget(userTitle);
    layout->addSpacing(5);

    QStringList uRules = {
        "Длина имени: от 3 до 16 символов.",
        "Без цифр и символов, кроме нижнего подчеркивания (_).",
        "Только латиница или только кириллица.",
        "Начинается с заглавной буквы."
    };

    for (const QString& text : uRules) {
        auto* row = new QHBoxLayout();
        row->setSpacing(8);
        row->setAlignment(Qt::AlignVCenter);

        auto* icon = new QLabel(container);
        icon->setFixedSize(16, 16);

        auto* lbl = new QLabel(text, container);
        lbl->setWordWrap(true);
        lbl->setStyleSheet("color: #A8998A; font-size: 11px;");
        lbl->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        row->addWidget(icon, 0, Qt::AlignVCenter);
        row->addWidget(lbl, 1, Qt::AlignVCenter);

        layout->addLayout(row);
        m_icons.append(icon);
    }

    layout->addSpacing(10);
    auto* line = new QFrame(container);
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet("background-color: #3A2F26; border: none;");
    layout->addWidget(line);
    layout->addSpacing(10);

    auto* passTitle = new QLabel("Требования к паролю", container);
    passTitle->setObjectName("rulesTitle");
    layout->addWidget(passTitle);
    layout->addSpacing(5);

    QStringList pRules = {
        "Минимальная длина пароля: 12 символов.",
        "Только латиница, цифры и спецсимволы."
    };

    for (const QString& text : pRules) {
        auto* row = new QHBoxLayout();
        row->setSpacing(8);
        row->setAlignment(Qt::AlignVCenter);

        auto* icon = new QLabel(container);
        icon->setFixedSize(16, 16);

        auto* lbl = new QLabel(text, container);
        lbl->setWordWrap(true);
        lbl->setStyleSheet("color: #A8998A; font-size: 11px;");
        lbl->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        row->addWidget(icon, 0, Qt::AlignVCenter);
        row->addWidget(lbl, 1, Qt::AlignVCenter);

        layout->addLayout(row);
        m_icons.append(icon);
    }

    setFixedWidth(290);
    adjustSize();
    setFixedHeight(height());
}

void RequirementsPanel::setRuleValid(int index, bool isValid)
{
    if (index < 0 || index >= m_icons.size()) return;

    QColor color = isValid ? ThemeManager::instance().accentColor() : QColor("#E5533D");

    QIcon icon = ThemeManager::instance().getIcon(
        isValid ? "yes" : "no",
        color,
        "RegisterWindow");

    if (!icon.isNull()) {
        m_icons[index]->setPixmap(icon.pixmap(16, 16));
    }
}

void RequirementsPanel::resetAll()
{
    for (int i = 0; i < m_icons.size(); ++i) {
        setRuleValid(i, false);
    }
}
