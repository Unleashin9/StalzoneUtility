#include "FiltersBar.h"
#include "core/ThemeManager.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QIntValidator>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

QStringList FiltersBar::rarityNames()
{
    return { "Обычный", "Необычный", "Особый", "Редкий",
             "Исключительный", "Легендарный", "Уникальный" };
}

QStringList FiltersBar::rarityColors()
{
    return { "#B0B0B0", "#4CD964", "#4A9EFF", "#C84BFF",
             "#FF9500", "#FF4D4D", "#FF1493" };
}

FiltersBar::FiltersBar(QWidget* parent) : QWidget(parent)
{
    setObjectName("filtersBar");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
}

void FiltersBar::setupUi()
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_rarityBtn = new QPushButton(" Редкость: все", this);
    m_rarityBtn->setObjectName("filterBtn");
    m_rarityBtn->setIcon(ThemeManager::instance().getIcon("filter", QColor("#A8998A"), "AuctionWindow"));
    m_rarityBtn->setIconSize(QSize(16, 16));
    m_rarityBtn->setCursor(Qt::PointingHandCursor);
    m_rarityBtn->setMinimumHeight(38);
    m_rarityBtn->setMinimumWidth(180);
    connect(m_rarityBtn, &QPushButton::clicked, this, &FiltersBar::onRarityFilterClicked);
    layout->addWidget(m_rarityBtn);

    m_enchantBtn = new QPushButton(" Заточка: любая", this);
    m_enchantBtn->setObjectName("filterBtn");
    m_enchantBtn->setIcon(ThemeManager::instance().getIcon("filter", QColor("#A8998A"), "AuctionWindow"));
    m_enchantBtn->setIconSize(QSize(16, 16));
    m_enchantBtn->setCursor(Qt::PointingHandCursor);
    m_enchantBtn->setMinimumHeight(38);
    m_enchantBtn->setMinimumWidth(180);
    connect(m_enchantBtn, &QPushButton::clicked, this, &FiltersBar::onEnchantFilterClicked);
    layout->addWidget(m_enchantBtn);

    layout->addStretch();

    m_rarityPopup = createRarityPopup();
    m_enchantPopup = createEnchantPopup();
}

QWidget* FiltersBar::createRarityPopup()
{
    auto* popup = new QWidget(this, Qt::Popup);
    popup->setObjectName("dropdownPopup");
    popup->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(popup);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(2);

    QStringList names = rarityNames();
    for (int i = 0; i < names.size(); ++i) {
        auto* btn = new QPushButton(" " + names[i], popup);
        btn->setObjectName("dropdownItem");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(34);
        btn->setMinimumWidth(200);
        updateRarityItemStyle(i);
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            if (m_selectedRarities.contains(i)) m_selectedRarities.remove(i);
            else m_selectedRarities.insert(i);
            updateRarityItemStyle(i);
            updateRarityButtonText();
            emit raritiesChanged(m_selectedRarities);
            });
        layout->addWidget(btn);
        m_rarityItems.append(btn);
    }

    auto* sep = new QFrame(popup);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background-color: #3A2F26; margin: 6px 0;");
    layout->addWidget(sep);

    auto* resetBtn = new QPushButton(" Сбросить", popup);
    resetBtn->setObjectName("dropdownItem");
    resetBtn->setIcon(ThemeManager::instance().getIcon("filterremove", QColor("#A8998A"), "AuctionWindow"));
    resetBtn->setIconSize(QSize(16, 16));
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setMinimumHeight(34);
    resetBtn->setMinimumWidth(200);
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_selectedRarities.clear();
        for (int i = 0; i < m_rarityItems.size(); ++i) updateRarityItemStyle(i);
        updateRarityButtonText();
        emit raritiesChanged(m_selectedRarities);
        m_rarityPopup->hide();
        });
    layout->addWidget(resetBtn);

    popup->hide();
    return popup;
}

