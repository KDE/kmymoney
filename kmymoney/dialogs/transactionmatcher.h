/***************************************************************************
                             transactionmatcher.h
                             ----------
    begin                : Tue Jul 08 2008
    copyright            : (C) 2008 by Thomas Baumgart
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

#ifndef TRANSACTIONMATCHER_H
#define TRANSACTIONMATCHER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QPair>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneytransaction.h>
#include <mymoneyaccount.h>
class MyMoneySchedule;

class TransactionMatcher
{
public:
  typedef enum {
    notMatched = 0,          ///< no matching transaction found
    matched,                 ///< matching transaction found
    matchedExact,            ///< matching transaction found with exact same date
    matchedDuplicate         ///< duplicate matching transaction found
  } autoMatchResultE;

  TransactionMatcher(const MyMoneyAccount& acc);

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

  /**
   * This method is used to automatically find a matching transaction in the ledger or the schedules.
   * It should also detect duplicate imports according to the splits bankid.
   *
   * To be designed
   *
   * @param ti the imported transaction we want to match
   * @param si the split of that transaction referencing the account we import into
   * @param sm the split of the object returned that matches the split @a si. In case
   *           the returned pointer is not 0 this object contains the split. In other
   *           cases it contains an empty MyMoneySplit.
   * @param result reference to the result details
   *
   * @returns pointer to MyMoneyObject (either a MyMoneyTransaction or a MyMoneySchedule). The
   *          caller of this method becomes the owner of the object pointed to and is responsible
   *          to delete the object
   */
  MyMoneyObject const * findMatch(const MyMoneyTransaction& ti, const MyMoneySplit& si, MyMoneySplit& sm, autoMatchResultE& result);

  /**
   * Sets the number of @a days to look for matching transactions. The default after object creation is 3 days.
   */
  void setMatchWindow(int days) {
    m_days = days;
  }

private:
  void checkTransaction(const MyMoneyTransaction& tm, const MyMoneyTransaction& ti, const MyMoneySplit& si, QPair<MyMoneyTransaction, MyMoneySplit>& lastMatch, autoMatchResultE& result, int variation = 0) const;

private:
  MyMoneyAccount            m_account;
  int                       m_days;
};


#endif
