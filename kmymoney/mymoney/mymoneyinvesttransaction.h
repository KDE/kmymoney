/***************************************************************************
                          mymoneyinvesttransaction.h  -  description
                             -------------------
    begin                : Sun Feb 3 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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
