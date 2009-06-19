/***************************************************************************
                          mymoneyqifreader.h  -  description
                             -------------------
    begin                : Mon Jan 27 2003
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

#ifndef MYMONEYQIFREADER_H
#define MYMONEYQIFREADER_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QObject>
#include <QString>
#include <QStringList>
//Added by qt3to4:
#include <QByteArray>
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Headers

#include <ktemporaryfile.h>
#include <k3process.h>
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifprofile.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"

class MyMoneyFileTransaction;

/**
  * @author Thomas Baumgart
  */
class MyMoneyQifReader : public QObject
{
  Q_OBJECT
  friend class Private;

private:
  typedef enum {
    EntryUnknown = 0,
    EntryAccount,
    EntryTransaction,
    EntryCategory,
    EntryMemorizedTransaction,
    EntryInvestmentTransaction,
    EntrySecurity,
    EntryPrice,
    EntryPayee,
    EntryClass,
    EntrySkip
  } QifEntryTypeE;

  struct qSplit
  {
    QString      m_strCategoryName;
    QString      m_strMemo;
    QString      m_amount;
  };


public:
  MyMoneyQifReader();
  ~MyMoneyQifReader();

  /**
    * This method is used to store the filename into the object.
    * The file should exist. If it does and an external filter
    * program is specified with the current selected profile,
    * the file is send through this filter and the result
    * is stored in the m_tempFile file.
    *
    * @param url URL of the file to be imported
    */
  void setURL(const KUrl& url);

  /**
    * This method is used to store the name of the profile into the object.
    * The selected profile will be loaded if it exists. If an external
    * filter program is specified with the current selected profile,
    * the file is send through this filter and the result
    * is stored in the m_tempFile file.
    *
    * @param name QString reference to the name of the profile
    */
  void setProfile(const QString& name);

  /**
    * This method actually starts the import of data from the selected file
    * into the MyMoney engine.
    *
    * This method also starts the user defined import filter program
    * defined in the QIF profile. If none is defined, the file is read
    * as is (actually the UNIX command 'cat -' is used as the filter).
    *
    * If data from the filter program is available, the slot
    * slotReceivedDataFromFilter() will be called.
    *
    * Make sure to connect the signal importFinished() to detect when
    * the import actually ended. Call the method finishImport() to clean
    * things up and get the overall result of the import.
    *
    * @retval true the import was started successfully
    * @retval false the import could not be started.
    */
  bool startImport(void);

  /**
    * This method must be called once the signal importFinished() has
    * been emitted. It will clean up the reader state and determines
    * the actual return code of the import.
    *
    * @retval true Import was successful.
    * @retval false Import failed because the filter program terminated
    *               abnormally or the user aborted the import process.
    */
  bool finishImport(void);

  void setCategoryMapping(bool map);

  const MyMoneyAccount& account() const { return m_account; };

  void setProgressCallback(void(*callback)(int, int, const QString&));

private:
  /**
    * This method is used to update the progress information. It
    * checks if an appropriate function is known and calls it.
    *
    * For a parameter description see KMyMoneyView::progressCallback().
    */
  void signalProgress(int current, int total, const QString& = "");

  /**
    * This method scans a transaction contained in
    * a QIF file formatted as an account record. This
    * format is used by MS-Money. If the specific data
    * is not found, then the data in the entry is treated
    * as a transaction. In this case, the user will be asked to
    * specify the account to which the transactions should be imported.
    * The entry data is found in m_qifEntry.
    *
    * @param accountType see MyMoneyAccount() for details. Defaults to MyMoneyAccount::Checkings
    */
  void processMSAccountEntry(const MyMoneyAccount::accountTypeE accountType = MyMoneyAccount::Checkings);

  /**
   * This method scans the m_qifEntry object as a payee record specified by Quicken
   */
  void processPayeeEntry(void);

  /**
    * This method scans the m_qifEntry object as an account record specified
    * by Quicken. In case @p resetAccountId is @p true (the default), the
    * global account id will be reset.
    *
    * The id of the account will be returned.
    */
  QString processAccountEntry(bool resetAccountId = true);

  /**
    * This method scans the m_qifEntry object as a category record specified
    * by Quicken.
    */
  void processCategoryEntry(void);

  /**
    * This method scans the m_qifEntry object as a transaction record specified
    * by Quicken.
    */
  void processTransactionEntry(void);

  /**
    * This method scans the m_qifEntry object as an investment transaction
    * record specified by Quicken.
    */
  void processInvestmentTransactionEntry(void);

  /**
    * This method scans the m_qifEntry object as a price record specified
    * by Quicken.
    */
  void processPriceEntry(void);

  /**
    * This method scans the m_qifEntry object as a security record specified
    * by Quicken.
    */
  void processSecurityEntry(void);

  /**
    * This method processes the lines previously collected in
    * the member variable m_qifEntry. If further information
    * by the user is required to process the entry it will
    * be collected.
    */
  void processQifEntry(void);

  /**
   * This method process a line starting with an exclamation mark
   */
  void processQifSpecial(const QString& _line);