QWidget* FiltersBar::createEnchantPopup()
{
    auto* popup = new QWidget(this, Qt::Popup);
    popup->setObjectName("dropdownPopup");
    popup->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QHBoxLayout(popup);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);

    auto* fromLabel = new QLabel("ОТ:", popup);
    fromLabel->setStyleSheet("color: #A8998A; font-size: 12px;");
    layout->addWidget(fromLabel);

    auto* fromEdit = new QLineEdit(popup);
    fromEdit->setObjectName("enchantEdit");
    fromEdit->setPlaceholderText("0");
    fromEdit->setValidator(new QIntValidator(0, 99, fromEdit));
    fromEdit->setFixedWidth(50);
    layout->addWidget(fromEdit);

    auto* toLabel = new QLabel("ДО:", popup);
    toLabel->setStyleSheet("color: #A8998A; font-size: 12px;");
    layout->addWidget(toLabel);

    auto* toEdit = new QLineEdit(popup);
    toEdit->setObjectName("enchantEdit");
    toEdit->setPlaceholderText("∞");
    toEdit->setValidator(new QIntValidator(0, 99, toEdit));
    toEdit->setFixedWidth(50);
    layout->addWidget(toEdit);

    auto* okBtn = new QPushButton("ОК", popup);
    okBtn->setObjectName("accentButton");
    okBtn->setFixedSize(40, 28);
    okBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(okBtn);

    connect(okBtn, &QPushButton::clicked, this, [this, fromEdit, toEdit]() {
        m_enchantMin = fromEdit->text().toInt();
        m_enchantMax = toEdit->text().isEmpty() ? 99 : toEdit->text().toInt();
        updateEnchantButtonText();
        emit enchantChanged(m_enchantMin, m_enchantMax);
        m_enchantPopup->hide();
        });

    popup->hide();
    return popup;
}

void FiltersBar::updateRarityItemStyle(int index)
{
    if (index < 0 || index >= m_rarityItems.size()) return;
    QString color = rarityColors()[index];
    bool active = m_selectedRarities.contains(index);
    QPushButton* btn = m_rarityItems[index];
    btn->setChecked(active);

    if (active) {
        QColor c(color);
        btn->setStyleSheet(QString(
            "QPushButton { background-color: rgba(%1, %2, %3, 0.15); color: %4; border: 1px solid %4; border-left: 3px solid %4; border-radius: 6px; font-weight: bold; text-align: left; padding: 0px 10px; font-size: 13px; }"
        ).arg(c.red()).arg(c.green()).arg(c.blue()).arg(color));
    }
    else {
        btn->setStyleSheet(
            "QPushButton { background-color: transparent; color: #A8998A; border: 1px solid transparent; border-left: 3px solid transparent; border-radius: 6px; text-align: left; padding: 0px 10px; font-size: 13px; }"
            "QPushButton:hover { background-color: rgba(255, 255, 255, 0.05); }"
        );
    }
}

void FiltersBar::updateRarityButtonText()
{
    if (m_selectedRarities.isEmpty()) m_rarityBtn->setText(" Редкость: все");
    else if (m_selectedRarities.size() == 1) {
        int idx = *m_selectedRarities.begin();
        m_rarityBtn->setText(" Редкость: " + rarityNames()[idx]);
    }
    else {
        m_rarityBtn->setText(QString(" Редкость: %1").arg(m_selectedRarities.size()));
    }
}

void FiltersBar::updateEnchantButtonText()
{
    if (m_enchantMin == 0 && m_enchantMax == 0) {
        m_enchantBtn->setText(" Заточка: любая");
    }
    else {
        QString maxStr = (m_enchantMax >= 99) ? "∞" : QString::number(m_enchantMax);
        m_enchantBtn->setText(QString(" Заточка: %1-%2").arg(m_enchantMin).arg(maxStr));
    }
}

void FiltersBar::onRarityFilterClicked()
{
    if (m_rarityPopup->isVisible()) { m_rarityPopup->hide(); return; }
    m_rarityPopup->adjustSize();
    m_rarityPopup->move(m_rarityBtn->mapToGlobal(QPoint(0, m_rarityBtn->height() + 4)));
    auto* effect = new QGraphicsOpacityEffect(m_rarityPopup);
    effect->setOpacity(0.0);
    m_rarityPopup->setGraphicsEffect(effect);
    m_rarityPopup->show();
    auto* anim = new QPropertyAnimation(effect, "opacity", m_rarityPopup);
    anim->setDuration(150);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FiltersBar::onEnchantFilterClicked()
{
    if (m_enchantPopup->isVisible()) { m_enchantPopup->hide(); return; }
    m_enchantPopup->adjustSize();
    m_enchantPopup->move(m_enchantBtn->mapToGlobal(QPoint(0, m_enchantBtn->height() + 4)));
    m_enchantPopup->show();
}
