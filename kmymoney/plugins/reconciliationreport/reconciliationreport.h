/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

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
  void plug(KXMLGUIFactory* guiFactory) override;
  void unplug() override;

protected Q_SLOTS:
  // reconciliation of an account has finished
  void slotGenerateReconciliationReport(const MyMoneyAccount& account, const QDate& date, const MyMoneyMoney& startingBalance, const MyMoneyMoney& endingBalance, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& transactionList);
};

#endif // RECONCILIATIONREPORT_H

