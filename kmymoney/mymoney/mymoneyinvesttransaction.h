/*
    SPDX-FileCopyrightText: 2002 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYINVESTTRANSACTION_H
#define MYMONEYINVESTTRANSACTION_H

#if 0
/**
  *@author Kevin Tambascio
  */

class MyMoneyInvestTransaction : public MyMoneyTransaction
{
public:
  MyMoneyInvestTransaction();
  MyMoneyInvestTransaction(MyMoneyAccount *parent, const long id, transactionMethod method, const QString& number, const QString& memo,
                           const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
                           const QString& fromTo, const QString& bankFrom, const QString& bankTo, stateE state);
  ~MyMoneyInvestTransaction();
};
#endif

#endif
