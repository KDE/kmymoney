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
#include "sortoptionlistitem.h"

TransactionSortOption::TransactionSortOption( QWidget *parent )
    :TransactionSortOptionDecl( parent )
{
    init();
}


void TransactionSortOption::init()
{
  KIconLoader* il = KIconLoader::global();
  m_addButton->setIconSet(KIcon(il->loadIcon("arrow-right", KIconLoader::Small, KIconLoader::SizeSmall)));
  m_removeButton->setIconSet(QIcon(il->loadIcon("arrow-left", KIconLoader::Small, KIconLoader::SizeSmall)));
  m_upButton->setIconSet(QIcon(il->loadIcon("arrow-up", KIconLoader::Small, KIconLoader::SizeSmall)));
  m_downButton->setIconSet(QIcon(il->loadIcon("arrow-down", KIconLoader::Small, KIconLoader::SizeSmall)));

  // don't allow sorting of the selected entries
  m_selectedList->setSortColumn(-1);

  // defaults to "post date, value" sorting
  // setSettings(QString("1,4"));
  setSettings(QString());

  Q3ListViewItem* p;
  if((p = m_availableList->firstChild()) != 0) {
    m_availableList->setSelected(p, true);
  }
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
  m_selectedList->clear();

  QStringList list = QStringList::split(',', settings);
  QMap<int, bool> selectedMap;

  // fill selected list
  QStringList::const_iterator it_s;
  Q3ListViewItem* last = 0;
  int dateSign = 1;
  for(it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
    int val = (*it_s).toInt();
    selectedMap[abs(val)] = true;
    // skip EntryDateSort but keep sign
    if(abs(val) == static_cast<int>(KMyMoneyRegister::EntryDateSort)) {
      dateSign = (val < 0) ? -1 : 1;
      continue;
    }
    last = addEntry(m_selectedList, last, val);
  }

  // make sure to create EntryOrderSort if missing but required
  if(selectedMap.find(static_cast<int>(KMyMoneyRegister::EntryDateSort)) != selectedMap.end()
  && selectedMap.find(static_cast<int>(KMyMoneyRegister::EntryOrderSort)) == selectedMap.end()) {
    int val = dateSign * static_cast<int>(KMyMoneyRegister::EntryOrderSort);
    selectedMap[static_cast<int>(KMyMoneyRegister::EntryOrderSort)] = true;
    last = addEntry(m_selectedList, last, val);
  }

  // fill available list
  QMap<int, bool>::const_iterator it_m;
  for(int i = static_cast<int>(KMyMoneyRegister::PostDateSort);
      i < static_cast<int>(KMyMoneyRegister::MaxSortFields); ++i) {
    // Never add EntryDateSort
    if(i == static_cast<int>(KMyMoneyRegister::EntryDateSort))
      continue;
    // Only add those, that are not present in the list of selected items
    if(selectedMap.find(i) == selectedMap.end()) {
      int val = i;
      if(i == static_cast<int>(KMyMoneyRegister::ValueSort))
        val = -val;
      addEntry(m_availableList, 0, val);
    }
  }
}

Q3ListViewItem* TransactionSortOption::addEntry( K3ListView * p, Q3ListViewItem* after, int idx )
{
  QString txt = KMyMoneyRegister::sortOrderToText(static_cast<KMyMoneyRegister::TransactionSortField>(abs(idx)));
  if(txt.isEmpty())
    txt = "Unknown";    // i18n should be handled in sortOptionToText()

  return new SortOptionListItem(p, after, txt, idx);
}

void TransactionSortOption::toggleDirection(Q3ListViewItem* item)
{
  SortOptionListItem* p = dynamic_cast<SortOptionListItem*>(item);
  if(p) {
    p->toggleDirection();
    emit settingsChanged(settings());
  }
}

