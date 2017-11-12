/*  This file is part of the KDE project
    Copyright (C) 2009 Laurent Montel <montel@kde.org>
    (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "transactionsortoption.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons/icons.h"
#include "widgetenums.h"

#include "ui_transactionsortoption.h"

using namespace eWidgets;
using namespace Icons;

static const char * sortOrderText[] = {
  I18N_NOOP2("Unknown sort order", "Unknown"),
  I18N_NOOP("Post date"),
  I18N_NOOP("Date entered"),
  I18N_NOOP("Payee"),
  I18N_NOOP("Amount"),
  I18N_NOOP("Number"),
  I18N_NOOP("Entry order"),
  I18N_NOOP("Type"),
  I18N_NOOP("Category"),
  I18N_NOOP("Reconcile state"),
  I18N_NOOP("Security")
  // add new values above this comment line
};

TransactionSortOption::TransactionSortOption(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TransactionSortOption)
{
  ui->setupUi(this);

  ui->m_addButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ArrowRight]));
  ui->m_removeButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ArrowLeft]));
  ui->m_upButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ArrowUp]));
  ui->m_downButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ArrowDown]));

  // don't allow sorting of the selected entries
  ui->m_selectedList->setSortingEnabled(false);

  setSettings(QString());

  // update UI when focus changes
  connect(qApp, &QApplication::focusChanged,
          this, &TransactionSortOption::slotFocusChanged);
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

  QStringList list = settings.split(',', QString::SkipEmptyParts);
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
  QMap<int, bool>::const_iterator it_m;
  for (int i = static_cast<int>(SortField::PostDate);
       i < static_cast<int>(SortField::MaxFields); ++i) {
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

  slotAvailableSelected();
}

QListWidgetItem* TransactionSortOption::addEntry(QListWidget* p, QListWidgetItem* after, int idx)
{
  auto txt = sortOrderToText(static_cast<SortField>(abs(idx)));
  if (txt.isEmpty())
    txt = "Unknown";    // i18n should be handled in sortOptionToText()

  int row = p->row(after) + 1;
  p->insertItem(row, txt);
  auto item = p->item(row);
  int direction = (idx >= 0) ? 1 : -1;
  item->setData(Qt::UserRole, QVariant(direction));
  setDirectionIcon(item);
  return item;
}



void TransactionSortOption::toggleDirection(QListWidgetItem* item)
{
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
    item->setIcon(QIcon::fromTheme(g_Icons[Icon::SortAscending]));
  } else {
    item->setIcon(QIcon::fromTheme(g_Icons[Icon::SortDescending]));
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
      rc  += QString::number(static_cast<int>(SortField::EntryDate) * item->data(Qt::UserRole).toInt()) + ',';
    }
    rc += QString::number((int)textToSortOrder(item->text()) * item->data(Qt::UserRole).toInt());
    item = ui->m_selectedList->item(ui->m_selectedList->row(item) + 1);
    if (item != 0)
      rc += ',';
  }
  return rc;
}

void TransactionSortOption::slotFocusChanged(QWidget *o, QWidget *n)
{
  Q_UNUSED(o);

  if (n == ui->m_availableList)
    slotAvailableSelected();
  if (n == ui->m_selectedList)
    slotSelectedSelected();
}

void TransactionSortOption::slotAvailableSelected()
{
  auto item = ui->m_availableList->currentItem();
  ui->m_addButton->setEnabled(item != 0);
  ui->m_removeButton->setDisabled(true);
  ui->m_upButton->setDisabled(true);
  ui->m_downButton->setDisabled(true);
}

void TransactionSortOption::slotSelectedSelected()
{
  auto item = ui->m_selectedList->currentItem();
  ui->m_addButton->setDisabled(true);
  ui->m_removeButton->setEnabled(item != 0);
  if (item) {
    ui->m_upButton->setEnabled(ui->m_selectedList->row(item) != 0);
    ui->m_downButton->setEnabled(ui->m_selectedList->row(item) < ui->m_selectedList->count() - 1);
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
    if (text == i18n(sortOrderText[idx])) {
      return static_cast<SortField>(idx);
    }
  }
  return SortField::Unknown;
}

QString TransactionSortOption::sortOrderToText(SortField idx)
{
  if ((int)idx < (int)SortField::PostDate || (int)idx >= (int)SortField::MaxFields)
    idx = SortField::Unknown;
  return i18n(sortOrderText[(int)idx]);
}
