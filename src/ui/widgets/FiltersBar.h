#pragma once
#include <QWidget>
#include <QSet>
#include <QStringList>

class QPushButton;

class FiltersBar : public QWidget
{
    Q_OBJECT
public:
    explicit FiltersBar(QWidget* parent = nullptr);

signals:
    void raritiesChanged(const QSet<int>& selectedRarities);
    void enchantChanged(int minValue, int maxValue);

private slots:
    void onRarityFilterClicked();
    void onEnchantFilterClicked();

private:
    void setupUi();
    void updateRarityButtonText();
    void updateEnchantButtonText();
    QWidget* createRarityPopup();
    QWidget* createEnchantPopup();
    void updateRarityItemStyle(int index);

    QPushButton* m_rarityBtn = nullptr;
    QPushButton* m_enchantBtn = nullptr;

    QWidget* m_rarityPopup = nullptr;
    QWidget* m_enchantPopup = nullptr;
    QList<QPushButton*> m_rarityItems;

    QSet<int> m_selectedRarities;
    int m_enchantMin = 0;
    int m_enchantMax = 0;

    static QStringList rarityNames();
    static QStringList rarityColors();
};
