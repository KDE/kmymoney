/***************************************************************************
                          kmymoneylistviewitem  -  description
                             -------------------
    begin                : Wed Jun 28 2006
    copyright            : (C) 2000-2006 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneylistviewitem.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPalette>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneychecklistitem.h"
#include "kmymoneyglobalsettings.h"

KMyMoneyListViewItem::KMyMoneyListViewItem(Q3ListView* parent, const QString& txt, const QString& key, const QString& id) :
  K3ListViewItem(parent, txt),
  m_key(key),
  m_id(id),
  m_isOdd(0),
  m_isKnown(0)
{
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyListViewItem::KMyMoneyListViewItem(Q3ListViewItem* parent, const QString& txt, const QString& key, const QString& id) :
  K3ListViewItem(parent, txt),
  m_key(key),
  m_id(id),
  m_isOdd(0),
  m_isKnown(0)
{
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyListViewItem::~KMyMoneyListViewItem()
{
}

QString KMyMoneyListViewItem::key(int column, bool ascending) const
{
  Q_UNUSED(ascending);

  if(column == 0)
    return m_key.isEmpty() ? text(0) : (m_key[0] + text(0));
  return m_key.isEmpty() ? QString() : m_key.mid(1);
}


void KMyMoneyListViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());

  // make sure to bypass K3ListViewItem::paintCell() as
  // we don't like it's logic - that's why we do this
  // here ;-)    (ipwizard)
  Q3ListViewItem::paintCell(p, _cg, column, width, alignment);
}

const QColor KMyMoneyListViewItem::backgroundColor()
{
  return isAlternate() ? KMyMoneyGlobalSettings::listBGColor() : KMyMoneyGlobalSettings::listColor();
}

bool KMyMoneyListViewItem::isAlternate(void)
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

#include "kmymoneylistviewitem.moc"
