/*  This file is part of the KDE project
    Copyright (C) 2009 Laurent Montel <montel@kde.org>

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

#include "transactionsortoptionimpl.h"
#include <kiconloader.h>
#include "register.h"
//#include "sortoptionlistitem.h"

TransactionSortOption::TransactionSortOption(QWidget *parent)
    : QWidget(parent)
{
  setupUi(this);
  init();
}


void TransactionSortOption::init()
{
  m_addButton->setIcon(KIcon("arrow-right"));
  m_removeButton->setIcon(KIcon("arrow-left"));
  m_upButton->setIcon(KIcon("arrow-up"));
  m_downButton->setIcon(KIcon("arrow-down"));

  // don't allow sorting of the selected entries
  m_selectedList->setSortingEnabled(false);

  setSettings(QString());

  // update UI when focus changes
  connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)),
          this, SLOT(slotFocusChanged(QWidget *, QWidget *)));
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
  m_availableList->clear();
  m_availableList->setCurrentItem(0);
  m_selectedList->clear();
  m_selectedList->setCurrentItem(0);

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
    if (abs(val) == static_cast<int>(KMyMoneyRegister::EntryDateSort)) {
      dateSign = (val < 0) ? -1 : 1;
      continue;
    }
    last = addEntry(m_selectedList, last, val);
  }

  // make sure to create EntryOrderSort if missing but required
  if (selectedMap.find(static_cast<int>(KMyMoneyRegister::EntryDateSort)) != selectedMap.end()
      && selectedMap.find(static_cast<int>(KMyMoneyRegister::EntryOrderSort)) == selectedMap.end()) {
    int val = dateSign * static_cast<int>(KMyMoneyRegister::EntryOrderSort);
    selectedMap[static_cast<int>(KMyMoneyRegister::EntryOrderSort)] = true;
    last = addEntry(m_selectedList, last, val);
  }

  // fill available list
  QMap<int, bool>::const_iterator it_m;
  for (int i = static_cast<int>(KMyMoneyRegister::PostDateSort);
       i < static_cast<int>(KMyMoneyRegister::MaxSortFields); ++i) {
    // Never add EntryDateSort
    if (i == static_cast<int>(KMyMoneyRegister::EntryDateSort))
      continue;
    // Only add those, that are not present in the list of selected items
    if (selectedMap.find(i) == selectedMap.end()) {
      int val = i;
      if (i == static_cast<int>(KMyMoneyRegister::ValueSort))
        val = -val;
      addEntry(m_availableList, 0, val);
    }
  }

  // update the current item on the lists
  QListWidgetItem* p;
  if ((p = m_availableList->item(0)) != 0) {
    m_availableList->setCurrentItem(p);
  }
  if ((p = m_selectedList->item(0)) != 0) {
    m_selectedList->setCurrentItem(p);
  }

  slotAvailableSelected();
}

QListWidgetItem* TransactionSortOption::addEntry(KListWidget* p, QListWidgetItem* after, int idx)
{
  QString txt = KMyMoneyRegister::sortOrderToText(static_cast<KMyMoneyRegister::TransactionSortField>(abs(idx)));
  if (txt.isEmpty())
    txt = "Unknown";    // i18n should be handled in sortOptionToText()

  int row = p->row(after) + 1;
  p->insertItem(row, txt);
  QListWidgetItem* item = p->item(row);
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
    item->setIcon(KIcon("view-sort-ascending"));
  } else {
    item->setIcon(KIcon("view-sort-descending"));
  }
}

QString TransactionSortOption::settings(void) const
{
  QString rc;
  QListWidgetItem* item = dynamic_cast<QListWidgetItem*>(m_selectedList->item(0));
  while (item) {
    int option = KMyMoneyRegister::textToSortOrder(item->text());
    // if we look at the EntryOrderSort option, we have to make
    // sure, that the EntryDateSort is prepended
    if (option == KMyMoneyRegister::EntryOrderSort) {
      rc  += QString::number(static_cast<int>(KMyMoneyRegister::EntryDateSort) * item->data(Qt::UserRole).toInt()) + ',';
    }
    rc += QString::number(KMyMoneyRegister::textToSortOrder(item->text()) * item->data(Qt::UserRole).toInt());
    item = m_selectedList->item(m_selectedList->row(item) + 1);
    if (item != 0)
      rc += ',';
  }
  return rc;
}

void TransactionSortOption::slotFocusChanged(QWidget *o, QWidget *n)
{
  Q_UNUSED(o);

  if (n == m_availableList)
    slotAvailableSelected();
  if (n == m_selectedList)
    slotSelectedSelected();
}

void TransactionSortOption::slotAvailableSelected()
{
  QListWidgetItem* item = m_availableList->currentItem();
  m_addButton->setEnabled(item != 0);
  m_removeButton->setDisabled(true);
  m_upButton->setDisabled(true);
  m_downButton->setDisabled(true);
}

void TransactionSortOption::slotSelectedSelected()
{
  QListWidgetItem* item = m_selectedList->currentItem();
  m_addButton->setDisabled(true);
  m_removeButton->setEnabled(item != 0);
  if (item) {
    m_upButton->setEnabled(m_selectedList->row(item) != 0);
    m_downButton->setEnabled(m_selectedList->row(item) < m_selectedList->count() - 1);
  } else {
    m_upButton->setEnabled(false);
    m_downButton->setEnabled(false);
  }
}

void TransactionSortOption::slotAddItem(void)
{
  QListWidgetItem* item;
  if ((item = m_availableList->currentItem()) != 0) {
    QListWidgetItem* next = m_availableList->item(m_availableList->row(item) + 1);
    if (!next)
      next = m_availableList->item(m_availableList->row(item) + 1);
    m_availableList->takeItem(m_availableList->row(item));
    m_selectedList->addItem(item);
    m_addButton->setEnabled((m_availableList->count() > 0));
    if (next) {
      m_availableList->setCurrentItem(next);
    }
    emit settingsChanged(settings());
  }
}

void TransactionSortOption::slotRemoveItem(void)
{
  QListWidgetItem* item;
  if ((item = m_selectedList->currentItem()) != 0) {
    QListWidgetItem* next = m_selectedList->item(m_selectedList->row(item) + 1);
    if (!next)
      next = m_selectedList->item(m_selectedList->row(item) + 1);
    m_selectedList->takeItem(m_selectedList->row(item));
    m_availableList->addItem(item);
    m_removeButton->setEnabled(m_selectedList->count() > 0);
    if (next) {
      m_selectedList->setCurrentItem(next);
    }
    emit settingsChanged(settings());
  }
}

void TransactionSortOption::slotUpItem(void)
{
  QListWidgetItem *item = m_selectedList->currentItem();
  QListWidgetItem *prev = m_selectedList->item(m_selectedList->row(item) - 1);
  int prevRow = m_selectedList->row(prev);
  if (prev) {
    m_selectedList->takeItem(m_selectedList->row(item));
    m_selectedList->insertItem(prevRow, item);
    m_selectedList->setCurrentRow(m_selectedList->row(item));
    m_upButton->setEnabled(m_selectedList->row(item) > 0);
    m_downButton->setEnabled(m_selectedList->row(item) < m_selectedList->count() - 1);
    emit settingsChanged(settings());
  }
}

void TransactionSortOption::slotDownItem(void)
{
  QListWidgetItem *item = m_selectedList->currentItem();
  QListWidgetItem *next = m_selectedList->item(m_selectedList->row(item) + 1);
  int nextRow = m_selectedList->row(next);
  if (next) {
    m_selectedList->takeItem(m_selectedList->row(item));
    m_selectedList->insertItem(nextRow, item);
    m_selectedList->setCurrentRow(m_selectedList->row(item));
    m_upButton->setEnabled(m_selectedList->row(item) > 0);
    m_downButton->setEnabled(m_selectedList->row(item) < m_selectedList->count() - 1);
    emit settingsChanged(settings());
  }
}
