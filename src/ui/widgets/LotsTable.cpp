#include "LotsTable.h"
#include "../models/LotsModel.h"

#include <QTableView>
#include <QHeaderView>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QLabel>
#include <QProgressBar>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPainterPath>
#include <QPainter>
#include <QRegion>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QBrush>
#include <QVariant>
#include <QApplication>
#include <QMouseEvent>
#include <QShowEvent>

namespace {

    constexpr int kRadius = 10;
    constexpr int kBorderW = 1;

    QString rootStyle()
    {
        return QStringLiteral(
            "#lotsTableRoot {"
            "  background-color: #1F1A16;"
            "  border: 1px solid #3A2F26;"
            "  border-radius: 10px;"
            "}");
    }

    QString scrollContainerStyle()
    {
        return QStringLiteral(
            "#lotsScrollRoot {"
            "  background-color: #1F1A16;"
            "  border: 1px solid #3A2F26;"
            "  border-radius: 10px;"
            "}");
    }

    QString viewStyle()
    {
        return QStringLiteral(
            "QTableView {"
            "  background: transparent;"
            "  border: none;"
            "  outline: none;"
            "  selection-background-color: transparent;"
            "  selection-color: #F5E6D3;"
            "}"
            "QTableView::item:selected, QTableView::item:hover {"
            "  background: transparent;"
            "  color: #F5E6D3;"
            "}"
            "QHeaderView::section {"
            "  background-color: #100d0a;"
            "  color: #FFFFFF;"
            "  font-size: 13px;"
            "  font-weight: 800;"
            "  padding: 12px 10px;"
            "  border: none;"
            "  border-bottom: 1px solid #3A2F26;"
            "}"
        );
    }

    class RoundedContainer : public QWidget {
    public:
        explicit RoundedContainer(int radius, bool needMask, QWidget* parent = nullptr)
            : QWidget(parent), m_radius(radius), m_needMask(needMask) {
            setAttribute(Qt::WA_StyledBackground, true);
        }
    protected:
        void resizeEvent(QResizeEvent* e) override {
            QWidget::resizeEvent(e);
            applyMask();
        }
        void showEvent(QShowEvent* e) override {
            QWidget::showEvent(e);
            applyMask();
        }
    private:
        void applyMask() {
            if (!m_needMask) return;
            if (width() > 0 && height() > 0) {
                QRectF r = QRectF(rect());
                QPainterPath path;
                path.addRoundedRect(r, m_radius, m_radius);
                setMask(QRegion(path.toFillPolygon().toPolygon()));
            }
        }
        int m_radius;
        bool m_needMask;
    };

    class LotsTableView : public QTableView
    {
    public:
        using QTableView::QTableView;
    protected:
        void mousePressEvent(QMouseEvent* event) override { event->accept(); }
        void mouseDoubleClickEvent(QMouseEvent* event) override { event->accept(); }
        void currentChanged(const QModelIndex&, const QModelIndex&) override {}
    };

    class LotsItemDelegate : public QStyledItemDelegate
    {
    public:
        explicit LotsItemDelegate(int hPad, QObject* parent = nullptr)
            : QStyledItemDelegate(parent)
            , m_hPad(hPad)
        {
        }

        void paint(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override
        {
            QStyleOptionViewItem opt = option;
            initStyleOption(&opt, index);

            opt.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus | QStyle::State_MouseOver);

            QColor bg = (index.row() % 2 == 0) ? QColor(0x1F, 0x1A, 0x16) : QColor(0x18, 0x14, 0x11);
            painter->fillRect(opt.rect, bg);

            const QString text = index.data(Qt::DisplayRole).toString();
            if (text.isEmpty()) return;

            QFont font = opt.font;
            const QVariant fontVar = index.data(Qt::FontRole);
            if (fontVar.canConvert<QFont>()) font = fontVar.value<QFont>();

            QColor color;
            const QVariant fgVar = index.data(Qt::ForegroundRole);
            if (fgVar.canConvert<QBrush>()) {
                const QColor c = fgVar.value<QBrush>().color();
                if (c.isValid()) color = c;
            }
            if (!color.isValid()) color = opt.palette.color(QPalette::Text);

            Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter;
            const QVariant alignVar = index.data(Qt::TextAlignmentRole);
            if (alignVar.isValid()) align = Qt::Alignment(alignVar.toInt());

            const QRect textRect = opt.rect.adjusted(m_hPad, 0, -m_hPad, 0);

            painter->save();
            painter->setFont(font);
            painter->setPen(color);
            painter->drawText(textRect, align | Qt::TextSingleLine, text);
            painter->restore();
        }

    private:
        int m_hPad;
    };

} // namespace

