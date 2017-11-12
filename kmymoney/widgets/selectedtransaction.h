/***************************************************************************
                          selectedtransaction.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
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

#ifndef SELECTEDTRANSACTION_H
#define SELECTEDTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class MyMoneySplit;
class MyMoneyTransaction;

namespace KMyMoneyRegister
{
  class SelectedTransactionPrivate;
  class SelectedTransaction
  {
  public:
    SelectedTransaction();
    SelectedTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s, const QString& scheduleId);
    SelectedTransaction(const SelectedTransaction & other);
    SelectedTransaction(SelectedTransaction && other);
    SelectedTransaction & operator=(SelectedTransaction other);
    friend void swap(SelectedTransaction& first, SelectedTransaction& second);
    ~SelectedTransaction();

    MyMoneyTransaction& transaction();
    const MyMoneyTransaction& transaction() const;

    MyMoneySplit& split();
    const MyMoneySplit& split() const;

    bool isScheduled() const;
    const QString& scheduleId() const;

    /**
   * checks the transaction for specific reasons which would
   * speak against editing/modifying it.
   * @retval 0 no sweat, user can modify
   * @retval 1 at least one split has been reconciled already
   * @retval 2 some transactions cannot be changed anymore - parts of them are frozen
   * @retval 3 some transactions cannot be changed anymore - they touch closed accounts
   */
    int warnLevel() const;

  private:
    SelectedTransactionPrivate* d_ptr;
    Q_DECLARE_PRIVATE(SelectedTransaction)
  };

  inline void swap(SelectedTransaction& first, SelectedTransaction& second) // krazy:exclude=inline
  {
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
  }

  inline SelectedTransaction::SelectedTransaction(SelectedTransaction && other) : SelectedTransaction() // krazy:exclude=inline
  {
    swap(*this, other);
  }

  inline SelectedTransaction & SelectedTransaction::operator=(SelectedTransaction other) // krazy:exclude=inline
  {
    swap(*this, other);
    return *this;
  }
} // namespace

#endif

