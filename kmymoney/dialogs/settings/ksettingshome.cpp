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

#include "ksettingshome.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney/kmymoneyglobalsettings.h"
#include "kmymoney/kmymoneyutils.h"
#include "icons/icons.h"

using namespace Icons;

KSettingsHome::KSettingsHome(QWidget* parent) :
    KSettingsHomeDecl(parent),
    m_noNeedToUpdateList(false)
{
  m_homePageList->setSortingEnabled(false);

  KGuiItem upButtonItem(i18nc("Move item up",  "&Up"),
                        QIcon::fromTheme(g_Icons[Icon::ArrowUp]),
                        i18n("Move selected item up"),
                        i18n("Use this to move the selected item up by one position in the list."));
  KGuiItem downButtonItem(i18n("&Down"),
                          QIcon::fromTheme(g_Icons[Icon::ArrowDown]),
                          i18n("Move selected item down"),
                          i18n("Use this to move the selected item down by one position in the list."));

  KGuiItem::assign(m_upButton, upButtonItem);
  m_upButton->setEnabled(false);
  KGuiItem::assign(m_downButton, downButtonItem);
  m_downButton->setEnabled(false);

  // connect this, so that the list gets loaded once the edit field is filled
  connect(kcfg_ItemList, SIGNAL(textChanged(QString)), this, SLOT(slotLoadItems()));

  connect(m_homePageList, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotSelectHomePageItem()));
  connect(m_homePageList, SIGNAL(clicked(QModelIndex)), this, SLOT(slotUpdateItemList()));

  connect(m_upButton, SIGNAL(clicked()), this, SLOT(slotMoveUp()));
  connect(m_downButton, SIGNAL(clicked()), this, SLOT(slotMoveDown()));

  // Don't show it to the user, we only need it to load and save the settings
  kcfg_ItemList->hide();
}

KSettingsHome::~KSettingsHome()
{
}

void KSettingsHome::slotLoadItems()
{
  if (m_noNeedToUpdateList)
    return;

  QStringList list = KMyMoneyGlobalSettings::itemList();
  QStringList::ConstIterator it;
  m_homePageList->clear();
  QListWidgetItem *sel = 0;

  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    int idx = (*it).toInt();
    // skip over unknown item entries
    if (idx == 0)
      continue;
    bool enabled = idx > 0;
    if (!enabled) idx = -idx;
    QListWidgetItem* item = new QListWidgetItem(m_homePageList);
    item->setText(KMyMoneyUtils::homePageItemToString(idx));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

    // qDebug("Adding %s", item->text(0).toLatin1());
    if (enabled) {
      item->setCheckState(Qt::Checked);
    } else {
      item->setCheckState(Qt::Unchecked);
    }

    if (sel == 0)
      sel = item;
  }

  if (sel) {
    m_homePageList->setCurrentItem(sel);
    slotSelectHomePageItem();
  }
}

void KSettingsHome::slotUpdateItemList()
{
  QString list;
  QListWidgetItem *it;

  for (it = m_homePageList->item(0); it;) {
    int item = KMyMoneyUtils::stringToHomePageItem(it->text());
    if (it->checkState() == Qt::Unchecked)
      item = -item;
    list += QString::number(item);
    if (m_homePageList->count() > (m_homePageList->row(it) + 1)) {
      it = m_homePageList->item(m_homePageList->row(it) + 1);
      if (it) {
        list += ',';
      }
    } else {
      break;
    }
  }

  // don't update the list
  m_noNeedToUpdateList = true;
  kcfg_ItemList->setText(list);
  m_noNeedToUpdateList = false;
}

void KSettingsHome::slotSelectHomePageItem()
{
  QListWidgetItem* item = m_homePageList->currentItem();
  m_upButton->setEnabled(m_homePageList->item(0) != item);
  m_downButton->setEnabled(m_homePageList->count() > (m_homePageList->row(item) + 1));
}

void KSettingsHome::slotMoveUp()
{
  QListWidgetItem *item = m_homePageList->currentItem();
  QListWidgetItem *prev = m_homePageList->item(m_homePageList->row(item) - 1);
  int prevRow = m_homePageList->row(prev);
  if (prev) {
    m_homePageList->takeItem(m_homePageList->row(item));
    m_homePageList->insertItem(prevRow, item);
    m_homePageList->setCurrentRow(m_homePageList->row(item));
    slotSelectHomePageItem();
    slotUpdateItemList();
  }
}

void KSettingsHome::slotMoveDown()
{
  QListWidgetItem *item = m_homePageList->currentItem();
  QListWidgetItem *next = m_homePageList->item(m_homePageList->row(item) + 1);
  int nextRow = m_homePageList->row(next);
  if (next) {
    m_homePageList->takeItem(m_homePageList->row(item));
    m_homePageList->insertItem(nextRow, item);
    m_homePageList->setCurrentRow(m_homePageList->row(item));
    slotSelectHomePageItem();
    slotUpdateItemList();
  }
}
