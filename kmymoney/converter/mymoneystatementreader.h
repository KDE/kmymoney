/***************************************************************************
                          mymoneystatementreader
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSTATEMENTREADER_H
#define MYMONEYSTATEMENTREADER_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QObject>
#include <QString>
#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Headers

#include <ktemporaryfile.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifprofile.h"
#include "mymoneyaccount.h"
#include "mymoneystatement.h"

class MyMoneyFileTransaction;
class QStringList;

/**
  * This is a pared-down version of a MyMoneyQifReader object
  *
  * @author Ace Jones
  */
class MyMoneyStatementReader : public QObject
{
  Q_OBJECT

public:
  MyMoneyStatementReader();
  ~MyMoneyStatementReader();

  /**
    * This method imports data from the MyMoneyStatement object @a s
    * into the MyMoney engine. It leaves some statistical information
    * in the @a messages string list
    *
    * @retval true the import was processed successfully
    * @retval false the import resulted in a failure.
    */
  bool import(const MyMoneyStatement& s, QStringList& messages);

  /**
    * This method is used to modify the auto payee creation flag.
    * If this flag is set, records for payees that are not currently
    * found in the engine will be automatically created with no
    * further user interaction required. If this flag is no set,
    * the user will be asked if the payee should be created or not.
    * If the MyMoneyQifReader object is created auto payee creation
    * is turned off.
    *
    * @param create flag if this feature should be turned on (@p true)
    *               or turned off (@p false)
    */
  void setAutoCreatePayee(bool create);
  void setAskPayeeCategory(bool ask);

  const MyMoneyAccount& account() const {
    return m_account;
  };

  void setProgressCallback(void(*callback)(int, int, const QString&));

  /**
   * Returns true in case any transaction has been added to the engine
   * during the import of the statement. Only returns useful result
   * after import() has been called.
   */
  bool anyTransactionAdded(void) const;

private:
  /**
    * This method is used to update the progress information. It
    * checks if an appropriate function is known and calls it.
    *
    * For a parameter description see KMyMoneyView::progressCallback().
    */
  void signalProgress(int current, int total, const QString& = "");

  void processTransactionEntry(const MyMoneyStatement::Transaction& t_in);
  void processSecurityEntry(const MyMoneyStatement::Security& s_in);
  void processPriceEntry(const MyMoneyStatement::Price& p_in);

  enum SelectCreateMode {
    Create = 0,
    Select
  };
  /**
    * This method is used to find an account using the account's name
    * stored in @p account in the current MyMoneyFile object. If it does not
    * exist, the user has the chance to create it or to skip processing
    * of this account.
    *
    * Please see the documentation for this function in MyMoneyQifReader
    *
    * @param mode Is either Create or Select depending on the above table
    * @param account Reference to MyMoneyAccount object
    */
  bool selectOrCreateAccount(const SelectCreateMode mode, MyMoneyAccount& account);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
  MyMoneyAccount          m_account;
  QStringList             m_dontAskAgain;
  bool                    m_skipAccount;
  bool                    m_userAbort;
  bool                    m_autoCreatePayee;
  bool                    m_askPayeeCategory;
  MyMoneyFileTransaction* m_ft;

  void (*m_progressCallback)(int, int, const QString&);
};

#endif
