/*
    SPDX-FileCopyrightText: 2005 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// krazy:excludeall=dpointer

#ifndef VIEWINTERFACE_H
#define VIEWINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"

#include "icons.h"
#include <kmm_plugin_export.h>

namespace KMyMoneyRegister {
class SelectedTransactions;
}

enum class View;

class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneySplit;
class MyMoneyTransaction;
class KMyMoneyViewBase;

namespace KMyMoneyPlugin {

/**
 * This abstract class represents the ViewInterface to
 * add new view pages to the JanusWidget of KMyMoney. It
 * also gives access to the account context menu.
 */
class KMM_PLUGIN_EXPORT ViewInterface : public QObject
{
    Q_OBJECT

public:
    explicit ViewInterface(QObject* parent, const char* name = 0);
    virtual ~ViewInterface();

    virtual void addView(KMyMoneyViewBase* view, const QString& name, View idView, Icons::Icon icon) = 0;
    virtual void removeView(View idView) = 0;

Q_SIGNALS:
    /**
     * This signal is emitted when a new account has been selected by
     * the GUI. If no account is selected or the selection is removed,
     * @a account is identical to MyMoneyAccount(). This signal is used
     * by plugins to get information about changes.
     */
    void accountSelected(const MyMoneyAccount& acc);

    /**
     * This signal is emitted when a transaction/list of transactions has been selected by
     * the GUI. If no transaction is selected or the selection is removed,
     * @p transactions is identical to an empty QList. This signal is used
     * by plugins to get information about changes.
     */
    void transactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions);

    /**
     * This signal is emitted when a new institution has been selected by
     * the GUI. If no institution is selected or the selection is removed,
     * @a institution is identical to MyMoneyInstitution(). This signal is used
     * by plugins to get information about changes.
     */
    //  void institutionSelected(const MyMoneyInstitution& institution);

    /**
     * This signal is emitted when an account has been successfully reconciled
     * and all transactions are updated in the engine. It can be used by plugins
     * to create reconciliation reports.
     *
     * @param account the account data
     * @param date the reconciliation date as provided through the dialog
     * @param startingBalance the starting balance as provided through the dialog
     * @param endingBalance the ending balance as provided through the dialog
     * @param transactionList reference to QStringList of JournalEntryIds
     */
    void accountReconciled(const MyMoneyAccount& account,
                           const QDate& date,
                           const MyMoneyMoney& startingBalance,
                           const MyMoneyMoney& endingBalance,
                           const QStringList& transactionList);

    void viewStateChanged(bool);
};

} // namespace
#endif
