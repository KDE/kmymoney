/***************************************************************************
    KMyMoney transaction importing module - base class for searching for a matching transaction

    copyright            : (C) 2012 by Lukasz Maszczynski <lukasz@maszczynski.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRANSACTIONMATCHFINDER_H
#define TRANSACTIONMATCHFINDER_H

#include <QScopedPointer>

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyschedule.h"

/** The class provides an interface for finding a MyMoneyTransaction match with the one being imported
 */
class TransactionMatchFinder
{
public:
  /// enumerates possible match results
  typedef enum {
    MatchNotFound,           ///< no matching transaction found
    MatchImprecise,          ///< matching transaction found
    MatchPrecise,            ///< matching transaction found with exactly the same parameters
    MatchDuplicate           ///< found transaction duplicate
  } MatchResult;

  /** Initializes the match finder.
   * @param matchWindow max number of days the transactions may vary and still be considered to be matching
   */
  explicit TransactionMatchFinder(int m_matchWindow);
  virtual ~TransactionMatchFinder();

  /** Searches for a matching transaction. See derived classes to learn where the transaction is looked for.
   *
   * @param transactionToMatch the imported transaction we want to match
   * @param splitToMatch the split of that transaction referencing the account we import into
   * @return the search result. There are several possible results:
   *      - @ref MatchNotFound means that the imported transaction does not match any other transaction
   *      - @ref MatchDuplicate means that the imported transaction is a duplicate of another transaction,
   *      - @ref MatchImprecise means that the imported transaction matches another transaction, but the match
   *              is not precise (e.g. transaction dates are not equal, but within matchWindow range)
   *      - @ref MatchPrecise means that the imported transaction matches another transaction precisely
   */
  TransactionMatchFinder::MatchResult findMatch(const MyMoneyTransaction& transactionToMatch, const MyMoneySplit& splitToMatch);

  /** Returns the matched split.
   *
   * @throws MyMoneyException if no match is found
   */
  MyMoneySplit getMatchedSplit() const;

  /** Returns the matched transaction
   *
   * @throws MyMoneyException if no transaction was matched
   */
  MyMoneyTransaction getMatchedTransaction() const;

  /** Returns the matched schedule
   *
   * @throws MyMoneyException if no schedule was matched
   */
  MyMoneySchedule getMatchedSchedule() const;

protected:
  int                                m_matchWindow;

  MyMoneyTransaction                 importedTransaction;    //!< the imported transaction that is being matched
  MyMoneySplit                       m_importedSplit;          //!< the imported transaction's split that is being matched

  MatchResult                        matchResult;            //!< match result
  QScopedPointer<MyMoneyTransaction> matchedTransaction;     //!< the transaction that matches the imported one
  QScopedPointer<MyMoneySchedule>    matchedSchedule;        //!< the schedule that matches the imported transaction
  QScopedPointer<MyMoneySplit>       matchedSplit;           //!< the split that matches the imported one

  /** Prepares a list of match candidates for further processing, must be implemented in subclass
   */
  virtual void createListOfMatchCandidates() = 0;

  /** Searches the list of match candidates for a real match, must be implemented in subclass
   */
  virtual void findMatchInMatchCandidatesList() = 0;

  /** Checks whether one split is a duplicate of the other
   * @param split1 the first split
   * @param split2 the second split
   * @param amountVariation the max number of percent the amounts may differ and still be considered matching
   * @return true, if split2 is a duplicate of split1 (and vice-versa); false otherwise
   *
   * Splits are considered duplicates if both have the same (non-empty) bankId assigned and same amounts.
   */
  bool splitsAreDuplicates(const MyMoneySplit & split1, const MyMoneySplit & split2, int amountVariation = 0) const;

  /** Checks whether one split matches the other
   * @param importedSplit the split being imported
   * @param existingSplit the existing split
   * @param amountVariation the max number of percent the amounts may differ and still be considered matching
   * @return true, if importedSplit matches existingSplit (not necessarily the other way around); false otherwise
   *
   * Splits are considered a match if both of them:
   * - reference the same account
   * - have matching bankID-s
   * - have matching amounts
   * - have empty or matching payees
   * - are not marked as matched already
   */
  bool splitsMatch(const MyMoneySplit & m_importedSplit, const MyMoneySplit & existingSplit, int amountVariation = 0) const;

  /** Checks whether splits reference the same account
   * @param split1 the first split
   * @param split2 the second split
   * @return true, if the same account is referenced by the splits; false otherwise
   */
  bool splitsAccountsMatch(const MyMoneySplit & split1, const MyMoneySplit & split2) const;

  /** Checks whether splits amounts match
   * @param split1 the first split
   * @param split2 the second split
   * @param amountVariation the max number of percent the amounts may differ and still be considered matching
   * @return true, if amounts match; false otherwise
   */
  bool splitsAmountsMatch(const MyMoneySplit & split1, const MyMoneySplit & split2, int amountVariation = 0) const;

  /** Checks whether the splits' bankId-s match
   * @param importedSplit the imported split
   * @param existingSplit the existing split
   * @return true, if bank ids match; false otherwise
   *
   * BankID-s match if any of the two occurs:
   * - they are equal
   * - bankId of existing split is empty
   */
  bool splitsBankIdsMatch(const MyMoneySplit & m_importedSplit, const MyMoneySplit & existingSplit) const;

  /** Checks whether the splits' bankId-s are duplicated
   * @param split1 the first split
   * @param split2 the second split
   * @return true, if bank ids are equal and non-empty; false otherwise
   */
  bool splitsBankIdsDuplicated(const MyMoneySplit & split1, const MyMoneySplit & split2) const;

  /** Checks whether payees of both splits match each other or at least one of them is empty
   * @param split1 the first split
   * @param split2 the second split
   * @return true, if splits reference the same payee or at least one payee is empty; false otherwise
   */
  bool splitsPayeesMatchOrEmpty(const MyMoneySplit & split1, const MyMoneySplit & split2) const;

  /** Searches for a split in the transaction which matches imported transaction's split
   * @param transaction the transaction to look for the split in
   * @param amountVariation the max number of percent the split amounts may differ and still be considered matching
   */
  void findMatchingSplit(const MyMoneyTransaction & transaction, int amountVariation);
};

#endif // TRANSACTIONMATCHFINDER_H
