/***************************************************************************
                          mymoneyqifwriter.cpp  -  description
                             -------------------
    begin                : Sun Jan 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
// QT Headers

#include <qfile.h>
#include <q3textstream.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifwriter.h"
#include "mymoneyfile.h"

MyMoneyQifWriter::MyMoneyQifWriter()
{
}

MyMoneyQifWriter::~MyMoneyQifWriter()
{
}

void MyMoneyQifWriter::write(const QString& filename, const QString& profile,
             const QString& accountId, const bool accountData,
             const bool categoryData,
             const QDate& startDate, const QDate& endDate)
{
  m_qifProfile.loadProfile("Profile-" + profile);

  QFile qifFile(filename);
  if(qifFile.open(QIODevice::WriteOnly)) {
    Q3TextStream s(&qifFile);

    try {
      if(categoryData) {
        writeCategoryEntries(s);
      }

      if(accountData) {
        writeAccountEntry(s, accountId, startDate, endDate);
      }
      emit signalProgress(-1, -1);

    } catch(MyMoneyException *e) {
      QString errMsg = i18n("Unexpected exception '%1' thrown in %2, line %3 "
                            "caught in MyMoneyQifWriter::write()",e->what(),e->file(),e->line());

      KMessageBox::error(0, errMsg);
      delete e;
    }

    qifFile.close();
  } else {
    KMessageBox::error(0, i18n("Unable to open file '%1' for writing",filename));
  }
}

void MyMoneyQifWriter::writeAccountEntry(Q3TextStream &s, const QString& accountId, const QDate& startDate, const QDate& endDate)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  account = file->account(accountId);
  MyMoneyTransactionFilter filter(accountId);
  filter.setDateFilter(startDate, endDate);
  Q3ValueList<MyMoneyTransaction> list = file->transactionList(filter);
  QString openingBalanceTransactionId;

  s << "!Type:" << m_qifProfile.profileType() << endl;
  if(!startDate.isValid() || startDate <= account.openingDate()) {
    s << "D" << m_qifProfile.date(account.openingDate()) << endl;
    openingBalanceTransactionId = file->openingBalanceTransaction(account);
    MyMoneySplit split;
    if(!openingBalanceTransactionId.isEmpty()) {
      MyMoneyTransaction openingBalanceTransaction = file->transaction(openingBalanceTransactionId);
      split = openingBalanceTransaction.splitByAccount(account.id(), true /* match */);
    }
    s << "T" << m_qifProfile.value('T', split.value()) << endl;
  } else {
    s << "D" << m_qifProfile.date(startDate) << endl;
    s << "T" << m_qifProfile.value('T', file->balance(accountId, startDate.addDays(-1))) << endl;
  }
  s << "CX" << endl;
  s << "P" << m_qifProfile.openingBalanceText() << endl;
  s << "L";
  if(m_qifProfile.accountDelimiter().length())
    s << m_qifProfile.accountDelimiter()[0];
  s << account.name();
  if(m_qifProfile.accountDelimiter().length() > 1)
    s << m_qifProfile.accountDelimiter()[1];
  s << endl;
  s << "^" << endl;

  Q3ValueList<MyMoneyTransaction>::ConstIterator it;
  signalProgress(0, list.count());
  int count = 0;
  for(it = list.begin(); it != list.end(); ++it) {
    // don't include the openingBalanceTransaction again
    if((*it).id() != openingBalanceTransactionId)
      writeTransactionEntry(s, *it, accountId);
    signalProgress(++count, 0);
  }
}

void MyMoneyQifWriter::writeCategoryEntries(Q3TextStream &s)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount income;
  MyMoneyAccount expense;

  income = file->income();
  expense = file->expense();

  s << "!Type:Cat" << endl;
  QStringList list = income.accountList() + expense.accountList();
  emit signalProgress(0, list.count());
  QStringList::Iterator it;
  int count = 0;
  for(it = list.begin(); it != list.end(); ++it) {
    writeCategoryEntry(s, *it, "");
    emit signalProgress(++count, 0);
  }
}

void MyMoneyQifWriter::writeCategoryEntry(Q3TextStream &s, const QString& accountId, const QString& leadIn)
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
  QString name = acc.name();

  s << "N" << leadIn << name << endl;
  s << (MyMoneyAccount::accountGroup(acc.accountType()) == MyMoneyAccount::Expense ? "E" : "I") << endl;
  s << "^" << endl;

  QStringList list = acc.accountList();
  QStringList::Iterator it;
  name += ":";
  for(it = list.begin(); it != list.end(); ++it) {
    writeCategoryEntry(s, *it, name);
  }
}

void MyMoneyQifWriter::writeTransactionEntry(Q3TextStream &s, const MyMoneyTransaction& t, const QString& accountId)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySplit split = t.splitByAccount(accountId);

  s << "D" << m_qifProfile.date(t.postDate()) << endl;

  switch(split.reconcileFlag()) {
    case MyMoneySplit::Cleared:
      s << "C*" << endl;
      break;

    case MyMoneySplit::Reconciled:
    case MyMoneySplit::Frozen:
      s << "CX" << endl;
      break;

    default:
      break;
  }

  if(split.memo().length() > 0)
    s << "M" << split.memo() << endl;

  s << "T" << m_qifProfile.value('T', split.value()) << endl;

  if(split.number().length() > 0)
    s << "N" << split.number() << endl;

  if(!split.payeeId().isEmpty()) {
    MyMoneyPayee payee = file->payee(split.payeeId());
    s << "P" << payee.name() << endl;
  }

  Q3ValueList<MyMoneySplit> list = t.splits();
  if(list.count() > 1) {
    MyMoneySplit sp = t.splitByAccount(accountId, false);
    MyMoneyAccount acc = file->account(sp.accountId());
    if(acc.accountGroup() != MyMoneyAccount::Income
    && acc.accountGroup() != MyMoneyAccount::Expense) {
      s << "L" << m_qifProfile.accountDelimiter()[0]
              << MyMoneyFile::instance()->accountToCategory(sp.accountId())
              << m_qifProfile.accountDelimiter()[1] << endl;
    } else {
      s << "L" << file->accountToCategory(sp.accountId()) << endl;
    }
    if(list.count() > 2) {
      Q3ValueList<MyMoneySplit>::ConstIterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        if(!((*it) == split)) {
          writeSplitEntry(s, *it);
        }
      }
    }
  }
  s << "^" << endl;
}

void MyMoneyQifWriter::writeSplitEntry(Q3TextStream& s, const MyMoneySplit& split)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  s << "S";
  MyMoneyAccount acc = file->account(split.accountId());
  if(acc.accountGroup() != MyMoneyAccount::Income
  && acc.accountGroup() != MyMoneyAccount::Expense) {
    s << m_qifProfile.accountDelimiter()[0]
      << file->accountToCategory(split.accountId())
      << m_qifProfile.accountDelimiter()[1];
  } else {
    s << file->accountToCategory(split.accountId());
  }
  s << endl;

  if(split.memo().length() > 0)
    s << "E" << split.memo() << endl;

  s << "$" << m_qifProfile.value('$', -split.value()) << endl;
}

#include "mymoneyqifwriter.moc"