  /**
    * This method is used to get the account id of the split for
    * a transaction from the text found in the QIF $ or L record.
    * If an account with the name is not found, the user is asked
    * if it should be created.
    *
    * @param name name of account as found in the QIF file
    * @param value value found in the T record
    * @param value2 value found in the $ record for splitted transactions
    *
    * @return id of the account for the split. If no name is specified
    *            or the account was not found and not created the
    *            return value will be "".
    */
  QString checkCategory(const QString& name, const MyMoneyMoney value, const MyMoneyMoney value2);

  /**
    * This method extracts the line beginning with the letter @p id
    * from the lines contained in the QStringList object @p m_qifEntry.
    * An empty QString is returned, if the line is not found.
    *
    * @param id QChar containing the letter to be found
    * @param cnt return cnt'th of occurance of id in lines. cnt defaults to 1.
    *
    * @return QString with the remainder of the line or empty if
    *         @p id is not found in @p lines
    */
  const QString extractLine(const QChar id, int cnt = 1);

  /**
    * This method examines each line in the QStringList object @p m_qifEntry,
    * searching for split entries, which it extracts into a struct qSplit and
    * stores all splits found in @p listqSplits .
    */
  void extractSplits(Q3ValueList<qSplit>& listqSplits) const;

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
    * If an account has been selected, account will be set to contain it's data.
    * If the skip operation was requested, account will be empty.
    *
    * Depending on @p mode the bahaviour of this method is slightly different.
    * The following table shows the dependencies:
    *
    * @code
    * case                              mode            operation
    * -----------------------------------------------------------------------------
    * account with same name exists     Create          returns immediately
    *                                                   m_account contains data
    *                                                   of existing account
    *
    * account does not exist            Create          immediately calls dialog
    *                                                   to create account
    *
    * account with same name exists     Select          User will be asked if
    *                                                   he wants to use the existing
    *                                                   account or create a new one
    *
    * account does not exist            Select          User will be asked to
    *                                                   select a different account
    *                                                   or create a new one
    *
    * @endcode
    *
    * @param mode Is either Create or Select depending on the above table
    * @param account Reference to MyMoneyAccount object
    */

  void selectOrCreateAccount(const SelectCreateMode mode, MyMoneyAccount& account, const MyMoneyMoney& openingBalance = MyMoneyMoney());

  /**
    * This method looks up the @p searchname account by name and returns its id
    * if it was found.  If it was not found, it creates a new income account using
    * @p searchname as a name, and returns the id if the newly created account
    *
    * @param searchname The name of the account to find or create
    * @return QString id of the found or created account
    */
  static const QString findOrCreateIncomeAccount(const QString& searchname);

  /**
    * This method looks up the @p searchname account by name and returns its id
    * if it was found.  If it was not found, it creates a new expense account using
    * @p searchname as a name, and returns the id if the newly created account
    *
    * @param searchname The name of the account to find or create
    * @return QString id of the found or created account
    */
  static const QString findOrCreateExpenseAccount(const QString& searchname);

  /**
   * This method returns the account id for a given account @a name. In
   * case @a name references an investment account and @a useBrokerage is @a true
   * (the default), the id of the corresponding brokerage account will be
   * returned. In case an account is not existant, it will be created.
   */
  QString transferAccount(QString name, bool useBrokerage = true);

  // void processQifLine(void);
  void createOpeningBalance(MyMoneyAccount::_accountTypeE accType = MyMoneyAccount::Checkings);

signals:
  /**
    * This signal will be emitted when the import is finished.
    */
  void importFinished(void);

private slots:
  void slotSendDataToFilter(void);
  void slotReceivedDataFromFilter(K3Process* /* proc */, char *buff, int len);
  void slotReceivedErrorFromFilter(K3Process* /* proc */, char *buff, int len);
  // void slotReceivedDataFromFilter(void);
  // void slotReceivedErrorFromFilter(void);
  void slotProcessData(void);

  /**
    * This slot is used to be informed about the end of the filtering process.
    * It emits the signal importFinished()
    */
  void slotImportFinished(void);


private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

  K3Process                m_filter;
  QString                 m_filename;
  KUrl                    m_url;
  MyMoneyQifProfile       m_qifProfile;
  MyMoneyAccount          m_account;
  unsigned long           m_transactionsSkipped;
  unsigned long           m_transactionsProcessed;
  QStringList             m_dontAskAgain;
  QMap<QString, QString>  m_accountTranslation;
  QMap<QString, QString>  m_investmentMap;
  QFile                   *m_file;
  char                    m_buffer[1024];
  QByteArray                m_lineBuffer;
  QStringList             m_qifEntry;
  int                     m_extractedLine;
  QString                 m_qifLine;
  QStringList             m_qifLines;
  QifEntryTypeE           m_entryType;
  bool                    m_skipAccount;
  bool                    m_processingData;
  bool                    m_userAbort;
  bool                    m_autoCreatePayee;
  unsigned long           m_pos;
  unsigned                m_linenumber;
  bool                    m_warnedInvestment;
  bool                    m_warnedSecurity;
  bool                    m_warnedPrice;
  Q3ValueList<MyMoneyTransaction> m_transactionCache;

  Q3ValueList<QByteArray>  m_data;

  void (*m_progressCallback)(int, int, const QString&);

  MyMoneyFileTransaction* m_ft;
};

#endif