LotsTable::LotsTable(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    // Используем Fixed по вертикали, так как высоту мы задаем вручную
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setupUi();
}

void LotsTable::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);

    // --- КОНТЕЙНЕР ТАБЛИЦЫ ---
    m_tableContainer = new RoundedContainer(kRadius, true, this);
    m_tableContainer->setObjectName(QStringLiteral("lotsTableRoot"));
    m_tableContainer->setStyleSheet(rootStyle());

    m_stackLayout = new QStackedLayout(m_tableContainer);
    m_stackLayout->setContentsMargins(kBorderW, kBorderW, kBorderW, kBorderW);

    m_model = new LotsModel(this);

    m_view = new LotsTableView(m_tableContainer);
    m_view->setObjectName(QStringLiteral("lotsTable"));
    m_view->setModel(m_model);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setStyleSheet(viewStyle());
    m_view->setItemDelegate(new LotsItemDelegate(10, m_view));

    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setShowGrid(false);
    m_view->setWordWrap(false);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setAlternatingRowColors(true);

    QPalette pal = m_view->palette();
    pal.setColor(QPalette::Base, QColor(0x1F, 0x1A, 0x16));
    pal.setColor(QPalette::AlternateBase, QColor(0x18, 0x14, 0x11));
    pal.setColor(QPalette::Text, QColor(0xF5, 0xE6, 0xD3));
    pal.setColor(QPalette::Highlight, Qt::transparent);
    m_view->setPalette(pal);

    QHeaderView* hh = m_view->horizontalHeader();
    hh->setStretchLastSection(false);
    hh->setSectionResizeMode(QHeaderView::Fixed);
    hh->setHighlightSections(false);
    hh->setFixedHeight(45);

    QHeaderView* vh = m_view->verticalHeader();
    vh->setVisible(false);
    vh->setDefaultSectionSize(30);
    vh->setSectionResizeMode(QHeaderView::Fixed);
    vh->setMinimumSectionSize(30);

    m_view->viewport()->setAutoFillBackground(false);
    m_view->setSortingEnabled(true);

    // --- ЭКРАН ЗАГРУЗКИ ---
    m_loadingWidget = new QWidget(m_tableContainer);
    m_loadingWidget->setStyleSheet("background-color: transparent;");
    auto* loadingLayout = new QVBoxLayout(m_loadingWidget);
    loadingLayout->setAlignment(Qt::AlignCenter);
    loadingLayout->setSpacing(15);

    auto* loadingLabel = new QLabel("Загрузка лотов...", m_loadingWidget);
    loadingLabel->setStyleSheet("color: #F0A93B; font-size: 18px; font-weight: bold; background: transparent;");
    loadingLabel->setAlignment(Qt::AlignCenter);

    m_progressBar = new QProgressBar(m_loadingWidget);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedWidth(300);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #2A2F26; border: none; border-radius: 3px; }"
        "QProgressBar::chunk { background-color: #F0A93B; border-radius: 3px; }"
    );

    loadingLayout->addWidget(loadingLabel, 0, Qt::AlignHCenter);
    loadingLayout->addWidget(m_progressBar, 0, Qt::AlignHCenter);

    m_stackLayout->addWidget(m_view);
    m_stackLayout->addWidget(m_loadingWidget);
    m_stackLayout->setCurrentWidget(m_view);

    // --- КОНТЕЙНЕР СКРОЛЛБАРА ---
    m_scrollContainer = new RoundedContainer(kRadius, true, this);
    m_scrollContainer->setObjectName(QStringLiteral("lotsScrollRoot"));
    m_scrollContainer->setStyleSheet(scrollContainerStyle());
    m_scrollContainer->setFixedWidth(16);
    m_scrollContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_scrollContainer->hide();

    auto* scrollLayout = new QVBoxLayout(m_scrollContainer);
    scrollLayout->setContentsMargins(4, 4, 4, 4);
    scrollLayout->setSpacing(0);

    m_externalScrollBar = new QScrollBar(Qt::Vertical, m_scrollContainer);
    m_externalScrollBar->setFixedWidth(8);
    m_externalScrollBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_externalScrollBar->setStyleSheet(
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #3A2F26; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #F0A93B; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
    );
    scrollLayout->addWidget(m_externalScrollBar);

    // Синхронизация
    connect(m_view->verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min, int max) {
        m_externalScrollBar->setRange(min, max);
        m_externalScrollBar->setPageStep(m_view->verticalScrollBar()->pageStep());

        bool needScroll = (max > 0);
        if (m_scrollContainer->isVisible() != needScroll) {
            m_scrollContainer->setVisible(needScroll);
            QTimer::singleShot(0, this, &LotsTable::adjustColumnWidths);
        }
        });
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, m_externalScrollBar, &QScrollBar::setValue);
    connect(m_externalScrollBar, &QScrollBar::valueChanged, m_view->verticalScrollBar(), &QScrollBar::setValue);

    mainLayout->addWidget(m_tableContainer, 1);
    mainLayout->addWidget(m_scrollContainer);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &LotsTable::updateTimers);
    m_timer->start();
}

