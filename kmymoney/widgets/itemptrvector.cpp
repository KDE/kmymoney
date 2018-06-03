/***************************************************************************
                             itemptrvector.cpp  -  description
                             -------------------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "itemptrvector.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"
#include "registeritem.h"
#include "mymoneymoney.h"
#include "widgetenums.h"

using namespace eWidgets;
using namespace KMyMoneyRegister;

void ItemPtrVector::sort()
{
  if (count() > 0) {
    // get rid of 0 pointers in the list
    KMyMoneyRegister::ItemPtrVector::iterator it_l;
    RegisterItem *item;
    for (it_l = begin(); it_l != end(); ++it_l) {
      if (*it_l == 0) {
        item = last();
        *it_l = item;
        pop_back();
        --it_l;
      }
    }

    std::sort(begin(), end(), item_cmp);
  }
}

bool ItemPtrVector::item_cmp(RegisterItem* i1, RegisterItem* i2)
{
  const QList<SortField>& sortOrder = i1->getParent()->sortOrder();
  QList<SortField>::const_iterator it;
  auto rc = 0;
  bool ok1, ok2;
  qulonglong n1, n2;

  for (it = sortOrder.begin(); it != sortOrder.end(); ++it) {
    SortField sortField = static_cast<SortField>(*it);
    switch (qAbs(static_cast<int>(sortField))) {
      case (int)SortField::PostDate:
        rc = i2->sortPostDate().daysTo(i1->sortPostDate());
        break;

      case (int)SortField::EntryDate:
        rc = i2->sortEntryDate().daysTo(i1->sortEntryDate());
        break;

      case (int)SortField::Payee:
        rc = QString::localeAwareCompare(i1->sortPayee(), i2->sortPayee());
        break;

      case (int)SortField::Value:
        if (i1->sortValue() == i2->sortValue())
          rc = 0;
        else if (i1->sortValue() < i2->sortValue())
          rc = -1;
        else
          rc = 1;
        break;

      case (int)SortField::NoSort:
        // convert both values to numbers
        n1 = i1->sortNumber().toULongLong(&ok1);
        n2 = i2->sortNumber().toULongLong(&ok2);
        // the following four cases exist:
        // a) both are converted correct
        //    compare them directly
        // b) n1 is numeric, n2 is not
        //    numbers come first, so return -1
        // c) n1 is not numeric, n2 is
        //    numbers come first, so return 1
        // d) both are non numbers
        //    compare using localeAwareCompare
        if (ok1 && ok2) { // case a)
          rc = (n1 > n2) ? 1 : ((n1 == n2) ? 0 : -1);
        } else if (ok1 && !ok2) {
          rc = -1;
        } else if (!ok1 && ok2) {
          rc = 1;
        } else
          rc = QString::localeAwareCompare(i1->sortNumber(), i2->sortNumber());
        break;

      case (int)SortField::EntryOrder:
        rc = qstrcmp(i1->sortEntryOrder().toLatin1(), i2->sortEntryOrder().toLatin1());
        break;

      case (int)SortField::Type:
        rc = (int)i1->sortType() - (int)i2->sortType();
        break;

      case (int)SortField::Category:
        rc = QString::localeAwareCompare(i1->sortCategory(), i2->sortCategory());
        break;

      case (int)SortField::ReconcileState:
        rc = static_cast<int>(i1->sortReconcileState()) - static_cast<int>(i2->sortReconcileState());
        break;

      case (int)SortField::Security:
        rc = QString::localeAwareCompare(i1->sortSecurity(), i2->sortSecurity());
        break;

      default:
        qDebug("Invalid sort key %d", (int)*it);
        break;
    }

    // take care of group markers, but only first sort item
    if ((rc == 0) && (it == sortOrder.begin())) {
      rc = i1->sortSamePostDate() - i2->sortSamePostDate();
      if (rc) {
        return rc < 0;
      }
    }

    // the items differ for this sort key so we can return a result
    if (rc != 0) {
      return ((int)*it < 0) ? rc >= 0 : rc < 0;
    }
  }

  if (rc == 0) {
    rc = qstrcmp(i1->sortEntryOrder().toLatin1(), i2->sortEntryOrder().toLatin1());
  }

  return rc < 0;
}
