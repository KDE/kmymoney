/*
    SPDX-FileCopyrightText: 2009-2011 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "transactionsortoption.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes
#include <KLazyLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons/icons.h"
#include "widgetenums.h"

#include "ui_transactionsortoption.h"

using namespace eWidgets;
using namespace Icons;

static constexpr KLazyLocalizedString sortOrderText[] = {
    kli18nc("Unknown sort order", "Unknown"),
    kli18n("Post date"),
    kli18n("Date entered"),
    kli18n("Payee"),
    kli18n("Amount"),
    kli18n("Number"),
    kli18n("Entry order"),
    kli18n("Type"),
    kli18n("Category"),
    kli18n("Reconcile state"),
    kli18n("Security"),
    // add new values above this comment line
};

TransactionSortOption::TransactionSortOption(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TransactionSortOption)
{
    ui->setupUi(this);

    ui->m_addButton->setIcon(Icons::get(Icon::ArrowRight));
    ui->m_removeButton->setIcon(Icons::get(Icon::ArrowLeft));
    ui->m_upButton->setIcon(Icons::get(Icon::ArrowUp));
    ui->m_downButton->setIcon(Icons::get(Icon::ArrowDown));

    // don't allow sorting of the selected entries
    ui->m_selectedList->setSortingEnabled(false);

    connect(ui->m_addButton, &QPushButton::pressed, this, &TransactionSortOption::slotAddItem);
    connect(ui->m_removeButton, &QPushButton::pressed, this, &TransactionSortOption::slotRemoveItem);
    connect(ui->m_upButton, &QPushButton::pressed, this, &TransactionSortOption::slotUpItem);
    connect(ui->m_downButton, &QPushButton::pressed, this, &TransactionSortOption::slotDownItem);

    connect(ui->m_availableList, &QListWidget::currentItemChanged, this, &TransactionSortOption::slotUpdateButtons);
    connect(ui->m_selectedList, &QListWidget::currentItemChanged, this, &TransactionSortOption::slotUpdateButtons);
    connect(ui->m_selectedList, &QListWidget::doubleClicked, this, &TransactionSortOption::slotToggleDirection);

    setSettings(QString());
}

TransactionSortOption::~TransactionSortOption()
{
    delete ui;
}

/**
 * Setup the two lists according to the elements found in @a list.
 * If an item is negative, it will show up in the available list,
 * if positive, it shows up in the selected list.
 *
 * Special care is taken about the two values @a EntryDateSort and
 * @a EntryOrderSort. These two entries cannot (should not) exist
 * alone. Inside this widget, only the @a EntryOrderSort is used.
 *
 * setSettings() takes care of hiding the @a EntryDateSort item and if
 * it exists in @p settings without @a EntryOrderSort being present, it
 * will add @a EntryOrderSort.
 */
void TransactionSortOption::setSettings(const QString& settings)
{
    ui->m_availableList->clear();
    ui->m_availableList->setCurrentItem(0);
    ui->m_selectedList->clear();
    ui->m_selectedList->setCurrentItem(0);

    QStringList list = settings.split(',', Qt::SkipEmptyParts);
    QMap<int, bool> selectedMap;

    // fill selected list
    QStringList::const_iterator it_s;
    QListWidgetItem* last = 0;
    int dateSign = 1;
    for (it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
        int val = (*it_s).toInt();
        selectedMap[abs(val)] = true;
        // skip EntryDateSort but keep sign
        if (abs(val) == static_cast<int>(SortField::EntryDate)) {
            dateSign = (val < 0) ? -1 : 1;
            continue;
        }
        last = addEntry(ui->m_selectedList, last, val);
    }

    // make sure to create EntryOrderSort if missing but required
    if (selectedMap.find(static_cast<int>(SortField::EntryDate)) != selectedMap.end()
        && selectedMap.find(static_cast<int>(SortField::EntryOrder)) == selectedMap.end()) {
        int val = dateSign * static_cast<int>(SortField::EntryOrder);
        selectedMap[static_cast<int>(SortField::EntryOrder)] = true;
        last = addEntry(ui->m_selectedList, last, val);
    }

    // fill available list
    for (int i = static_cast<int>(SortField::PostDate); i < static_cast<int>(SortField::MaxFields); ++i) {
        // Never add EntryDateSort
        if (i == static_cast<int>(SortField::EntryDate))
            continue;
        // Only add those, that are not present in the list of selected items
        if (selectedMap.find(i) == selectedMap.end()) {
            int val = i;
            if (i == static_cast<int>(SortField::Value))
                val = -val;
            addEntry(ui->m_availableList, 0, val);
        }
    }

    // update the current item on the lists
    QListWidgetItem* p;
    if ((p = ui->m_availableList->item(0)) != 0) {
        ui->m_availableList->setCurrentItem(p);
    }
    if ((p = ui->m_selectedList->item(0)) != 0) {
        ui->m_selectedList->setCurrentItem(p);
    }

    slotUpdateButtons();
}

QListWidgetItem* TransactionSortOption::addEntry(QListWidget* p, QListWidgetItem* after, int idx)
{
    auto txt = sortOrderToText(static_cast<SortField>(abs(idx)));
    if (txt.isEmpty())
        txt = "Unknown"; // i18n should be handled in sortOptionToText()

    int row = p->row(after) + 1;
    p->insertItem(row, txt);
    auto item = p->item(row);
    int direction = (idx >= 0) ? 1 : -1;
    item->setData(Qt::UserRole, QVariant(direction));
    setDirectionIcon(item);
    return item;
}