void LotsTable::showLoading()
{
    m_progressBar->setValue(0);
    m_model->clear();
    m_stackLayout->setCurrentWidget(m_loadingWidget);
}

void LotsTable::hideLoading()
{
    m_stackLayout->setCurrentIndex(0);
}

void LotsTable::setLoadingProgress(int current, int total)
{
    if (total > 0) {
        int percent = (current * 100) / total;
        m_progressBar->setValue(percent);
    }
    else {
        m_progressBar->setValue(0);
    }
}

void LotsTable::setLots(const QJsonArray& lots,
    const QString& itemId,
    const QString& itemName,
    const QString& itemIconPath)
{
    Q_UNUSED(itemId);
    Q_UNUSED(itemIconPath);

    if (lots.isEmpty()) {
        m_model->showEmptyState(itemName);
        m_view->setSpan(0, 0, 1, LotsModel::ColumnCount);
    }
    else {
        m_view->clearSpans();
        m_model->setLots(lots, itemName);
    }

    updateTableHeight();
    QTimer::singleShot(0, this, &LotsTable::adjustColumnWidths);
}

void LotsTable::clear()
{
    m_view->clearSpans();
    m_model->clear();
    updateTableHeight();
}

void LotsTable::updateTableHeight()
{
    if (!m_model) return;

    int rows = m_model->rowCount();
    if (rows == 0) rows = 1; // Для заглушки

    int headerHeight = 45;
    int rowHeight = 30;
    int border = kBorderW * 2;
    int contentHeight = headerHeight + (rows * rowHeight) + border;

    // Получаем доступную высоту от родителя. 
    // parentWidget()->height() - это высота lotsContainer в ItemDetailWidget.
    // Вычитаем 20 (отступы лейаута: 10 сверху, 10 снизу).
    int availableHeight = QWIDGETSIZE_MAX;
    if (parentWidget()) {
        availableHeight = qMax(100, parentWidget()->height() - 20);
    }

    // Если контент меньше экрана — фиксируем высоту по контенту. 
    // Если больше — растягиваем до низа экрана (сработает скроллбар).
    int finalHeight = qMin(contentHeight, availableHeight);

    setFixedHeight(finalHeight);
}

void LotsTable::updateTimers()
{
    m_model->refreshTimeLeft();
}

void LotsTable::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    applyRoundedMask();

    // При ресайзе окна пересчитываем высоту
    QTimer::singleShot(0, this, &LotsTable::updateTableHeight);
    QTimer::singleShot(0, this, &LotsTable::adjustColumnWidths);
}

void LotsTable::adjustColumnWidths()
{
    if (!m_view || !m_view->horizontalHeader()) return;

    int totalWidth = m_view->viewport()->width();
    if (totalWidth <= 0) return;

    int w1 = totalWidth / 4;
    int remaining = totalWidth - w1;
    int w2 = remaining / 6;
    int wLast = remaining - (w2 * 5);

    m_view->setColumnWidth(LotsModel::LotName, w1);
    m_view->setColumnWidth(LotsModel::Enchant, w2);
    m_view->setColumnWidth(LotsModel::Quantity, w2);
    m_view->setColumnWidth(LotsModel::StartPrice, w2);
    m_view->setColumnWidth(LotsModel::BuyoutPrice, w2);
    m_view->setColumnWidth(LotsModel::PerItemPrice, w2);
    m_view->setColumnWidth(LotsModel::TimeLeft, wLast);
}

void LotsTable::applyRoundedMask()
{
    if (m_tableContainer && m_tableContainer->width() > 0 && m_tableContainer->height() > 0) {
        QRectF r1 = QRectF(m_tableContainer->rect());
        QPainterPath path1;
        path1.addRoundedRect(r1, kRadius, kRadius);
        m_tableContainer->setMask(QRegion(path1.toFillPolygon().toPolygon()));
    }

    if (m_scrollContainer && m_scrollContainer->isVisible() && m_scrollContainer->width() > 0) {
        QRectF r2 = QRectF(m_scrollContainer->rect());
        QPainterPath path2;
        path2.addRoundedRect(r2, kRadius, kRadius);
        m_scrollContainer->setMask(QRegion(path2.toFillPolygon().toPolygon()));
    }
}
