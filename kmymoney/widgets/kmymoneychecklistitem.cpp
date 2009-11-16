/***************************************************************************
                          kmymoneychecklistitem
                             -------------------
    begin                : Wed Jun 28 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneychecklistitem.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qfont.h>
#include <QPainter>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneylistviewitem.h"
#include "kmymoneyglobalsettings.h"

KMyMoneyCheckListItem::KMyMoneyCheckListItem(Q3ListView* parent, const QString& txt, const QString& key, const QString& id, Type type) :
  Q3CheckListItem(parent, txt, type),
  m_key(key),
  m_id(id),
  m_isOdd(0),
  m_isKnown(0)
{
  setOn(true);
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::KMyMoneyCheckListItem(Q3ListViewItem* parent, const QString& txt, const QString& key, const QString& id, Type type) :
  Q3CheckListItem(parent, txt, type),
  m_key(key),
  m_id(id),
  m_isOdd(0),
  m_isKnown(0)
{
  setOn(true);
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::KMyMoneyCheckListItem(Q3ListView* parent, Q3ListViewItem* after, const QString& txt, const QString& key, const QString& id, Type type) :
  Q3CheckListItem(parent, after, txt, type),
  m_key(key),
  m_id(id),
  m_isOdd(0),
  m_isKnown(0)
{
  setOn(true);
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::~KMyMoneyCheckListItem()
{
}

QString KMyMoneyCheckListItem::key(int column, bool ascending) const
{
  Q_UNUSED(ascending);

  if(column == 0)
    return m_key[0] + text(0);
  return m_key.mid(1);
}

void KMyMoneyCheckListItem::stateChange(bool state)
{
  emit stateChanged(state);
}

void KMyMoneyCheckListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());

  // write the groups in bold
  QFont f = p->font();
  f.setBold(!isSelectable());
  p->setFont(f);

  Q3CheckListItem::paintCell(p, _cg, column, width, alignment);
}

const QColor KMyMoneyCheckListItem::backgroundColor()
{
  return isAlternate() ? KMyMoneyGlobalSettings::listBGColor() : KMyMoneyGlobalSettings::listColor();
}

bool KMyMoneyCheckListItem::isAlternate(void)
{
// logic taken from K3ListViewItem::isAlternate()
  KMyMoneyCheckListItem* ciAbove;
  KMyMoneyListViewItem* liAbove;
  ciAbove = dynamic_cast<KMyMoneyCheckListItem*> (itemAbove());
  liAbove = dynamic_cast<KMyMoneyListViewItem*> (itemAbove());

  m_isKnown = ciAbove ? ciAbove->m_isKnown : (liAbove ? liAbove->m_isKnown : true);
  if(m_isKnown) {
    m_isOdd = ciAbove ? !ciAbove->m_isOdd : (liAbove ? !liAbove->m_isOdd : false);
  } else {
    KMyMoneyCheckListItem* clItem;
    KMyMoneyListViewItem* liItem;
    bool previous = true;
    if(Q3ListViewItem::parent()) {
      clItem = dynamic_cast<KMyMoneyCheckListItem *>(Q3ListViewItem::parent());
      liItem = dynamic_cast<KMyMoneyListViewItem*>(Q3ListViewItem::parent());
      if(clItem)
        previous = clItem->m_isOdd;
      else
        previous = liItem->m_isOdd;
      clItem = dynamic_cast<KMyMoneyCheckListItem *>(Q3ListViewItem::parent()->firstChild());
      liItem = dynamic_cast<KMyMoneyListViewItem*>(Q3ListViewItem::parent()->firstChild());
    } else {
      clItem = dynamic_cast<KMyMoneyCheckListItem *>(listView()->firstChild());
      liItem = dynamic_cast<KMyMoneyListViewItem*>(listView()->firstChild());
    }
    while(clItem || liItem) {
      if(clItem) {
        clItem->m_isOdd = previous = !previous;
        clItem->m_isKnown = true;
        liItem = dynamic_cast<KMyMoneyListViewItem *>(clItem->nextSibling());
        clItem = dynamic_cast<KMyMoneyCheckListItem *>(clItem->nextSibling());
      } else if(liItem) {
        liItem->m_isOdd = previous = !previous;
        liItem->m_isKnown = true;
        clItem = dynamic_cast<KMyMoneyCheckListItem *>(liItem->nextSibling());
        liItem = dynamic_cast<KMyMoneyListViewItem *>(liItem->nextSibling());
      }
    }
  }
  return m_isOdd;
}

#include "kmymoneychecklistitem.moc"