void TransactionSortOption::slotToggleDirection()
{
    const auto item = ui->m_selectedList->currentItem();
    if (item) {
        int direction = item->data(Qt::UserRole).toInt() * (-1);
        item->setData(Qt::UserRole, QVariant(direction));
        setDirectionIcon(item);
        emit settingsChanged(settings());
    }
}

void TransactionSortOption::setDirectionIcon(QListWidgetItem* item)
{
    if (item->data(Qt::UserRole).toInt() > 0) {
        item->setIcon(Icons::get(Icon::SortAscending));
    } else {
        item->setIcon(Icons::get(Icon::SortDescending));
    }
}

QString TransactionSortOption::settings() const
{
    QString rc;
    auto item = dynamic_cast<QListWidgetItem*>(ui->m_selectedList->item(0));
    while (item) {
        auto option = textToSortOrder(item->text());
        // if we look at the EntryOrderSort option, we have to make
        // sure, that the EntryDateSort is prepended
        if (option == SortField::EntryOrder) {
            rc += QString::number(static_cast<int>(SortField::EntryDate) * item->data(Qt::UserRole).toInt()) + ',';
        }
        rc += QString::number((int)textToSortOrder(item->text()) * item->data(Qt::UserRole).toInt());
        item = ui->m_selectedList->item(ui->m_selectedList->row(item) + 1);
        if (item != 0)
            rc += ',';
    }
    return rc;
}

void TransactionSortOption::slotUpdateButtons()
{
    auto availableItem = ui->m_availableList->currentItem();
    auto selectedItem = ui->m_selectedList->currentItem();
    ui->m_addButton->setEnabled(availableItem != 0);
    ui->m_removeButton->setEnabled(selectedItem != 0);
    if (selectedItem) {
        ui->m_upButton->setEnabled(ui->m_selectedList->row(selectedItem) != 0);
        ui->m_downButton->setEnabled(ui->m_selectedList->row(selectedItem) < ui->m_selectedList->count() - 1);
    } else {
        ui->m_upButton->setEnabled(false);
        ui->m_downButton->setEnabled(false);
    }
}

void TransactionSortOption::slotAddItem()
{
    QListWidgetItem* item;
    if ((item = ui->m_availableList->currentItem()) != 0) {
        auto next = ui->m_availableList->item(ui->m_availableList->row(item) + 1);
        if (!next)
            next = ui->m_availableList->item(ui->m_availableList->row(item) + 1);
        ui->m_availableList->takeItem(ui->m_availableList->row(item));
        ui->m_selectedList->addItem(item);
        ui->m_addButton->setEnabled((ui->m_availableList->count() > 0));
        if (next) {
            ui->m_availableList->setCurrentItem(next);
        }
        emit settingsChanged(settings());
    }
}

void TransactionSortOption::slotRemoveItem()
{
    QListWidgetItem* item;
    if ((item = ui->m_selectedList->currentItem()) != 0) {
        auto next = ui->m_selectedList->item(ui->m_selectedList->row(item) + 1);
        if (!next)
            next = ui->m_selectedList->item(ui->m_selectedList->row(item) + 1);
        ui->m_selectedList->takeItem(ui->m_selectedList->row(item));
        ui->m_availableList->addItem(item);
        ui->m_removeButton->setEnabled(ui->m_selectedList->count() > 0);
        if (next) {
            ui->m_selectedList->setCurrentItem(next);
        }
        emit settingsChanged(settings());
    }
}

void TransactionSortOption::slotUpItem()
{
    auto item = ui->m_selectedList->currentItem();
    auto prev = ui->m_selectedList->item(ui->m_selectedList->row(item) - 1);
    int prevRow = ui->m_selectedList->row(prev);
    if (prev) {
        ui->m_selectedList->takeItem(ui->m_selectedList->row(item));
        ui->m_selectedList->insertItem(prevRow, item);
        ui->m_selectedList->setCurrentRow(ui->m_selectedList->row(item));
        ui->m_upButton->setEnabled(ui->m_selectedList->row(item) > 0);
        ui->m_downButton->setEnabled(ui->m_selectedList->row(item) < ui->m_selectedList->count() - 1);
        emit settingsChanged(settings());
    }
}

void TransactionSortOption::slotDownItem()
{
    auto item = ui->m_selectedList->currentItem();
    auto next = ui->m_selectedList->item(ui->m_selectedList->row(item) + 1);
    int nextRow = ui->m_selectedList->row(next);
    if (next) {
        ui->m_selectedList->takeItem(ui->m_selectedList->row(item));
        ui->m_selectedList->insertItem(nextRow, item);
        ui->m_selectedList->setCurrentRow(ui->m_selectedList->row(item));
        ui->m_upButton->setEnabled(ui->m_selectedList->row(item) > 0);
        ui->m_downButton->setEnabled(ui->m_selectedList->row(item) < ui->m_selectedList->count() - 1);
        emit settingsChanged(settings());
    }
}

SortField TransactionSortOption::textToSortOrder(const QString& text)
{
    for (auto idx = 1; idx < static_cast<int>(SortField::MaxFields); ++idx) {
        if (text == sortOrderText[idx].toString()) {
            return static_cast<SortField>(idx);
        }
    }
    return SortField::Unknown;
}

QString TransactionSortOption::sortOrderToText(SortField idx)
{
    if ((int)idx < (int)SortField::PostDate || (int)idx >= (int)SortField::MaxFields)
        idx = SortField::Unknown;
    return sortOrderText[(int)idx].toString();
}
