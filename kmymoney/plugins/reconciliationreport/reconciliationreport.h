/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
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
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
    explicit ReconciliationReport(QObject *parent, const QVariantList &args);
#else
    explicit ReconciliationReport(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
#endif
    ~ReconciliationReport() override;

public Q_SLOTS:
    void plug(KXMLGUIFactory* guiFactory) override;
    void unplug() override;

protected Q_SLOTS:
    // reconciliation of an account has finished
    void slotGenerateReconciliationReport(const MyMoneyAccount& account,
                                          const QDate& date,
                                          const MyMoneyMoney& startingBalance,
                                          const MyMoneyMoney& endingBalance,
                                          const QStringList& transactionList);
};

#endif // RECONCILIATIONREPORT_H

