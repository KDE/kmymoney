/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELECTEDTRANSACTION_H
#define SELECTEDTRANSACTION_H

#include "kmm_oldregister_export.h"

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
  class KMM_OLDREGISTER_EXPORT SelectedTransaction
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
    MyMoneyTransaction transaction() const;

    MyMoneySplit& split();
    MyMoneySplit split() const;

    bool isScheduled() const;
    QString scheduleId() const;

    typedef enum {
      NoWarning = 0,
      OneSplitReconciled,
      OneSplitFrozen,
      OneAccountClosed
    } warnLevel_t;
    /**
   * checks the transaction for specific reasons which would
   * speak against editing/modifying it.
   * @retval 0 no sweat, user can modify
   * @retval 1 at least one split has been reconciled already
   * @retval 2 some transactions cannot be changed anymore - parts of them are frozen
   * @retval 3 some transactions cannot be changed anymore - they touch closed accounts
   */
    SelectedTransaction::warnLevel_t warnLevel() const;

    bool operator==(const SelectedTransaction& other) const;

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

uint qHash(const KMyMoneyRegister::SelectedTransaction& t, uint seed) Q_DECL_NOTHROW;

#endif

