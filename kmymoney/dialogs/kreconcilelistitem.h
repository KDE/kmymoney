/***************************************************************************
                          kreconcilelistitem.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRECONCILELISTITEM_H
#define KRECONCILELISTITEM_H

#if 0 // currently, this object is unused

#include <qwidget.h>
#include <q3listview.h>
#include "mymoneytransaction.h"

/**
  *@author Michael Edwardes
  */

class KReconcileListItem : public Q3ListViewItem
{
//   Q_OBJECT
  MyMoneyTransaction *m_transaction;
public:
  KReconcileListItem(Q3ListView *parent, MyMoneyTransaction *transaction);
  ~KReconcileListItem();
  MyMoneyTransaction* transaction(void);
  void setReconciled(bool rec);

  /**
    * Overrides QListViewItem::key(int, bool)
    */
  QString key(int column, bool ascending) const;

};

#endif  // #if 0
#endif
