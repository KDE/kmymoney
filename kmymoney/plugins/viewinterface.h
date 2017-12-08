/***************************************************************************
                          viewinterface.h
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#include "mymoneymoney.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"

#include <kmm_plugin_export.h>

namespace KMyMoneyRegister
{
class SelectedTransactions;
}

class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneySplit;
class MyMoneyTransaction;
class IMyMoneyStorageFormat;

namespace KMyMoneyPlugin
{

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

  /**
    * Calls MyMoneyFile::readAllData which reads a MyMoneyFile into appropriate
    * data structures in memory.  The return result is examined to make sure no
    * errors occurred whilst parsing.
    *
    * @param url The URL to read from.
    *            If no protocol is specified, file:// is assumed.
    *
    * @return Whether the read was successful.
    */
  virtual bool readFile(const QUrl &url, IMyMoneyStorageFormat *pExtReader = nullptr) = 0;

  /**
    * Makes sure that a MyMoneyFile is open and has been created successfully.
    *
    * @return Whether the file is open and initialised
    */
  virtual bool fileOpen() = 0;

  /**
    * Brings up a dialog to change the list(s) settings and saves them into the
    * class KMyMoneySettings (a singleton).
    *
    * @see KListSettingsDlg
    * Refreshes all views. Used e.g. after settings have been changed or
    * data has been loaded from external sources (QIF import).
    **/
  virtual void slotRefreshViews() = 0;

  /**
    * This method creates a new page in the application.
    * See KPageWidget::addPage() for details.
    */
//  virtual KMyMoneyViewBase* addPage(const QString& item, const QString& icon) = 0;

  /**
    * This method adds a widget to the layout of the view
    * created with addPage()
    *
    * @param view pointer to view widget
    * @param w widget to be added to @p page
    */
//  virtual void addWidget(KMyMoneyViewBase* view, QWidget* w) = 0;

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
  void institutionSelected(const MyMoneyInstitution& institution);

  /**
   * This signal is emitted when an account has been successfully reconciled
   * and all transactions are updated in the engine. It can be used by plugins
   * to create reconciliation reports.
   *
   * @param account the account data
   * @param date the reconciliation date as provided through the dialog
   * @param startingBalance the starting balance as provided through the dialog
   * @param endingBalance the ending balance as provided through the dialog
   * @param transactionList reference to QList of QPair containing all
   *        transaction/split pairs processed by the reconciliation.
   */
  void accountReconciled(const MyMoneyAccount& account, const QDate& date, const MyMoneyMoney& startingBalance, const MyMoneyMoney& endingBalance, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& transactionList);

  void viewStateChanged(bool);
  void kmmFilePlugin(unsigned int);
};

} // namespace
#endif
