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

    layout->addStretch();
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
    m_lotsValue->setText("...");
    m_avgValue->setText("...");
    m_minValue->setText("...");
    m_maxValue->setText("...");
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
    m_lotsValue->setText(QString::number(lotsCount));
    m_avgValue->setText(QString::number(qRound(avgPrice)) + " ₽");
    m_minValue->setText(QString::number(qRound(minPrice)) + " ₽");
    m_maxValue->setText(QString::number(qRound(maxPrice)) + " ₽");
}
