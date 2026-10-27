#pragma once

#include <QWidget>
#include <QJsonArray>
#include <QString>

class QTableView;
class QTimer;
class QResizeEvent;
class QProgressBar;
class QStackedLayout;
class QScrollBar;
class LotsModel;

class LotsTable : public QWidget
{
    Q_OBJECT
public:
    explicit LotsTable(QWidget* parent = nullptr);

    void setLots(const QJsonArray& lots,
        const QString& itemId,
        const QString& itemName,
        const QString& itemIconPath);

    void clear();

    void showLoading();
    void hideLoading();
    void setLoadingProgress(int current, int total);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateTimers();

private:
    void setupUi();
    void applyRoundedMask();
    void adjustColumnWidths();
    void updateTableHeight(); // <--- НОВЫЙ МЕТОД

    QWidget* m_tableContainer = nullptr;
    QWidget* m_scrollContainer = nullptr;

    QStackedLayout* m_stackLayout = nullptr;
    QWidget* m_loadingWidget = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QScrollBar* m_externalScrollBar = nullptr;

    QTableView* m_view = nullptr;
    LotsModel* m_model = nullptr;
    QTimer* m_timer = nullptr;
};
