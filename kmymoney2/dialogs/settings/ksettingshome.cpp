/***************************************************************************
                             ksettingshome.cpp
                             --------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <q3header.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>
#include <kpushbutton.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ktextedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ksettingshome.h"
#include "kmymoney2/kmymoneyglobalsettings.h"
#include "kmymoney2/kmymoneyutils.h"

KSettingsHome::KSettingsHome(QWidget* parent) :
  KSettingsHomeDecl(parent),
  m_noNeedToUpdateList(false)
{
  m_homePageList->addColumn("");
  m_homePageList->setSorting(-1);
  m_homePageList->header()->hide();
  m_homePageList->setAllColumnsShowFocus(true);

  KIconLoader* il = KIconLoader::global();
  KGuiItem upButtonItem( i18n( "&Up" ),
                    KIcon(il->loadIcon("up", KIconLoader::Small, KIconLoader::SizeSmall)),
                    i18n("Move selected item up"),
                    i18n("Use this to move the selected item up by one position in the list."));
  KGuiItem downButtonItem( i18n( "&Down" ),
                    KIcon(il->loadIcon("down", KIconLoader::Small, KIconLoader::SizeSmall)),
                    i18n("Move selected item down"),
                    i18n("Use this to move the selected item down by one position in the list."));

  m_upButton->setGuiItem(upButtonItem);
  m_upButton->setEnabled(false);
  m_downButton->setGuiItem(downButtonItem);
  m_downButton->setEnabled(false);

  // connect this, so that the list gets loaded once the edit field is filled
  connect(kcfg_ItemList, SIGNAL(textChanged()), this, SLOT(slotLoadItems()));

  connect(m_homePageList, SIGNAL(selectionChanged(Q3ListViewItem*)),
          this, SLOT(slotSelectHomePageItem(Q3ListViewItem *)));
  connect(m_homePageList, SIGNAL(pressed(Q3ListViewItem*)), this, SLOT(slotUpdateItemList()));

  connect(m_upButton, SIGNAL(clicked()), this, SLOT(slotMoveUp()));
  connect(m_downButton, SIGNAL(clicked()), this, SLOT(slotMoveDown()));

  // Don't show it to the user, we only need it to load and save the settings
  kcfg_ItemList->hide();
}

KSettingsHome::~KSettingsHome()
{
}

void KSettingsHome::slotLoadItems(void)
{
  if(m_noNeedToUpdateList)
    return;

  QStringList list = KMyMoneyGlobalSettings::itemList();
  QStringList::ConstIterator it;
  int w = 0;
  m_homePageList->clear();
  Q3CheckListItem *sel = 0;

  QFontMetrics fm( KGlobalSettings::generalFont());
  Q3CheckListItem* last = 0;

  for(it = list.begin(); it != list.end(); ++it) {
    int idx = (*it).toInt();
    // skip over unknown item entries
    if(idx == 0)
      continue;
    bool enabled = idx > 0;
    if(!enabled) idx = -idx;
    Q3CheckListItem* item = new Q3CheckListItem(m_homePageList, KMyMoneyUtils::homePageItemToString(idx), Q3CheckListItem::CheckBox);
    if(last)
      item->moveItem(last);

    // qDebug("Adding %s", item->text(0).toLatin1());
    item->setOn(enabled);
    if(item->width(fm, m_homePageList, 0) > w)
      w = item->width(fm, m_homePageList, 0);

    if(sel == 0)
      sel = item;
    last = item;
  }

  if(sel) {
    m_homePageList->setSelected(sel, true);
    slotSelectHomePageItem(sel);
  }
}

void KSettingsHome::slotUpdateItemList(void)
{
  QString list;
  Q3ListViewItem *it;

  for(it = m_homePageList->firstChild(); it; ) {
    int item = KMyMoneyUtils::stringToHomePageItem(it->text(0));
    if(!(static_cast<Q3CheckListItem*>(it)->isOn()))
      item = -item;
    list += QString::number(item);
    it = it->nextSibling();
    if(it)
      list += ",";
  }

  // don't update the list
  m_noNeedToUpdateList = true;
  kcfg_ItemList->setText(list);
  m_noNeedToUpdateList = false;
}

void KSettingsHome::slotSelectHomePageItem(Q3ListViewItem *item)
{
  m_upButton->setEnabled(m_homePageList->firstChild() != item);
  m_downButton->setEnabled(item->nextSibling());
}

void KSettingsHome::slotMoveUp(void)
{
  Q3ListViewItem *item = m_homePageList->currentItem();
  Q3ListViewItem *prev = item->itemAbove();
  if(prev) {
    prev->moveItem(item);
    slotSelectHomePageItem(item);
    slotUpdateItemList();
  }
}

void KSettingsHome::slotMoveDown(void)
{
  Q3ListViewItem *item = m_homePageList->currentItem();
  Q3ListViewItem *next = item->nextSibling();
  if(next) {
    item->moveItem(next);
    slotSelectHomePageItem(item);
    slotUpdateItemList();
  }
}

#include "ksettingshome.moc"
