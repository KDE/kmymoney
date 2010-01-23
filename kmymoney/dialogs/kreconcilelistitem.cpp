/***************************************************************************
                          kReconcileListitem.cpp
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

#if 0  // currently, this object is unused

#include <kglobal.h>
#include <klocale.h>
#include "kreconcilelistitem.h"

KReconcileListItem::KReconcileListItem(Q3ListView *parent, MyMoneyTransaction *transaction)
    : Q3ListViewItem(parent)
{
  /*
    QString colText;

    m_transaction = transaction;

    setText(0, KGlobal::locale()->formatDate(m_transaction->date(), true));
    setText(1, m_transaction->number());
    setText(2, m_transaction->payee());
    setText(3, KGlobal::locale()->formatMoney(m_transaction->amount().amount(),""));

    QString tmp = " ";
    switch (m_transaction->state()) {
      case MyMoneyTransaction::Reconciled:
        tmp = i18n("R");
        setSelected(true);
        break;
      case MyMoneyTransaction::Cleared:
        tmp = i18n("C");
        break;
      default:
        break;
    }
    setText(4, tmp);
  */
}

KReconcileListItem::~KReconcileListItem()
{
}

void KReconcileListItem::setReconciled(bool rec)
{
  /*
    QString temp = " ";
    if(rec == true) {
      m_transaction->setState(MyMoneyTransaction::Reconciled);
      temp = i18n("R");
    } else {
      m_transaction->setState(MyMoneyTransaction::Unreconciled);
    }
    setText(4,temp);
  */
}

MyMoneyTransaction* KReconcileListItem::transaction(void)
{
  return m_transaction;
}

QString KReconcileListItem::key(int column, bool ascending) const
{
  /*
    QString tmp = "";
    switch (column ) {
      case 0:
        tmp.sprintf("%08d", m_transaction->date().daysTo( QDate(1900, 1, 1) ));
        break;
      case 3:
        tmp.sprintf("%020.2f", m_transaction->amount().amount());
        break;
      default: tmp = text(column);
        break;
    }
    return tmp;
  */
}

#endif

