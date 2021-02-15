/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

