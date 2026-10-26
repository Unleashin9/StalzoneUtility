#include "SidebarAuctionStats.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QtMath>

SidebarAuctionStats::SidebarAuctionStats(QWidget* parent) : QWidget(parent)
{
    setObjectName("sidebarAuctionStats");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(210);
    setupUi();
    reset();
}

void SidebarAuctionStats::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 12, 15, 12);
    layout->setSpacing(4);

    auto addBlock = [this, layout](const QString& title, QLabel*& valueOut) {
        auto* titleLabel = new QLabel(title, this);
        titleLabel->setObjectName("statTitle");
        layout->addWidget(titleLabel);

        valueOut = new QLabel("—", this);
        valueOut->setObjectName("statValue");
        layout->addWidget(valueOut);
        };

    auto addDivider = [this, layout]() {
        auto* line = new QFrame(this);
        line->setObjectName("statDivider");
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Plain);
        line->setFixedHeight(1);
        layout->addWidget(line);
        };

    addBlock("ЛОТОВ", m_lotsValue);
    addDivider();
    addBlock("СРЕДНЯЯ ЦЕНА", m_avgValue);
    addDivider();
    addBlock("МИНИМАЛЬНАЯ ЦЕНА", m_minValue);
    addDivider();
    addBlock("МАКСИМАЛЬНАЯ ЦЕНА", m_maxValue);
}

void SidebarAuctionStats::reset()
{
    m_lotsValue->setText("—");
    m_avgValue->setText("—");
    m_minValue->setText("—");
    m_maxValue->setText("—");
}

void SidebarAuctionStats::setLoading()
{
    m_lotsValue->setText("Загрузка...");
    m_avgValue->setText("Загрузка...");
    m_minValue->setText("Загрузка...");
    m_maxValue->setText("Загрузка...");
}

void SidebarAuctionStats::setStats(int lotsCount, double avgPrice, double minPrice, double maxPrice)
{
    if (lotsCount <= 0) {
        m_lotsValue->setText("0");
        m_avgValue->setText("—");
        m_minValue->setText("—");
        m_maxValue->setText("—");
        return;
    }
    m_lotsValue->setText(formatPrice(lotsCount));
    m_avgValue->setText(formatPrice(qRound(avgPrice)) + " ₽");
    m_minValue->setText(formatPrice(qRound(minPrice)) + " ₽");
    m_maxValue->setText(formatPrice(qRound(maxPrice)) + " ₽");
}

QString SidebarAuctionStats::formatPrice(int value) const
{
    QString s = QString::number(value);
    if (value < 0) s = s.mid(1); // убираем минус на время форматирования

    QString result;
    int count = 0;
    for (int i = s.length() - 1; i >= 0; --i) {
        result.prepend(s[i]);
        ++count;
        if (count % 3 == 0 && i > 0) {
            result.prepend(' ');
        }
    }
    return (value < 0 ? "-" : "") + result;
}
