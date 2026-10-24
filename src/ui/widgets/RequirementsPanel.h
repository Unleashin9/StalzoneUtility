#pragma once
#include <QWidget>
#include <QVector>

class QLabel;

class RequirementsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RequirementsPanel(QWidget* parent = nullptr);

    void setRuleValid(int index, bool isValid);
    void resetAll();

    static constexpr int TotalRules = 6;

private:
    void setupUi();
    QVector<QLabel*> m_icons;
};