QString TransactionSortOption::settings( void ) const
{
  QString rc;
  SortOptionListItem* item = dynamic_cast<SortOptionListItem*>(m_selectedList->firstChild());
  while(item) {
    int option = KMyMoneyRegister::textToSortOrder(item->text(0));
    // if we look at the EntryOrderSort option, we have to make
    // sure, that the EntryDateSort is prepended
    if(option == KMyMoneyRegister::EntryOrderSort) {
      rc  += QString::number(static_cast<int>(KMyMoneyRegister::EntryDateSort)*item->direction())+",";
    }
    rc += QString::number(KMyMoneyRegister::textToSortOrder(item->text(0))*item->direction());
    item = dynamic_cast<SortOptionListItem*>(item->itemBelow());
    if(item != 0)
      rc += ",";
  }
  return rc;
}

void TransactionSortOption::slotAvailableSelected( Q3ListViewItem * item )
{
  m_addButton->setEnabled(item != 0);
  m_removeButton->setDisabled(true);
  m_upButton->setDisabled(true);
  m_downButton->setDisabled(true);

  Q3ListViewItem* p = m_selectedList->currentItem();
  if(p) {
    m_selectedList->setSelected(p, false);
  }
}

void TransactionSortOption::slotSelectedSelected( Q3ListViewItem * item )
{
  m_addButton->setDisabled(true);
  m_removeButton->setEnabled(item != 0);
  if(item) {
    m_upButton->setEnabled(item->itemAbove() != 0);
    m_downButton->setEnabled(item->itemBelow() != 0);
  } else {
    m_upButton->setEnabled(false);
    m_downButton->setEnabled(false);
  }

  Q3ListViewItem* p = m_availableList->currentItem();
  if(p) {
    m_availableList->setSelected(p, false);
  }
}

void TransactionSortOption::slotAddItem( void )
{
  Q3ListViewItem* item;
  if((item = m_availableList->currentItem()) != 0) {
    Q3ListViewItem* next = item->itemBelow();
    if(!next)
      next = item->itemAbove();
    m_availableList->takeItem(item);
    m_selectedList->insertItem(item);
    m_addButton->setEnabled(m_availableList->firstChild() != 0);
    if(next) {
      m_availableList->setCurrentItem(next);
      m_availableList->setSelected(next, true);
    }
    emit settingsChanged(settings());
  }
}

void TransactionSortOption::slotRemoveItem( void )
{
  Q3ListViewItem* item;
  if((item = m_selectedList->currentItem()) != 0) {
    Q3ListViewItem* next = item->itemBelow();
    if(!next)
      next = item->itemAbove();
    m_selectedList->takeItem(item);
    m_availableList->insertItem(item);
    m_removeButton->setEnabled(m_selectedList->firstChild() != 0);
    if(next) {
      m_selectedList->setCurrentItem(next);
      m_selectedList->setSelected(next, true);
    }
    emit settingsChanged(settings());
  }
}

void TransactionSortOption::slotUpItem( void )
{
  Q3ListViewItem* item;
  if((item = m_selectedList->currentItem()) != 0) {
    Q3ListViewItem* prev = item->itemAbove();
    if(prev) {
      prev->moveItem(item);
      m_selectedList->setCurrentItem(item);
      m_selectedList->setSelected(item, true);
      m_upButton->setEnabled(item->itemAbove() != 0);
      m_downButton->setEnabled(item->itemBelow() != 0);
      emit settingsChanged(settings());
    }
  }
}

void TransactionSortOption::slotDownItem( void )
{
  Q3ListViewItem* item;
  if((item = m_selectedList->currentItem()) != 0) {
    Q3ListViewItem* next = item->itemBelow();
    if(next) {
      item->moveItem(next);
      m_selectedList->setCurrentItem(item);
      m_selectedList->setSelected(item, true);
      m_upButton->setEnabled(item->itemAbove() != 0);
      m_downButton->setEnabled(item->itemBelow() != 0);
      emit settingsChanged(settings());
    }
  }
}
