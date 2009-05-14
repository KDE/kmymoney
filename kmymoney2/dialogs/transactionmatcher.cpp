/***************************************************************************
                             transactionmatcher.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "transactionmatcher.h"
#include <mymoneyfile.h>
#include <mymoneyscheduled.h>
#include <kmymoneyutils.h>
//Added by qt3to4:
#include <Q3ValueList>

TransactionMatcher::TransactionMatcher(const MyMoneyAccount& acc) :
  m_account(acc),
  m_days(3)
{
}

void TransactionMatcher::match(MyMoneyTransaction tm, MyMoneySplit sm, MyMoneyTransaction ti, MyMoneySplit si, bool allowImportedTransactions)
{
  const MyMoneySecurity& sec = MyMoneyFile::instance()->security(m_account.currencyId());

  // Now match the transactions.
  //
  // 'Matching' the transactions entails DELETING the end transaction,
  // and MODIFYING the start transaction as needed.
  //
  // There are a variety of ways that a transaction can conflict.
  // Post date, splits, amount are the ones that seem to matter.
  // TODO: Handle these conflicts intelligently, at least warning
  // the user, or better yet letting the user choose which to use.
  //
  // For now, we will just use the transaction details from the start
  // transaction.  The only thing we'll take from the end transaction
  // are the bank ID's.
  //
  // What we have to do here is iterate over the splits in the end
  // transaction, and find the corresponding split in the start
  // transaction.  If there is a bankID in the end split but not the
  // start split, add it to the start split.  If there is a bankID
  // in BOTH, then this transaction cannot be merged (both transactions
  // were imported!!)  If the corresponding start split cannot  be
  // found and the end split has a bankID, we should probably just fail.
  // Although we could ADD it to the transaction.

  // ipwizard: Don't know if iterating over the transactions is a good idea.
  // In case of a split transaction recorded with KMyMoney and the transaction
  // data being imported consisting only of a single category assignment, this
  // does not make much sense. The same applies for investment transactions
  // stored in KMyMoney against imported transactions. I think a better solution
  // is to just base the match on the splits referencing the same (currently
  // selected) account.

  // verify, that tm is a manually (non-matched) transaction and ti an imported one
  if(sm.isMatched() || (!allowImportedTransactions && tm.isImported()))
    throw new MYMONEYEXCEPTION(i18n("First transaction does not match requirement for matching"));
  if(!ti.isImported())
    throw new MYMONEYEXCEPTION(i18n("Second transaction does not match requirement for matching"));

  // verify that the amounts are the same, otherwise we should not be matching!
  if(sm.shares() != si.shares()) {
    throw new MYMONEYEXCEPTION(i18n("Splits for %1 have conflicting values (%2,%3)").arg(m_account.name()).arg(sm.shares().formatMoney(m_account, sec), si.shares().formatMoney(m_account, sec)));
  }

  // ipwizard: I took over the code to keep the bank id found in the endMatchTransaction
  // This might not work for QIF imports as they don't setup this information. It sure
  // makes sense for OFX and HBCI.
  const QString& bankID = si.bankID();
  if (!bankID.isEmpty()) {
    try {
      if (sm.bankID().isEmpty() ) {
        sm.setBankID( bankID );
        tm.modifySplit(sm);
      } else if(sm.bankID() != bankID) {
        throw new MYMONEYEXCEPTION(i18n("Both of these transactions have been imported into %1.  Therefore they cannot be matched.  Matching works with one imported transaction and one non-imported transaction.").arg(m_account.name()));
      }
    } catch(MyMoneyException *e) {
      QString estr = e->what();
      delete e;
      throw new MYMONEYEXCEPTION(i18n("Unable to match all splits (%1)").arg(estr));
    }
  }

#if 0 // Ace's original code
  // TODO (Ace) Add in another error to catch the case where a user
  // tries to match two hand-entered transactions.
  Q3ValueList<MyMoneySplit> endSplits = endMatchTransaction.splits();
  Q3ValueList<MyMoneySplit>::const_iterator it_split = endSplits.begin();
  while (it_split != endSplits.end())
  {
    // find the corresponding split in the start transaction
    MyMoneySplit startSplit;
    QString accountid = (*it_split).accountId();
    try
    {
      startSplit = startMatchTransaction.splitByAccount( accountid );
    }
      // only exception is thrown if we cannot find a split like this
    catch(MyMoneyException *e)
    {
      delete e;
      startSplit = (*it_split);
      startSplit.clearId();
      startMatchTransaction.addSplit(startSplit);
    }

    // verify that the amounts are the same, otherwise we should not be
    // matching!
    if ( (*it_split).value() != startSplit.value() )
    {
      QString accountname = MyMoneyFile::instance()->account(accountid).name();
      throw new MYMONEYEXCEPTION(i18n("Splits for %1 have conflicting values (%2,%3)").arg(accountname).arg((*it_split).value().formatMoney(),startSplit.value().formatMoney()));
    }

    QString bankID = (*it_split).bankID();
    if ( ! bankID.isEmpty() )
    {
      try
      {
        if ( startSplit.bankID().isEmpty() )
        {
          startSplit.setBankID( bankID );
          startMatchTransaction.modifySplit(startSplit);
        }
        else
        {
          QString accountname = MyMoneyFile::instance()->account(accountid).name();
          throw new MYMONEYEXCEPTION(i18n("Both of these transactions have been imported into %1.  Therefore they cannot be matched.  Matching works with one imported transaction and one non-imported transaction.").arg(accountname));
        }
      }
      catch(MyMoneyException *e)
      {
        QString estr = e->what();
        delete e;
        throw new MYMONEYEXCEPTION(i18n("Unable to match all splits (%1)").arg(estr));
      }
    }
    ++it_split;
  }
#endif

  // mark the split as cleared if it does not have a reconciliation information yet
  if(sm.reconcileFlag() == MyMoneySplit::NotReconciled) {
    sm.setReconcileFlag(MyMoneySplit::Cleared);
  }

  // if we don't have a payee assigned to the manually entered transaction
  // we use the one we found in the imported transaction
  if(sm.payeeId().isEmpty() && !si.payeeId().isEmpty()) {
    sm.setValue("kmm-orig-payee", sm.payeeId());
    sm.setPayeeId(si.payeeId());
  }

  // We use the imported postdate and keep the previous one for unmatch
  if(tm.postDate() != ti.postDate()) {
    sm.setValue("kmm-orig-postdate", tm.postDate().toString(Qt::ISODate));
    tm.setPostDate(ti.postDate());
  }

  // combine the two memos into one
  QString memo = sm.memo();
  if(!si.memo().isEmpty() && si.memo() != memo) {
    sm.setValue("kmm-orig-memo", memo);
    if(!memo.isEmpty())
      memo += "\n";
    memo += si.memo();
  }
  sm.setMemo(memo);

  // remember the split we matched
  sm.setValue("kmm-match-split", si.id());

  sm.addMatch(ti);
  tm.modifySplit(sm);

  MyMoneyFile::instance()->modifyTransaction(tm);
  // Delete the end transaction if it was stored in the engine
  if(!ti.id().isEmpty())
    MyMoneyFile::instance()->removeTransaction(ti);
}

void TransactionMatcher::unmatch(const MyMoneyTransaction& _t, const MyMoneySplit& _s)
{
  if(_s.isMatched()) {
    MyMoneyTransaction tm(_t);
    MyMoneySplit sm(_s);
    MyMoneyTransaction ti(sm.matchedTransaction());
    MyMoneySplit si;
    // if we don't have a split, then we don't have a memo
    try {
      si = ti.splitById(sm.value("kmm-match-split"));
    } catch(MyMoneyException* e) {
      delete e;
    }
    sm.removeMatch();

    // restore the postdate if modified
    if(!sm.value("kmm-orig-postdate").isEmpty()) {
      tm.setPostDate(QDate::fromString(sm.value("kmm-orig-postdate"), Qt::ISODate));
    }

    // restore payee if modified
    if(!sm.value("kmm-orig-payee").isEmpty()) {
      sm.setPayeeId(sm.value("kmm-orig-payee"));
    }

    // restore memo if modified
    if(!sm.value("kmm-orig-memo").isEmpty()) {
      sm.setMemo(sm.value("kmm-orig-memo"));
    }

    sm.deletePair("kmm-orig-postdate");
    sm.deletePair("kmm-orig-payee");
    sm.deletePair("kmm-orig-memo");
    sm.deletePair("kmm-match-split");
    tm.modifySplit(sm);

    MyMoneyFile::instance()->modifyTransaction(tm);
    MyMoneyFile::instance()->addTransaction(ti);
  }
}

void TransactionMatcher::accept(const MyMoneyTransaction& _t, const MyMoneySplit& _s)
{
  if(_s.isMatched()) {
    MyMoneyTransaction tm(_t);
    MyMoneySplit sm(_s);
    sm.removeMatch();
    sm.deletePair("kmm-orig-postdate");
    sm.deletePair("kmm-orig-payee");
    sm.deletePair("kmm-orig-memo");
    sm.deletePair("kmm-match-split");
    tm.modifySplit(sm);

    MyMoneyFile::instance()->modifyTransaction(tm);
  }
}

void TransactionMatcher::checkTransaction(const MyMoneyTransaction& tm, const MyMoneyTransaction& ti, const MyMoneySplit& si, QPair<MyMoneyTransaction, MyMoneySplit>& lastMatch, TransactionMatcher::autoMatchResultE& result, int variation) const
{
  Q_UNUSED(ti);


  const Q3ValueList<MyMoneySplit>& splits = tm.splits();
  Q3ValueList<MyMoneySplit>::const_iterator it_s;
  for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
    MyMoneyMoney upper((*it_s).shares());
    MyMoneyMoney lower(upper);
    if((variation > 0) && (variation < 100)) {
      lower = lower - (lower.abs() * MyMoneyMoney(variation, 100));
      upper = upper + (upper.abs() * MyMoneyMoney(variation, 100));
    }
    // we only check for duplicates / matches if the sign
    // of the amount for this split is identical
    if((si.shares() >= lower) && (si.shares() <= upper)) {
      // check for duplicate (we can only do that, if we have a bankID)
      if(!si.bankID().isEmpty()) {
        if((*it_s).bankID() == si.bankID()) {
          lastMatch = QPair<MyMoneyTransaction, MyMoneySplit>(tm, *it_s);
          result = matchedDuplicate;
          break;
        }
        // in case the stored split already has a bankid
        // assigned, it must be a different one and therefore
        // will certainly not match
        if(!(*it_s).bankID().isEmpty())
          continue;
      }
      // check if this is the one that matches
      if((*it_s).accountId() == si.accountId()
      && (si.shares() >= lower) && (si.shares() <= upper)
      && !(*it_s).isMatched()) {
        if(tm.postDate() == ti.postDate()) {
          lastMatch = QPair<MyMoneyTransaction, MyMoneySplit>(tm, *it_s);
          result = matchedExact;
        } else if(result != matchedExact) {
          lastMatch = QPair<MyMoneyTransaction, MyMoneySplit>(tm, *it_s);
          result = matched;
        }
      }
    }
  }
}

MyMoneyObject const * TransactionMatcher::findMatch(const MyMoneyTransaction& ti, const MyMoneySplit& si, MyMoneySplit& sm, autoMatchResultE& result)
{
  result = notMatched;
  sm = MyMoneySplit();

  MyMoneyTransactionFilter filter(si.accountId());
  filter.setReportAllSplits(false);
  filter.setDateFilter(ti.postDate().addDays(-m_days), ti.postDate().addDays(m_days));
  filter.setAmountFilter(si.shares(), si.shares());

  Q3ValueList<QPair<MyMoneyTransaction, MyMoneySplit> > list;
  MyMoneyFile::instance()->transactionList(list, filter);

  // parse list
  Q3ValueList<QPair<MyMoneyTransaction, MyMoneySplit> >::iterator it_l;
  QPair<MyMoneyTransaction, MyMoneySplit> lastMatch;

  for(it_l = list.begin(); (result != matchedDuplicate) && (it_l != list.end()); ++it_l) {
    // just skip myself
    if((*it_l).first.id() == ti.id()) {
      continue;
    }

    checkTransaction((*it_l).first, ti, si, lastMatch, result);
  }

  MyMoneyObject* rc = 0;
  if(result != notMatched) {
    sm = lastMatch.second;
    rc = new MyMoneyTransaction(lastMatch.first);

  } else {
    // if we did not find anything, we need to scan for scheduled transactions
    Q3ValueList<MyMoneySchedule> list;
    Q3ValueList<MyMoneySchedule>::iterator it_sch;
    // find all schedules that have a reference to the current account
    list = MyMoneyFile::instance()->scheduleList(m_account.id());
    for(it_sch = list.begin(); (result != matched && result != matchedExact) && (it_sch != list.end()); ++it_sch) {
      // get the next due date adjusted by the weekend switch
      QDate nextDueDate = (*it_sch).nextDueDate();
      if((*it_sch).isOverdue() ||
         (nextDueDate >= ti.postDate().addDays(-m_days)
         && nextDueDate <= ti.postDate().addDays(m_days))) {
        MyMoneyTransaction st = KMyMoneyUtils::scheduledTransaction(*it_sch);
        checkTransaction(st, ti, si, lastMatch, result, (*it_sch).variation());
        if(result == matched || result == matchedExact) {
          sm = lastMatch.second;
          rc = new MyMoneySchedule(*it_sch);
        }
      }
    }
  }

  return rc;
}

