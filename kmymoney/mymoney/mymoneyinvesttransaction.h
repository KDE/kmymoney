/*
 * Copyright 2002       Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2002       Kevin Tambascio <ktambascio@users.sourceforge.net>
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
