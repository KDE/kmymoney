/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYQIFREADER_H
#define MYMONEYQIFREADER_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QUrl>
#include <QFile>
#include <QProcess>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyqifprofile.h"
#include "mymoneytransaction.h"

class MyMoneyFileTransaction;
class MyMoneyStatement;
class MyMoneyQifProfile;

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
        EntrySkip,
    } QifEntryTypeE;

    struct qSplit {
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
    void setURL(const QUrl &url);

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
    bool startImport();

    void setCategoryMapping(bool map);

    inline const MyMoneyAccount& account() const {
        return m_account;
    };

    int statementCount() const;

    void setProgressCallback(void(*callback)(qint64, qint64, const QString&));

private:
    /**
      * This method is used to update the progress information. It
      * checks if an appropriate function is known and calls it.
      *
      * For a parameter description see KMyMoneyView::progressCallback().
      */
    void signalProgress(qint64 current, qint64 total, const QString& = "");

    /**
      * This method scans a transaction contained in
      * a QIF file formatted as an account record. This
      * format is used by MS-Money. If the specific data
      * is not found, then the data in the entry is treated
      * as a transaction. In this case, the user will be asked to
      * specify the account to which the transactions should be imported.
      * The entry data is found in m_qifEntry.
      *
      * @param accountType see MyMoneyAccount() for details. Defaults to eMyMoney::Account::Type::Checkings
      */
    void processMSAccountEntry(const eMyMoney::Account::Type accountType = eMyMoney::Account::Type::Checkings);

    /**
     * This method scans the m_qifEntry object as a payee record specified by Quicken
     */
    void processPayeeEntry();

    /**
      * This method scans the m_qifEntry object as an account record specified
      * by Quicken. In case @p resetAccountId is @p true (the default), the
      * global account id will be reset.
      *
      * The id of the account will be returned.
      */
    const QString processAccountEntry(bool resetAccountId = true);

    /**
      * This method scans the m_qifEntry object as a category record specified
      * by Quicken.
      */
    void processCategoryEntry();

    /**
      * This method scans the m_qifEntry object as a transaction record specified
      * by Quicken.
      */
    void processTransactionEntry();

    /**
      * This method scans the m_qifEntry object as an investment transaction
      * record specified by Quicken.
      */
    void processInvestmentTransactionEntry();

    /**
      * This method scans the m_qifEntry object as a price record specified
      * by Quicken.
      */
    void processPriceEntry();

    /**
      * This method scans the m_qifEntry object as a security record specified
      * by Quicken.
      */
    void processSecurityEntry();

    /**
      * This method processes the lines previously collected in
      * the member variable m_qifEntry. If further information
      * by the user is required to process the entry it will
      * be collected.
      */
    void processQifEntry();

    /**
     * This method process a line starting with an exclamation mark
     */
    void processQifSpecial(const QString& _line);

    /**
      * This method extracts the line beginning with the letter @p id
      * from the lines contained in the QStringList object @p m_qifEntry.
      * An empty QString is returned, if the line is not found.
      *
      * @param id QChar containing the letter to be found
      * @param cnt return cnt'th of occurrence of id in lines. cnt defaults to 1.
      *
      * @return QString with the remainder of the line or empty if
      *         @p id is not found in @p lines
      */
    const QString extractLine(const QChar& id, int cnt = 1);

    /**
      * This method examines each line in the QStringList object @p m_qifEntry,
      * searching for split entries, which it extracts into a struct qSplit and
      * stores all splits found in @p listqSplits .
      */
    bool extractSplits(QList<qSplit>& listqSplits) const;

    enum SelectCreateMode {
        Create = 0,
        Select,
    };

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
     * This methods returns the account from the list of accounts identified by
     * an account id or account name including an account hierarchy.
     *
     * The parent account specifies from which account the search should be started.
     * In case the parent account does not have an id, the method scans all top-level accounts.
     *
     * If the account is not found in the list of accounts, MyMoneyAccount() is returned.
     *
     * @param acc account to find
     * @param parent parent account to search from
     * @retval found MyMoneyAccount account instance
     * @retval MyMoneyAccount() if not found
     */
    MyMoneyAccount findAccount(const MyMoneyAccount& acc, const MyMoneyAccount& parent) const;

    /**
     * This method returns the account id for a given account @a name. In
     * case @a name references an investment account and @a useBrokerage is @a true
     * (the default), the id of the corresponding brokerage account will be
     * returned. In case an account does not exist, it will be created.
     */
    const QString transferAccount(const QString& name, bool useBrokerage = true);

    // void processQifLine();
    void createOpeningBalance(eMyMoney::Account::Type accType = eMyMoney::Account::Type::Checkings);

Q_SIGNALS:
    void statementsReady(const QList<MyMoneyStatement> &);

private Q_SLOTS:
    void slotSendDataToFilter();
    void slotReceivedDataFromFilter();
    void slotReceivedErrorFromFilter();
    void slotProcessData();

    /**
      * This slot is used to be informed about the end of the filtering process.
      * It emits the signal importFinished()
      */
    void slotImportFinished();


private:

    void parseReceivedData(const QByteArray& data);


    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;

    QProcess                m_filter;
    QString                 m_filename;
    QUrl                    m_url;
    MyMoneyQifProfile       m_qifProfile;
    MyMoneyAccount          m_account;
    unsigned long           m_transactionsSkipped;
    unsigned long           m_transactionsProcessed;
    QStringList             m_dontAskAgain;
    QMap<QString, QString>  m_accountTranslation;
    QMap<QString, QString>  m_investmentMap;
    QFile                   *m_file;
    char                    m_buffer[1024];
    QByteArray              m_lineBuffer;
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
    QList<MyMoneyTransaction> m_transactionCache;

    QList<QByteArray>  m_data;

    void (*m_progressCallback)(qint64, qint64, const QString&);

    MyMoneyFileTransaction* m_ft;
};

#endif
