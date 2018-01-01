/***************************************************************************
                          mymoneydatabasemgr.cpp
                             -------------------
    begin                : June 5 2007
    copyright            : (C) 2007 by Fernando Vilas
    email                : Fernando Vilas <fvilas@iname.com>
                           2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYDATABASEMGR_P_H
#define MYMONEYDATABASEMGR_P_H

#include "mymoneydatabasemgr.h"

#include <algorithm>
#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QBitArray>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransactionfilter.h"
#include "mymoneycategory.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneytag.h"
#include "mymoneybudget.h"
#include "mymoneyschedule.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneypayee.h"
#include "mymoneyreport.h"
#include "mymoneycostcenter.h"
#include "mymoneymap.h"
#include "mymoneystoragesql.h"
#include "mymoneyenums.h"
#include "storageenums.h"

using namespace eStorage;

class MyMoneyDatabaseMgrPrivate
{
  Q_DISABLE_COPY(MyMoneyDatabaseMgrPrivate)
  Q_DECLARE_PUBLIC(MyMoneyDatabaseMgr)

public:
  explicit MyMoneyDatabaseMgrPrivate(MyMoneyDatabaseMgr* qq) :
    q_ptr(qq),
    m_creationDate(QDate::currentDate()),
    m_currentFixVersion(0),
    m_fileFixVersion(0),
    m_lastModificationDate(QDate::currentDate()),
    m_sql(0)
  {
  }

  ~MyMoneyDatabaseMgrPrivate()
  {
  }

  void removeReferences(const QString& id)
  {
    QMap<QString, MyMoneyReport>::const_iterator it_r;
    QMap<QString, MyMoneyBudget>::const_iterator it_b;

    // remove from reports
    QMap<QString, MyMoneyReport> reportList = m_sql->fetchReports();
    for (it_r = reportList.constBegin(); it_r != reportList.constEnd(); ++it_r) {
      MyMoneyReport r = *it_r;
      r.removeReference(id);
  //    reportList.modify(r.id(), r);
    }

    // remove from budgets
    QMap<QString, MyMoneyBudget> budgetList = m_sql->fetchBudgets();
    for (it_b = budgetList.constBegin(); it_b != budgetList.constEnd(); ++it_b) {
      MyMoneyBudget b = *it_b;
      b.removeReference(id);
  //    budgetList.modify(b.id(), b);
    }
  }

  MyMoneyDatabaseMgr *q_ptr;
  /**
    * This member variable keeps the creation date of this MyMoneyDatabaseMgr
    * object. It is set during the constructor and can only be modified using
    * the stream read operator.
    */
  QDate m_creationDate;

  /**
    * This member variable contains the current fix level of application
    * data files. (see kmymoneyview.cpp)
    */
  uint m_currentFixVersion;

  /**
   * This member variable contains the current fix level of the
   *  presently open data file. (see kmymoneyview.cpp)
   */
  uint m_fileFixVersion;

  /**
    * This member variable keeps the date of the last modification of
    * the MyMoneyDatabaseMgr object.
    */
  QDate m_lastModificationDate;

  /**
    * This contains the interface with SQL reader for database access
    */
  QExplicitlySharedDataPointer <MyMoneyStorageSql> m_sql;

  /**
    * This member variable keeps the User information.
    * @see setUser()
    */
  MyMoneyPayee m_user;
};
#endif
