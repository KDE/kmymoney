/*
 * SPDX-FileCopyrightText: 2008-2015 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TRANSACTIONMATCHER_H
#define TRANSACTIONMATCHER_H

#include "kmm_base_dialogs_export.h"

#include <qglobal.h>

class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneyAccount;

class TransactionMatcherPrivate;
class KMM_BASE_DIALOGS_EXPORT TransactionMatcher
{
public:
  Q_DISABLE_COPY(TransactionMatcher)

  explicit TransactionMatcher(const MyMoneyAccount& acc);
  ~TransactionMatcher();

  /**
   * This method matches the manual entered transaction @p tm with the imported
   * transaction @p ti based on the splits @p sm and @p si. If the match can be applied,
   * MyMoneyTransaction::addMatch() is used to include @p ti inside @p tm and the
   * engine data (MyMoneyFile) is updated. A possible bankid found in the imported
   * split is carried over into the manual transaction.
   *
   * The following things will be done in case of a match:
   *
   * - if the postdate differs between the two transactions
   *   - the postdate of the manual entered transaction is stored in kmm-orig-postdate
   *   - the postdate of the imported transaction is assigned to the resulting transaction
   * - if the payee differs between the two splits
   *   - the payee of the manual split is stored in kmm-orig-payee
   *   - the payee of the imported split is assigned to the resulting split
   * - if the reconciliation state is not-reconciled
   *   - the reconciliation state is set to cleared
   * - the bankid of the imported transaction is assigned to the resulting transaction
   * - the resulting transaction will be updated and the imported transaction removed
   *   from the engine
   *
   * The application of the match depends on the following items:
   *
   * - both share values of @p sm and @p si must be identical
   * - @p tm must be a non-imported (see below), non-matched transaction
   * - @p ti must be an imported transaction
   *
   * If @p allowImportedTransactions is true, @p tm may be an imported transaction. The
   * default of @p allowImportedTransactions is @p false.
   *
   * In case of errors, an exception is thrown.
   */
  void match(MyMoneyTransaction tm, MyMoneySplit sm, MyMoneyTransaction ti, MyMoneySplit si, bool allowImportedTransactions = false);

  /**
   * This method is used to unmatch a previously matched transaction (see match() and findMatch() )
   * and restore the original and imported transaction in the engine.
   *
   * The following things will be done in case @p t is a matched transaction:
   *
   * - the enclosed imported transaction is extracted and restored
   * - if the kvp contains a kmm-orig-payee record
   *   - the payee is updated to this value if it still exists, otherwise the payee is left empty
   * - if the kvp contains a kmm-orig-postdate record
   *   - the postdate of the transaction is changed to the value stored in this record
   * - a matching bankid is removed from the transaction
   * - the resulting transaction will be updated and the imported transaction inserted
   *   into the engine
   *
   * In case of errors, an exception is thrown.
   */
  void unmatch(const MyMoneyTransaction& t, const MyMoneySplit& s);

  /**
   * This method is used to accept a previously matched transaction (see match() and findMatch())
   *
   * The following things will be done in case @p _t is a matched transaction
   *
   * - the enclosed imported transaction is removed
   * - the kvps kmm-orig-payee and kmm-orig-postdate are removed
   * - the resulting transaction will be updated
   *
   * In case of errors, an exception is thrown
   */
  void accept(const MyMoneyTransaction& t, const MyMoneySplit& s);

private:
  TransactionMatcherPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(TransactionMatcher)
};

#endif
