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

#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneyCheckListItem::KMyMoneyCheckListItem(QTreeWidget* parent, const QString& txt, const QString& key, const QString& id) :
    QTreeWidgetItem(parent, QStringList(txt)),
    m_key(key),
    m_id(id)
{
  setCheckState(0,Qt::Checked);
  if (key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::KMyMoneyCheckListItem(QTreeWidgetItem* parent, const QString& txt, const QString& key, const QString& id) :
    QTreeWidgetItem(parent, QStringList(txt)),
    m_key(key),
    m_id(id)
{
  setCheckState(0,Qt::Checked);
  if (key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::~KMyMoneyCheckListItem()
{
}

QString KMyMoneyCheckListItem::key(int column, bool ascending) const
{
  Q_UNUSED(ascending);

  if (column == 0)
    return m_key[0] + text(0);
  return m_key.mid(1);
}

void KMyMoneyCheckListItem::stateChange(bool state)
{
  emit stateChanged(state);
}

#include "kmymoneychecklistitem.moc"
