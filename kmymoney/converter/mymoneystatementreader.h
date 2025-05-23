/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSTATEMENTREADER_H
#define MYMONEYSTATEMENTREADER_H

#include "qcontainerfwd.h"
#include <QObject>
#include <QString>
#include <QStringList>

#include "mymoneyenums.h"
#include "mymoneystatement.h"
#include "transactionmatchfinder.h"

class MyMoneyFileTransaction;
class TransactionMatcher;
class MyMoneyAccount;
class MyMoneyInstitution;
class StatementModel;

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
     * into the MyMoney engine.
     *
     * @retval true the import was processed successfully
     * @retval false the import resulted in a failure.
     */
    bool import(const MyMoneyStatement& s);

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
    void setPayeeCreationMode(eMyMoney::Account::PayeeCreation creationMode);

    void setAskPayeeCategory(bool ask);

    const MyMoneyAccount& account() const;

    void setProgressCallback(void(*callback)(int, int, const QString&));

    /**
     * Returns true in case any transaction has been added to the engine
     * during the import of the statement. Only returns useful result
     * after import() has been called.
     */
    bool anyTransactionAdded() const;

    /**
      * Imports a KMM statement into the engine, triggering the appropriate
      * UI to handle account matching, payee creation, and someday
      * payee and transaction matching.
      */
    static void importStatement(const QString& url);
    static bool importStatement(const MyMoneyStatement& s);

    static void clearImportResults();
    static StatementModel* importResultsModel();

private:
    void processTransactionEntry(const MyMoneyStatement::Transaction& t_in);
    void processSecurityEntry(const MyMoneyStatement::Security& s_in);

    enum SelectCreateMode {
        Create = 0,
        Select,
    };

    /**
      * This method is used to select or create brokerage account if
      * it isn't specified in statement transaction.
      */
    QString SelectBrokerageAccount();

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
    QStringList             m_dontAskAgain;
    bool                    m_userAbort;
    eMyMoney::Account::PayeeCreation m_payeeCreationMode;
    bool                    m_askPayeeCategory;
    MyMoneyFileTransaction* m_ft;

    void handleMatchingOfExistingTransaction(MyMoneyTransaction matchedTransaction,
                                             MyMoneySplit matchedSplit,
                                             MyMoneyTransaction& importedTransaction,
                                             const MyMoneySplit& importedSplit,
                                             const TransactionMatchFinder::MatchResult& matchResult);

    void handleMatchingOfScheduledTransaction(MyMoneySchedule matchedSchedule,
                                              MyMoneySplit matchedSplit,
                                              const MyMoneyTransaction& importedTransaction,
                                              const MyMoneySplit& importedSplit);
    void addTransaction(MyMoneyTransaction & transaction);

    /** Asks the user whether to enter a schedule transaction to match it with imported one
     * @param matchedSchedule the schedule which matches the imported transaction
     * @param importedSplit the split of the imported transaction which matches the split of the schedule
     * @return true, if user confirmed to enter the schedule to match it with imported transaction; false otherwise
     */
    bool askUserToEnterScheduleForMatching(const MyMoneySchedule& matchedSchedule, const MyMoneySplit& importedSplit, const MyMoneyTransaction & importedTransaction) const;

private Q_SLOTS:
    void slotNewAccount(const MyMoneyAccount& acc);
};

#endif
