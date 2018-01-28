/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#ifndef RECONCILIATIONREPORT_H
#define RECONCILIATIONREPORT_H

#include "kmymoneyplugin.h"
#include "mymoneymoney.h"

class MyMoneyAccount;
class MyMoneySplit;
class MyMoneyTransaction;

class ReconciliationReport: public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  explicit ReconciliationReport(QObject *parent, const QVariantList &args);
  ~ReconciliationReport() override;

public Q_SLOTS:
  void plug() override;
  void unplug() override;

protected Q_SLOTS:
  // reconciliation of an account has finished
  void slotGenerateReconciliationReport(const MyMoneyAccount& account, const QDate& date, const MyMoneyMoney& startingBalance, const MyMoneyMoney& endingBalance, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& transactionList);
};

#endif // RECONCILIATIONREPORT_H

