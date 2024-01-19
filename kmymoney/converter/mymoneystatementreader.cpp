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

#include "mymoneystatementreader.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QList>
#include <QStringList>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KMessageBox>
#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "accountsmodel.h"
#include "amountedit.h"
#include "dialogenums.h"
#include "existingtransactionmatchfinder.h"
#include "kaccountselectdlg.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneymvccombo.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "knewaccountwizard.h"
#include "knewinvestmentwizard.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "mymoneystatement.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyutils.h"
#include "payeesmodel.h"
#include "scheduledtransactionmatchfinder.h"
#include "transactionmatcher.h"

#include "kmmyesno.h"

using namespace eMyMoney;

bool matchNotEmpty(const QString &l, const QString &r)
{
    return !l.isEmpty() && QString::compare(l, r, Qt::CaseInsensitive) == 0;
}

Q_GLOBAL_STATIC(QStringList, globalResultMessages);

class MyMoneyStatementReader::Private
{
public:
    Private() :
        transactionsCount(0),
        transactionsAdded(0),
        transactionsMatched(0),
        transactionsDuplicate(0),
        m_skipCategoryMatching(true),
        m_progressCallback(nullptr),
        scannedCategories(false) {}

    const QString& feeId(const MyMoneyAccount& invAcc);
    const QString& interestId(const MyMoneyAccount& invAcc);
    QString interestId(const QString& name);
    QString expenseId(const QString& name);
    QString feeId(const QString& name);
    void assignUniqueBankID(MyMoneySplit& s, const MyMoneyStatement::Transaction& t_in);
    void setupPrice(MyMoneySplit &s, const MyMoneyAccount &splitAccount, const MyMoneyAccount &transactionAccount, const QDate &postDate);

    MyMoneyAccount                 lastAccount;
    MyMoneyAccount                 m_account;
    MyMoneyAccount m_brokerageAccount;
    QList<MyMoneyPayee>       payees;
    int                            transactionsCount;
    int                            transactionsAdded;
    int                            transactionsMatched;
    int                            transactionsDuplicate;
    QMap<QString, bool>            uniqIds;
    QMap<QString, MyMoneySecurity> securitiesBySymbol;
    QMap<QString, MyMoneySecurity> securitiesByName;
    bool                           m_skipCategoryMatching;
    void (*m_progressCallback)(int, int, const QString&);
    QDate m_oldestPostDate;

private:
    void scanCategories(QString& id, const MyMoneyAccount& invAcc, const MyMoneyAccount& parentAccount, const QString& defaultName);
    /**
     * This method tries to figure out the category to be used for fees and interest
     * from previous transactions in the given @a investmentAccount and returns the
     * ids of those categories in @a feesId and @a interestId. The last used category
     * will be returned.
     */
    void previouslyUsedCategories(const QString& investmentAccount, QString& feesId, QString& interestId);

    QString nameToId(const QString& name, const MyMoneyAccount& parent);

private:
    QString                        m_feeId;
    QString                        m_interestId;
    bool                           scannedCategories;
};


const QString& MyMoneyStatementReader::Private::feeId(const MyMoneyAccount& invAcc)
{
    scanCategories(m_feeId, invAcc, MyMoneyFile::instance()->expense(), i18n("_Fees"));
    return m_feeId;
}

const QString& MyMoneyStatementReader::Private::interestId(const MyMoneyAccount& invAcc)
{
    scanCategories(m_interestId, invAcc, MyMoneyFile::instance()->income(), i18n("_Dividend"));
    return m_interestId;
}

QString MyMoneyStatementReader::Private::nameToId(const QString& name, const MyMoneyAccount& parent)
{
    //  Adapted from KMyMoneyApp::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
    //  Needed to find/create category:sub-categories
    MyMoneyFile* file = MyMoneyFile::instance();

    QString id = file->categoryToAccount(name, Account::Type::Unknown);
    // if it does not exist, we have to create it
    if (id.isEmpty()) {
        MyMoneyAccount newAccount;
        MyMoneyAccount parentAccount = parent;
        newAccount.setName(name) ;
        int pos;
        // check for ':' in the name and use it as separator for a hierarchy
        while ((pos = newAccount.name().indexOf(MyMoneyFile::AccountSeparator)) != -1) {
            QString part = newAccount.name().left(pos);
            QString remainder = newAccount.name().mid(pos + 1);
            const MyMoneyAccount& existingAccount = file->subAccountByName(parentAccount, part);
            if (existingAccount.id().isEmpty()) {
                newAccount.setName(part);
                newAccount.setAccountType(parentAccount.accountType());
                file->addAccount(newAccount, parentAccount);
                parentAccount = newAccount;
            } else {
                parentAccount = existingAccount;
            }
            newAccount.setParentAccountId(QString());  // make sure, there's no parent
            newAccount.clearId();                       // and no id set for adding
            newAccount.removeAccountIds();              // and no sub-account ids
            newAccount.setName(remainder);
        }//end while
        newAccount.setAccountType(parentAccount.accountType());

        // make sure we have a currency. If none is assigned, we assume base currency
        if (newAccount.currencyId().isEmpty())
            newAccount.setCurrencyId(file->baseCurrency().id());

        file->addAccount(newAccount, parentAccount);
        id = newAccount.id();
    }
    return id;
}

QString MyMoneyStatementReader::Private::expenseId(const QString& name)
{
    MyMoneyAccount parent = MyMoneyFile::instance()->expense();
    return nameToId(name, parent);
}

QString MyMoneyStatementReader::Private::interestId(const QString& name)
{
    MyMoneyAccount parent = MyMoneyFile::instance()->income();
    return nameToId(name, parent);
}

QString MyMoneyStatementReader::Private::feeId(const QString& name)
{
    MyMoneyAccount parent = MyMoneyFile::instance()->expense();
    return nameToId(name, parent);
}

void MyMoneyStatementReader::Private::previouslyUsedCategories(const QString& investmentAccount, QString& feesId, QString& interestId)
{
    feesId.clear();
    interestId.clear();
    MyMoneyFile* file = MyMoneyFile::instance();
    try {
        MyMoneyAccount acc = file->account(investmentAccount);
        MyMoneyTransactionFilter filter(investmentAccount);
        filter.setReportAllSplits(false);
        // since we assume an investment account here, we need to collect the stock accounts as well
        filter.addAccount(acc.accountList());
        QList< QPair<MyMoneyTransaction, MyMoneySplit> > list;
        file->transactionList(list, filter);
        QList< QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it_t;
        for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
            const MyMoneyTransaction& t = (*it_t).first;
            MyMoneySplit s = (*it_t).second;

            acc = file->account(s.accountId());
            // stock split shouldn't be fee or interest because it won't play nice with dissectTransaction
            // it was caused by processTransactionEntry adding splits in wrong order != with manual transaction entering
            if (acc.accountGroup() == Account::Type::Expense || acc.accountGroup() == Account::Type::Income) {
                const auto splits = t.splits();
                for (const auto& sNew : splits) {
                    acc = file->account(sNew.accountId());
                    if (acc.accountGroup() != Account::Type::Expense && // shouldn't be fee
                            acc.accountGroup() != Account::Type::Income &&  // shouldn't be interest
                            ((sNew.value() != sNew.shares()) ||                 // shouldn't be checking account...
                             (sNew.price() != MyMoneyMoney::ONE))) {            // ...but sometimes it may look like checking account
                        s = sNew;
                        break;
                    }
                }
            }

            MyMoneySplit assetAccountSplit;
            QList<MyMoneySplit> feeSplits;
            QList<MyMoneySplit> interestSplits;
            MyMoneySecurity security;
            MyMoneySecurity currency;
            eMyMoney::Split::InvestmentTransactionType transactionType;
            MyMoneyUtils::dissectTransaction(t, s, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
            if (!feeSplits.isEmpty()) {
                feesId = feeSplits.first().accountId();
                if (!interestId.isEmpty())
                    break;
            }
            if (!interestSplits.isEmpty()) {
                interestId = interestSplits.first().accountId();
                if (!feesId.isEmpty())
                    break;
            }
        }
    } catch (const MyMoneyException &) {
    }

}

void MyMoneyStatementReader::Private::scanCategories(QString& id, const MyMoneyAccount& invAcc, const MyMoneyAccount& parentAccount, const QString& defaultName)
{
    if (!scannedCategories) {
        previouslyUsedCategories(invAcc.id(), m_feeId, m_interestId);
        scannedCategories = true;
    }

    if (id.isEmpty()) {
        MyMoneyFile* file = MyMoneyFile::instance();
        MyMoneyAccount acc = file->accountByName(defaultName);
        // if it does not exist, we have to create it
        if (acc.id().isEmpty()) {
            MyMoneyAccount parent = parentAccount;
            acc.setName(defaultName);
            acc.setAccountType(parent.accountType());
            acc.setCurrencyId(parent.currencyId());
            file->addAccount(acc, parent);
        }
        id = acc.id();
    }
}

void MyMoneyStatementReader::Private::assignUniqueBankID(MyMoneySplit& s, const MyMoneyStatement::Transaction& t_in)
{
    QString base(t_in.m_strBankID);

    if (base.isEmpty()) {
        // in case the importer did not assign a bankID, we will do it here
        // we use the same algorithm as used in the KBanking plugin as this
        // has been served well for a long time already.
        auto h = MyMoneyTransaction::hash(t_in.m_strPayee.trimmed());
        h = MyMoneyTransaction::hash(t_in.m_strMemo, h);
        h = MyMoneyTransaction::hash(t_in.m_amount.toString(), h);
        base = QString("%1-%2-%3").arg(s.accountId(), t_in.m_datePosted.toString(Qt::ISODate)).arg(h, 7, 16, QChar('0'));
    }

    // make sure that id's are unique from this point on by appending a -#
    // postfix if needed
    QString hash(base);
    int idx = 1;
    for (;;) {
        QMap<QString, bool>::const_iterator it;
        it = uniqIds.constFind(hash);
        if (it == uniqIds.constEnd()) {
            uniqIds[hash] = true;
            break;
        }
        hash = QString("%1-%2").arg(base).arg(idx);
        ++idx;
    }

    s.setBankID(hash);
}

void MyMoneyStatementReader::Private::setupPrice(MyMoneySplit &s, const MyMoneyAccount &splitAccount, const MyMoneyAccount &transactionAccount, const QDate &postDate)
{
    if (transactionAccount.currencyId() != splitAccount.currencyId()) {
        // a currency conversion is needed assume that split has already a proper value
        MyMoneyFile* file = MyMoneyFile::instance();
        MyMoneySecurity toCurrency = file->security(splitAccount.currencyId());
        MyMoneySecurity fromCurrency = file->security(transactionAccount.currencyId());
        // get the price for the transaction's date
        const MyMoneyPrice &price = file->price(fromCurrency.id(), toCurrency.id(), postDate);
        // if the price is valid calculate the shares
        if (price.isValid()) {
            const int fract = splitAccount.fraction(toCurrency);
            const MyMoneyMoney &shares = s.value() * price.rate(toCurrency.id());
            s.setShares(shares.convert(fract));
            qDebug("Setting second split shares to %s", qPrintable(s.shares().formatMoney(toCurrency.id(), 2)));
        } else {
            qDebug("No price entry was found to convert from '%s' to '%s' on '%s'",
                   qPrintable(fromCurrency.tradingSymbol()), qPrintable(toCurrency.tradingSymbol()), qPrintable(postDate.toString(Qt::ISODate)));
        }
    }
}

MyMoneyStatementReader::MyMoneyStatementReader() :
    d(new Private),
    m_userAbort(false),
    m_autoCreatePayee(false),
    m_ft(0),
    m_progressCallback(0)
{
    m_askPayeeCategory = KMyMoneySettings::askForPayeeCategory();
}

MyMoneyStatementReader::~MyMoneyStatementReader()
{
    delete d;
}

bool MyMoneyStatementReader::anyTransactionAdded() const
{
    return (d->transactionsAdded != 0) ? true : false;
}

void MyMoneyStatementReader::setAutoCreatePayee(bool create)
{
    m_autoCreatePayee = create;
}

void MyMoneyStatementReader::setAskPayeeCategory(bool ask)
{
    m_askPayeeCategory = ask;
}

QStringList MyMoneyStatementReader::importStatement(const QString& url, bool silent, void(*callback)(int, int, const QString&))
{
    QStringList summary;
    MyMoneyStatement s;
    if (MyMoneyStatement::readXMLFile(s, url))
        summary = MyMoneyStatementReader::importStatement(s, silent, callback);
    else
        KMessageBox::error(nullptr, i18n("Error importing %1: This file is not a valid KMM statement file.", url), i18n("Invalid Statement"));

    return summary;
}

QStringList MyMoneyStatementReader::importStatement(const MyMoneyStatement& s, bool silent, void(*callback)(int, int, const QString&))
{
    auto result = false;

    // keep a copy of the statement
    if (KMyMoneySettings::logImportedStatements()) {
        auto logFile = QString::fromLatin1("%1/kmm-statement-%2.txt")
                           .arg(KMyMoneySettings::logPath(), QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddThh-mm-ss.zzz")));
        MyMoneyStatement::writeXMLFile(s, logFile);
    }

    auto reader = new MyMoneyStatementReader;
    reader->setAutoCreatePayee(true);
    if (callback)
        reader->setProgressCallback(callback);

    QStringList messages;
    result = reader->import(s, messages);

    auto transactionAdded = reader->anyTransactionAdded();

    delete reader;

    if (callback)
        callback(-1, -1, QString());

    if (!silent && transactionAdded) {
        globalResultMessages()->append(messages);
    }

    if (!result)
        messages.clear();
    return messages;
}

bool MyMoneyStatementReader::import(const MyMoneyStatement& s, QStringList& messages)
{
    //
    // Select the account
    //

    d->m_account = MyMoneyAccount();
    d->m_brokerageAccount = MyMoneyAccount();

    d->m_skipCategoryMatching = s.m_skipCategoryMatching;

    // if the statement source left some information about
    // the account, we use it to get the current data of it
    if (!s.m_accountId.isEmpty()) {
        try {
            d->m_account = MyMoneyFile::instance()->account(s.m_accountId);
        } catch (const MyMoneyException &) {
            qDebug("Received reference '%s' to unknown account in statement", qPrintable(s.m_accountId));
        }
    }

    if (d->m_account.id().isEmpty()) {
        d->m_account.setName(s.m_strAccountName);
        d->m_account.setNumber(s.m_strAccountNumber);

        switch (s.m_eType) {
        case eMyMoney::Statement::Type::Checkings:
            d->m_account.setAccountType(Account::Type::Checkings);
            break;
        case eMyMoney::Statement::Type::Savings:
            d->m_account.setAccountType(Account::Type::Savings);
            break;
        case eMyMoney::Statement::Type::Investment:
            //testing support for investment statements!
            //m_userAbort = true;
            //KMessageBox::error(kmymoney, i18n("This is an investment statement.  These are not supported currently."), i18n("Critical Error"));
            d->m_account.setAccountType(Account::Type::Investment);
            break;
        case eMyMoney::Statement::Type::CreditCard:
            d->m_account.setAccountType(Account::Type::CreditCard);
            break;
        default:
            d->m_account.setAccountType(Account::Type::Unknown);
            break;
        }


        // we ask the user only if we have some transactions to process
        if (!m_userAbort && s.m_listTransactions.count() > 0)
            m_userAbort = ! selectOrCreateAccount(Select, d->m_account);
    }

    // open an engine transaction
    m_ft = new MyMoneyFileTransaction();

    // see if we need to update some values stored with the account
    // take into account, that an older statement might get here again

    const auto statementEndDate = s.statementEndDate();
    const auto statementBalance = s.m_closingBalance;
    const auto previousStatementEndDate = QDate::fromString(d->m_account.value("lastImportedTransactionDate"), Qt::ISODate);
    const auto previousStatementBalance = MyMoneyMoney(d->m_account.value("lastStatementBalance"));

    // in case balance or date differs
    if ((previousStatementBalance != statementBalance) || (previousStatementEndDate != statementEndDate)) {
        // we only update if we have a valid statement date and the previous statement date is empty or older
        if (statementEndDate.isValid() && (!previousStatementEndDate.isValid() || (statementEndDate >= previousStatementEndDate))) {
            d->m_account.setValue("lastImportedTransactionDate", statementEndDate.toString(Qt::ISODate));
            if (!statementBalance.isAutoCalc()) {
                d->m_account.setValue("lastStatementBalance", statementBalance.toString());
            } else {
                d->m_account.deletePair("lastStatementBalance");
            }

            try {
                MyMoneyFile::instance()->modifyAccount(d->m_account);
            } catch (const MyMoneyException&) {
                qDebug("Updating account in MyMoneyStatementReader::startImport failed");
            }
        }
    }

    if (!d->m_account.name().isEmpty())
        messages += i18n("Importing statement for account %1", d->m_account.name());
    else if (s.m_listTransactions.count() == 0)
        messages += i18n("Importing statement without transactions");

    qDebug("Importing statement for '%s'", qPrintable(d->m_account.name()));

    //
    // Determine oldest transaction date
    // (we will use that as opening date for security accounts)
    //
    d->m_oldestPostDate = QDate::currentDate();
    for (const auto& transaction : s.m_listTransactions) {
        if (transaction.m_datePosted < d->m_oldestPostDate) {
            d->m_oldestPostDate = transaction.m_datePosted;
        }
    }

    //
    // Process the securities
    //
    signalProgress(0, s.m_listSecurities.count(), "Importing Statement ...");
    int progress = 0;
    QList<MyMoneyStatement::Security>::const_iterator it_s = s.m_listSecurities.begin();
    while (it_s != s.m_listSecurities.end()) {
        processSecurityEntry(*it_s);
        signalProgress(++progress, 0);
        ++it_s;
    }
    signalProgress(-1, -1);

    //
    // Process the transactions
    //

    if (!m_userAbort) {
        try {
            qDebug("Processing transactions (%s)", qPrintable(d->m_account.name()));
            signalProgress(0, s.m_listTransactions.count(), "Importing Statement ...");
            progress = 0;
            QList<MyMoneyStatement::Transaction>::const_iterator it_t = s.m_listTransactions.begin();
            while (it_t != s.m_listTransactions.end() && !m_userAbort) {
                processTransactionEntry(*it_t);
                signalProgress(++progress, 0);
                ++it_t;
            }
            qDebug("Processing transactions done (%s)", qPrintable(d->m_account.name()));

        } catch (const MyMoneyException &e) {
            if (QString::fromLatin1(e.what()).contains("USERABORT"))
                m_userAbort = true;
            else
                qDebug("Caught exception from processTransactionEntry() not caused by USERABORT: %s", e.what());
        }
        signalProgress(-1, -1);
    }

    //
    // process price entries
    //
    if (!m_userAbort) {
        try {
            signalProgress(0, s.m_listPrices.count(), "Importing Statement ...");
            KMyMoneyUtils::processPriceList(s);
        } catch (const MyMoneyException &e) {
            if (QString::fromLatin1(e.what()).contains("USERABORT"))
                m_userAbort = true;
            else
                qDebug("Caught exception from processPriceEntry() not caused by USERABORT: %s", e.what());
        }
        signalProgress(-1, -1);
    }

    bool  rc = false;

    // delete all payees created in vain
    int payeeCount = d->payees.count();
    QList<MyMoneyPayee>::const_iterator it_p;
    for (it_p = d->payees.constBegin(); it_p != d->payees.constEnd(); ++it_p) {
        try {
            MyMoneyFile::instance()->removePayee(*it_p);
            --payeeCount;
        } catch (const MyMoneyException &) {
            // if we can't delete it, it must be in use which is ok for us
        }
    }

    if (s.m_closingBalance.isAutoCalc()) {
        messages += i18n("  Statement balance is not contained in statement.");
    } else {
        messages += i18n("  Statement balance on %1 is reported to be %2", s.m_dateEnd.toString(Qt::ISODate), s.m_closingBalance.formatMoney("", 2));
    }
    messages += i18n("  Transactions");
    messages += i18np("    %1 processed", "    %1 processed", d->transactionsCount);
    messages += i18ncp("x transactions have been added", "    %1 added", "    %1 added", d->transactionsAdded);
    messages += i18np("    %1 matched", "    %1 matched", d->transactionsMatched);
    messages += i18np("    %1 duplicate", "    %1 duplicates", d->transactionsDuplicate);
    messages += i18n("  Payees");
    messages += i18ncp("x transactions have been created", "    %1 created", "    %1 created", payeeCount);
    messages += QString();

    // remove the Don't ask again entries
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group(QString::fromLatin1("Notification Messages"));
    QStringList::ConstIterator it;

    for (it = m_dontAskAgain.constBegin(); it != m_dontAskAgain.constEnd(); ++it) {
        grp.deleteEntry(*it);
    }
    config->sync();
    m_dontAskAgain.clear();

    rc = !m_userAbort;

    // finish the transaction
    if (rc)
        m_ft->commit();
    delete m_ft;
    m_ft = 0;

    qDebug("Importing statement for '%s' done", qPrintable(d->m_account.name()));

    return rc;
}

void MyMoneyStatementReader::processSecurityEntry(const MyMoneyStatement::Security& sec_in)
{
    // For a security entry, we will just make sure the security exists in the
    // file. It will not get added to the investment account until it's called
    // for in a transaction.
    MyMoneyFile* file = MyMoneyFile::instance();

    // check if we already have the security
    // In a statement, we do not know what type of security this is, so we will
    // not use type as a matching factor.
    MyMoneySecurity security;
    QList<MyMoneySecurity> list = file->securityList();
    QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
    while (it != list.constEnd() && security.id().isEmpty()) {
        if (matchNotEmpty(sec_in.m_strSymbol, (*it).tradingSymbol()) || matchNotEmpty(sec_in.m_strId, (*it).value("kmm-security-id"))
            || matchNotEmpty(sec_in.m_strName, (*it).name())) {
            security = *it;
        }
        ++it;
    }

    // if the security was not found, we have to create it while not forgetting
    // to setup the type
    if (security.id().isEmpty()) {
        security.setName(sec_in.m_strName);
        security.setTradingSymbol(sec_in.m_strSymbol);
        security.setTradingCurrency(file->baseCurrency().id());
        security.setValue("kmm-security-id", sec_in.m_strId);
        security.setValue("kmm-online-source", "Yahoo Finance");
        security.setSecurityType(Security::Type::Stock);
        security.setSmallestAccountFraction(static_cast<int>(sec_in.m_smallestFraction.toDouble()));
        MyMoneyFileTransaction ft;
        try {
            file->addSecurity(security);
            ft.commit();
            qDebug() << "Created " << security.name() << " with id " << security.id();
        } catch (const MyMoneyException &e) {
            KMessageBox::error(0, i18n("Error creating security record: %1", QString::fromLatin1(e.what())), i18n("Error"));
        }
    } else {
        qDebug() << "Found " << security.name() << " with id " << security.id();
    }
}

void MyMoneyStatementReader::processTransactionEntry(const MyMoneyStatement::Transaction& statementTransactionUnderImport)
{
    MyMoneyFile* file = MyMoneyFile::instance();

    MyMoneyTransaction transactionUnderImport;

    QString dbgMsg;
    dbgMsg = QString("Process on: '%1', id: '%2', symbol: '%3', amount: '%4', fees: '%5'")
             .arg(statementTransactionUnderImport.m_datePosted.toString(Qt::ISODate))
             .arg(statementTransactionUnderImport.m_strBankID)
             .arg(statementTransactionUnderImport.m_strSymbol)
             .arg(statementTransactionUnderImport.m_amount.formatMoney("", 2))
             .arg(statementTransactionUnderImport.m_fees.formatMoney("", 2));
    qDebug("%s", qPrintable(dbgMsg));

    // mark it imported for the view
    transactionUnderImport.setImported();

    // TODO (Ace) We can get the commodity from the statement!!
    // Although then we would need UI to verify
    transactionUnderImport.setCommodity(d->m_account.currencyId());

    transactionUnderImport.setPostDate(statementTransactionUnderImport.m_datePosted);
    if (statementTransactionUnderImport.m_dateProcessed.isValid()) {
        transactionUnderImport.setEntryDate(statementTransactionUnderImport.m_dateProcessed);
    }
    transactionUnderImport.setMemo(statementTransactionUnderImport.m_strMemo);

    MyMoneySplit s1;
    MyMoneySplit s2;
    MyMoneySplit sFees;
    MyMoneySplit sBrokerage;

    s1.setMemo(statementTransactionUnderImport.m_strMemo);
    s1.setValue(statementTransactionUnderImport.m_amount + statementTransactionUnderImport.m_fees);
    s1.setShares(s1.value());
    s1.setNumber(statementTransactionUnderImport.m_strNumber);

    // set these values if a transfer split is needed at the very end.
    MyMoneyMoney transfervalue;

    // If the user has chosen to import into an investment account, determine the correct account to use
    MyMoneyAccount thisaccount = d->m_account;
    QString brokerageactid;

    if (thisaccount.accountType() == Account::Type::Investment) {
        // determine the brokerage account
        brokerageactid = d->m_account.value("kmm-brokerage-account");
        if (brokerageactid.isEmpty()) {
            brokerageactid = file->accountByName(statementTransactionUnderImport.m_strBrokerageAccount).id();
        }
        if (brokerageactid.isEmpty()) {
            brokerageactid = file->nameToAccount(statementTransactionUnderImport.m_strBrokerageAccount);
        }
        if (brokerageactid.isEmpty()) {
            brokerageactid = file->nameToAccount(thisaccount.brokerageName());
        }
        if (brokerageactid.isEmpty()) {
            brokerageactid = file->accountByName(thisaccount.brokerageName()).id();
        }
        if (brokerageactid.isEmpty()) {
            brokerageactid = SelectBrokerageAccount();
        }

        // find the security transacted, UNLESS this transaction didn't
        // involve any security.
        if ((statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::None)
                //  eaInterest transactions MAY have a security.
                //  && (t_in.m_eAction != MyMoneyStatement::Transaction::eaInterest)
                && (statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::Fees)) {
            // the correct account is the stock account which matches two criteria:
            // (1) it is a sub-account of the selected investment account, and
            // (2a) the symbol of the underlying security matches the security of the
            // transaction, or
            // (2b) the name of the security matches the name of the security of the transaction.

            // search through each subordinate account
            auto found = false;
            QString currencyid;
            const auto accountList = thisaccount.accountList();
            for (const auto& sAccount : accountList) {
                currencyid = file->account(sAccount).currencyId();
                auto security = file->security(currencyid);
                if (matchNotEmpty(statementTransactionUnderImport.m_strSymbol, security.tradingSymbol()) ||
                        matchNotEmpty(statementTransactionUnderImport.m_strSecurity, security.name())) {
                    thisaccount = file->account(sAccount);
                    found = true;
                    break;
                }
            }

            // If there was no stock account under the m_account investment account,
            // add one using the security.
            if (!found) {
                // The security should always be available, because the statement file
                // should separately list all the securities referred to in the file,
                // and when we found a security, we added it to the file.

                if (statementTransactionUnderImport.m_strSecurity.isEmpty()) {
                    KMessageBox::information(0, i18n("This imported statement contains investment transactions with no security.  These transactions will be ignored."), i18n("Security not found"), QString("BlankSecurity"));
                    return;
                } else {
                    MyMoneySecurity security;
                    QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
                    QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
                    while (it != list.constEnd() && security.id().isEmpty()) {
                        if (matchNotEmpty(statementTransactionUnderImport.m_strSymbol, (*it).tradingSymbol()) ||
                                matchNotEmpty(statementTransactionUnderImport.m_strSecurity, (*it).name())) {
                            security = *it;
                        }
                        ++it;
                    }
                    if (!security.id().isEmpty()) {
                        thisaccount = MyMoneyAccount();
                        thisaccount.setName(security.name());
                        thisaccount.setAccountType(Account::Type::Stock);
                        thisaccount.setCurrencyId(security.id());
                        thisaccount.setOpeningDate(d->m_oldestPostDate);
                        currencyid = thisaccount.currencyId();

                        file->addAccount(thisaccount, d->m_account);
                        qDebug() << Q_FUNC_INFO << ": created account " << thisaccount.id() << " for security " << statementTransactionUnderImport.m_strSecurity << " under account " << d->m_account.id();
                    }
                    // this security does not exist in the file.
                    else {
                        thisaccount = MyMoneyAccount();
                        thisaccount.setName(statementTransactionUnderImport.m_strSecurity);

                        qDebug() << Q_FUNC_INFO << ": opening new investment wizard for security " << statementTransactionUnderImport.m_strSecurity << " under account " << d->m_account.id();
                        KNewInvestmentWizard::newInvestment(thisaccount, d->m_account);
                    }
                }
            }
            // Don't update price if there is no price information contained in the transaction
            if (statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::CashDividend
                    && statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::Shrsin
                    && statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::Shrsout) {
                // update the price, while we're here.  in the future, this should be
                // an option
                const MyMoneyPrice &price = file->price(currencyid, transactionUnderImport.commodity(), statementTransactionUnderImport.m_datePosted, true);
                if (!price.isValid()  && ((!statementTransactionUnderImport.m_amount.isZero() && !statementTransactionUnderImport.m_shares.isZero()) || !statementTransactionUnderImport.m_price.isZero())) {
                    MyMoneyPrice newprice;
                    if (!statementTransactionUnderImport.m_price.isZero()) {
                        newprice = MyMoneyPrice(currencyid, transactionUnderImport.commodity(), statementTransactionUnderImport.m_datePosted,
                                                statementTransactionUnderImport.m_price.abs(), i18n("Statement Importer"));
                    } else {
                        newprice = MyMoneyPrice(currencyid, transactionUnderImport.commodity(), statementTransactionUnderImport.m_datePosted,
                                                (statementTransactionUnderImport.m_amount / statementTransactionUnderImport.m_shares).abs(), i18n("Statement Importer"));
                    }
                    file->addPrice(newprice);
                }
            }
        }
        s1.setAccountId(thisaccount.id());
        d->assignUniqueBankID(s1, statementTransactionUnderImport);

        if (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::ReinvestDividend) {
            s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend));
            s1.setShares(statementTransactionUnderImport.m_shares);

            if (!statementTransactionUnderImport.m_price.isZero()) {
                s1.setPrice(statementTransactionUnderImport.m_price);
            } else {
                if (statementTransactionUnderImport.m_shares.isZero()) {
                    KMessageBox::information(0, i18n("This imported statement contains investment transactions with no share amount.  These transactions will be ignored."), i18n("No share amount provided"), QString("BlankAmount"));
                    return;
                }
                MyMoneyMoney total = -statementTransactionUnderImport.m_amount - statementTransactionUnderImport.m_fees;
                s1.setPrice(MyMoneyMoney((total / statementTransactionUnderImport.m_shares).convertPrecision(file->security(thisaccount.currencyId()).pricePrecision())));
            }

            s2.setMemo(statementTransactionUnderImport.m_strMemo);
            if (statementTransactionUnderImport.m_strInterestCategory.isEmpty())
                s2.setAccountId(d->interestId(thisaccount));
            else
                s2.setAccountId(d->interestId(statementTransactionUnderImport.m_strInterestCategory));

            s2.setShares(-statementTransactionUnderImport.m_amount - statementTransactionUnderImport.m_fees);
            s2.setValue(s2.shares());
        } else if (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::CashDividend) {
            // Cash dividends require setting 2 splits to get all of the information
            // in.  Split #2 will be the income split, and we'll set it to the first
            // income account.  This is a hack, but it's needed in order to get the
            // amount into the transaction.

            if (statementTransactionUnderImport.m_strInterestCategory.isEmpty())
                s2.setAccountId(d->interestId(thisaccount));
            else {//  Ensure category sub-accounts are dealt with properly
                s2.setAccountId(d->interestId(statementTransactionUnderImport.m_strInterestCategory));
            }
            s2.setShares(-statementTransactionUnderImport.m_amount - statementTransactionUnderImport.m_fees);
            s2.setValue(-statementTransactionUnderImport.m_amount - statementTransactionUnderImport.m_fees);

            // Split 1 will be the zero-amount investment split that serves to
            // mark this transaction as a cash dividend and note which stock account
            // it belongs to and which already contains the correct id and bankId
            s1.setMemo(statementTransactionUnderImport.m_strMemo);
            s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Dividend));
            s1.setShares(MyMoneyMoney());
            s1.setValue(MyMoneyMoney());

            /*  at this point any fees have been taken into account already
             *  so don't deduct them again.
             *  BUG 322381
             */
            transfervalue = statementTransactionUnderImport.m_amount;
        } else if (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Interest) {
            if (statementTransactionUnderImport.m_strInterestCategory.isEmpty())
                s2.setAccountId(d->interestId(thisaccount));
            else {//  Ensure category sub-accounts are dealt with properly
                if (statementTransactionUnderImport.m_amount.isPositive())
                    s2.setAccountId(d->interestId(statementTransactionUnderImport.m_strInterestCategory));
                else
                    s2.setAccountId(d->expenseId(statementTransactionUnderImport.m_strInterestCategory));
            }
            s2.setShares(-statementTransactionUnderImport.m_amount - statementTransactionUnderImport.m_fees);
            s2.setValue(-statementTransactionUnderImport.m_amount - statementTransactionUnderImport.m_fees);

            /// ***********   Add split as per Div       **********
            // Split 1 will be the zero-amount investment split that serves to
            // mark this transaction as a cash dividend and note which stock account
            // it belongs to.
            s1.setMemo(statementTransactionUnderImport.m_strMemo);
            s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::InterestIncome));
            s1.setShares(MyMoneyMoney());
            s1.setValue(MyMoneyMoney());
            transfervalue = statementTransactionUnderImport.m_amount;

        } else if (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Fees) {
            if (statementTransactionUnderImport.m_strInterestCategory.isEmpty())
                s1.setAccountId(d->feeId(thisaccount));
            else//  Ensure category sub-accounts are dealt with properly
                s1.setAccountId(d->feeId(statementTransactionUnderImport.m_strInterestCategory));
            s1.setShares(statementTransactionUnderImport.m_amount);
            s1.setValue(statementTransactionUnderImport.m_amount);

            transfervalue = statementTransactionUnderImport.m_amount;

        } else if ((statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Buy) ||
                   (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Sell)) {
            s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares));
            if (!statementTransactionUnderImport.m_price.isZero())  {
                s1.setPrice(statementTransactionUnderImport.m_price.abs());
            } else if (!statementTransactionUnderImport.m_shares.isZero()) {
                MyMoneyMoney total = statementTransactionUnderImport.m_amount + statementTransactionUnderImport.m_fees.abs();
                s1.setPrice(MyMoneyMoney((total / statementTransactionUnderImport.m_shares).abs().convertPrecision(file->security(thisaccount.currencyId()).pricePrecision())));
            }
            if (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Buy)
                s1.setShares(statementTransactionUnderImport.m_shares.abs());
            else
                s1.setShares(-statementTransactionUnderImport.m_shares.abs());
            s1.setValue(-(statementTransactionUnderImport.m_amount + statementTransactionUnderImport.m_fees.abs()));
            transfervalue = statementTransactionUnderImport.m_amount;

        } else if ((statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Shrsin) ||
                   (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Shrsout)) {
            s1.setValue(MyMoneyMoney());
            s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::AddShares));
            if (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::Shrsin) {
                s1.setShares(statementTransactionUnderImport.m_shares.abs());
            } else {
                s1.setShares(-(statementTransactionUnderImport.m_shares.abs()));
            }
        } else if (statementTransactionUnderImport.m_eAction == eMyMoney::Transaction::Action::None) {
            // User is attempting to import a non-investment transaction into this
            // investment account.  This is not supportable the way KMyMoney is
            // written.  However, if a user has an associated brokerage account,
            // we can stuff the transaction there.

            brokerageactid = d->m_account.value("kmm-brokerage-account");
            if (brokerageactid.isEmpty()) {
                brokerageactid = file->accountByName(d->m_account.brokerageName()).id();
            }
            if (! brokerageactid.isEmpty()) {
                s1.setAccountId(brokerageactid);
                d->assignUniqueBankID(s1, statementTransactionUnderImport);

                // Needed to satisfy the bankid check below.
                thisaccount = file->account(brokerageactid);
            } else {
                // Warning!! Your transaction is being thrown away.
            }
        }
        if (!statementTransactionUnderImport.m_fees.isZero()) {
            sFees.setMemo(i18n("(Fees) %1", statementTransactionUnderImport.m_strMemo));
            sFees.setValue(statementTransactionUnderImport.m_fees);
            sFees.setShares(statementTransactionUnderImport.m_fees);
            sFees.setAccountId(d->feeId(thisaccount));
        }
    } else {
        // For non-investment accounts, just use the selected account
        // Note that it is perfectly reasonable to import an investment statement into a non-investment account
        // if you really want.  The investment-specific information, such as number of shares and action will
        // be discarded in that case.
        s1.setAccountId(d->m_account.id());
        d->assignUniqueBankID(s1, statementTransactionUnderImport);
    }

    const auto importedPayeeName = statementTransactionUnderImport.m_strPayee;
    if (!importedPayeeName.isEmpty()) {
        qDebug() << QLatin1String("Start matching payee") << importedPayeeName;
        QString payeeid;
        try {
            QList<MyMoneyPayee> pList = file->payeeList();
            QList<MyMoneyPayee>::const_iterator it_p;
            QMap<int, QString> matchMap;
            for (it_p = pList.constBegin(); it_p != pList.constEnd(); ++it_p) {
                bool ignoreCase;
                QStringList keys;
                QStringList::const_iterator it_s;
                const auto matchType = (*it_p).matchData(ignoreCase, keys);
                switch (matchType) {
                case eMyMoney::Payee::MatchType::Disabled:
                    break;

                case eMyMoney::Payee::MatchType::Name:
                case eMyMoney::Payee::MatchType::NameExact:
                    keys << QString("%1").arg(QRegularExpression::escape((*it_p).name()));
                    if(matchType == eMyMoney::Payee::MatchType::NameExact) {
                        keys.clear();
                        keys << QString("^%1$").arg(QRegularExpression::escape((*it_p).name()));
                    }
                // intentional fall through

                case eMyMoney::Payee::MatchType::Key:
                    for (it_s = keys.constBegin(); it_s != keys.constEnd(); ++it_s) {
                        QRegularExpression exp(*it_s, ignoreCase ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption);
                        QRegularExpressionMatch match(exp.match(importedPayeeName));
                        if (match.hasMatch()) {
                            qDebug() << "Found match with" << importedPayeeName << "on" << (*it_p).name() << "for" << match.capturedLength();
                            matchMap[match.capturedLength()] = (*it_p).id();
                        }
                    }
                    break;
                }
            }

            // at this point we can have several scenarios:
            // a) multiple matches
            // b) a single match
            // c) no match at all
            //
            // for c) we just do nothing, for b) we take the one we found
            // in case of a) we take the one with the largest matchedLength()
            // which happens to be the last one in the map
            if (matchMap.count() > 1) {
                qDebug("Multiple matches");
                QMap<int, QString>::const_iterator it_m = matchMap.constEnd();
                --it_m;
                payeeid = *it_m;
            } else if (matchMap.count() == 1) {
                qDebug("Single matches");
                payeeid = *(matchMap.constBegin());
            }

            // if we did not find a matching payee, we throw an exception and try to create it
            if (payeeid.isEmpty())
                throw MYMONEYEXCEPTION_CSTRING("payee not matched");

            s1.setPayeeId(payeeid);

            // in case the payee name differs from the match and the memo has
            // not been changed then keep the original payee name which may contain
            // some details which are otherwise lost
            const auto payee = file->payeesModel()->itemById(payeeid);
            if ((s1.memo() == transactionUnderImport.memo()) && (payee.name().toLower() != importedPayeeName.toLower())) {
                s1.setMemo(i18nc("Prepend name of payee (%1) to original memo (%2)", "Original payee: %1\n%2", importedPayeeName, s1.memo()));
            }
        } catch (const MyMoneyException &) {
            MyMoneyPayee payee;
            int rc = KMessageBox::PrimaryAction;

            if (m_autoCreatePayee == false) {
                // Ask the user if that is what he intended to do?
                QString msg = i18n("Do you want to add \"%1\" as payee/receiver?\n\n", importedPayeeName);
                msg += i18n(
                    "Selecting \"Yes\" will create the payee, \"No\" will skip "
                    "creation of a payee record and remove the payee information "
                    "from this transaction. Selecting \"Cancel\" aborts the import "
                    "operation.\n\nIf you select \"No\" here and mark the \"Do not ask "
                    "again\" checkbox, the payee information for all following transactions "
                    "referencing \"%1\" will be removed.",
                    importedPayeeName);

                QString askKey = QString("Statement-Import-Payee-") + importedPayeeName;
                if (!m_dontAskAgain.contains(askKey)) {
                    m_dontAskAgain += askKey;
                }
                rc = KMessageBox::questionTwoActionsCancel(0,
                                                           msg,
                                                           i18n("New payee/receiver"),
                                                           KMMYesNo::yes(),
                                                           KMMYesNo::no(),
                                                           KStandardGuiItem::cancel(),
                                                           askKey);
            }

            if (rc == KMessageBox::PrimaryAction) {
                // for now, we just add the payee to the pool and turn
                // on simple name matching, so that future transactions
                // with the same name don't get here again.
                //
                // In the future, we could open a dialog and ask for
                // all the other attributes of the payee, but since this
                // is called in the context of an automatic procedure it
                // might distract the user.
                payee.setName(importedPayeeName);
                payee.setMatchData(eMyMoney::Payee::MatchType::Key, true, QStringList() << QString("^%1$").arg(QRegularExpression::escape(importedPayeeName)));
                if (m_askPayeeCategory) {
                    // We use a QPointer because the dialog may get deleted
                    // during exec() if the parent of the dialog gets deleted.
                    // In that case the guarded ptr will reset to 0.
                    QPointer<QDialog> dialog = new QDialog;
                    dialog->setWindowTitle(i18n("Default Category for Payee"));
                    dialog->setModal(true);

                    QWidget *mainWidget = new QWidget;
                    QVBoxLayout *topcontents = new QVBoxLayout(mainWidget);

                    //add in caption? and account combo here
                    QLabel* const label1 = new QLabel(i18n("Please select a default category for payee '%1'", importedPayeeName));
                    topcontents->addWidget(label1);

                    auto filterProxyModel = new AccountNamesFilterProxyModel(this);
                    filterProxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
                    filterProxyModel->setHideZeroBalancedEquityAccounts(KMyMoneySettings::hideZeroBalanceEquities());
                    filterProxyModel->setHideZeroBalancedAccounts(KMyMoneySettings::hideZeroBalanceAccounts());
                    filterProxyModel->addAccountGroup(QVector<Account::Type> {Account::Type::Asset, Account::Type::Liability, Account::Type::Equity, Account::Type::Income, Account::Type::Expense});

                    filterProxyModel->setSourceModel(MyMoneyFile::instance()->accountsModel());
                    filterProxyModel->sort(AccountsModel::Column::AccountName);

                    QPointer<KMyMoneyAccountCombo> accountCombo = new KMyMoneyAccountCombo(filterProxyModel);
                    topcontents->addWidget(accountCombo);
                    mainWidget->setLayout(topcontents);
                    QVBoxLayout *mainLayout = new QVBoxLayout;

                    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::No|QDialogButtonBox::Yes);
                    dialog->setLayout(mainLayout);
                    mainLayout->addWidget(mainWidget);
                    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
                    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
                    mainLayout->addWidget(buttonBox);
                    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), KGuiItem(i18n("Save Category")));
                    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), KGuiItem(i18n("No Category")));
                    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KGuiItem(i18n("Abort")));

                    int result = dialog->exec();

                    QString accountId;
                    if (accountCombo && !accountCombo->getSelected().isEmpty()) {
                        accountId = accountCombo->getSelected();
                    }
                    delete dialog;
                    //if they hit yes instead of no, then grab setting of account combo
                    if (result == QDialog::Accepted) {
                        payee.setDefaultAccountId(accountId);
                    } else if (result != QDialog::Rejected) {
                        //add cancel button? and throw exception like below
                        throw MYMONEYEXCEPTION_CSTRING("USERABORT");
                    }
                }

                try {
                    file->addPayee(payee);
                    qDebug("Payee '%s' created", qPrintable(payee.name()));
                    d->payees << payee;
                    payeeid = payee.id();
                    s1.setPayeeId(payeeid);

                } catch (const MyMoneyException &e) {
                    KMessageBox::detailedError(nullptr, i18n("Unable to add payee/receiver"), QString::fromLatin1(e.what()));

                }

            } else if (rc == KMessageBox::SecondaryAction) {
                s1.setPayeeId(QString());

            } else {
                throw MYMONEYEXCEPTION_CSTRING("USERABORT");
            }
        }

        if (thisaccount.accountType() != Account::Type::Stock) {
            //
            // Fill in other side of the transaction (category/etc) based on payee
            //
            // Note, this logic is lifted from KLedgerView::slotPayeeChanged(),
            // however this case is more complicated, because we have an amount and
            // a memo.  We just don't have the other side of the transaction.
            //
            // We'll search for the most recent transaction in this account with
            // this payee.  If this reference transaction is a simple 2-split
            // transaction, it's simple.  If it's a complex split, and the amounts
            // are different, we have a problem.  Somehow we have to balance the
            // transaction.  For now, we'll leave it unbalanced, and let the user
            // handle it.
            //
            const MyMoneyPayee payeeObj = MyMoneyFile::instance()->payee(payeeid);
            if (statementTransactionUnderImport.m_listSplits.isEmpty() && !payeeObj.defaultAccountId().isEmpty()) {
                MyMoneyAccount splitAccount = file->account(payeeObj.defaultAccountId());
                MyMoneySplit s;
                s.setReconcileFlag(eMyMoney::Split::State::Cleared);
                s.clearId();
                s.setBankID(QString());
                s.setShares(-s1.shares());
                s.setValue(-s1.value());
                s.setAccountId(payeeObj.defaultAccountId());
                s.setMemo(transactionUnderImport.memo());
                s.setPayeeId(payeeid);
                d->setupPrice(s, splitAccount, d->m_account, statementTransactionUnderImport.m_datePosted);
                transactionUnderImport.addSplit(s);

            } else if (statementTransactionUnderImport.m_listSplits.isEmpty() && !d->m_skipCategoryMatching) {
                MyMoneyTransactionFilter filter(thisaccount.id());
                filter.addPayee(payeeid);
                QList<MyMoneyTransaction> list;
                file->transactionList(list, filter);
                if (!list.empty()) {
                    // Default to using the most recent transaction as the reference
                    MyMoneyTransaction t_old = list.last();

                    // if there is more than one matching transaction, try to be a little
                    // smart about which one we use.  we scan them all and check if
                    // we find an exact match or use the one with the closest value

                    if (list.count() > 1) {
                        QList<MyMoneyTransaction>::ConstIterator it_trans = list.constEnd();
                        MyMoneyMoney minDiff;
                        do {
                            --it_trans;
                            MyMoneySplit s = (*it_trans).splitByAccount(thisaccount.id());
                            if (s.value() == s1.value()) {
                                // in case of an exact match, we won't get better and we can stop.
                                // keep searching if this transaction references a closed account
                                if (!MyMoneyFile::instance()->referencesClosedAccount(*it_trans)) {
                                    t_old = *it_trans;
                                    break;
                                }
                            } else {
                                MyMoneyMoney newDiff = (s.value() - s1.value()).abs();
                                if (newDiff < minDiff || minDiff.isZero()) {
                                    // keep it if it matches better than the current match
                                    // but only if it does not reference a closed account
                                    if (!MyMoneyFile::instance()->referencesClosedAccount(*it_trans)) {
                                        minDiff = newDiff;
                                        t_old = *it_trans;
                                    }
                                }
                            }
                        } while (it_trans != list.constBegin());
                    }

                    // Only copy the splits if the transaction found does not reference a closed account
                    if (!MyMoneyFile::instance()->referencesClosedAccount(t_old)) {
                        // special care must be taken, if the old transaction references
                        // a category and vat account combination that is not effective
                        // anymore due to a change in the tax category. If that is the
                        // case, we simply don't add the old tax category to the
                        // transaction and let MyMoneyFile::updateVAT handle the correct
                        // addition of a tax split. Since we don't know the order of
                        // the splits we scan over all of them and update the
                        // transactionUnderImport at the end of the loop.
                        MyMoneySplit categorySplit;
                        MyMoneySplit taxSplit;
                        QString newVatAccountId;
                        QString oldVatAccountId;

                        for (const auto& split : t_old.splits()) {
                            // We don't need the split that covers this account,
                            // we just need the other ones.
                            if (split.accountId() != thisaccount.id()) {
                                MyMoneySplit s(split);
                                s.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                                s.clearId();
                                s.setBankID(QString());
                                s.removeMatch();

                                // in case the old transaction has two splits
                                // we simply inverse the amount of the current
                                // transaction found in s1. In other cases (more
                                // than two splits we copy all splits and don't
                                // modify the splits. This may lead to unbalanced
                                // transactions which the user has to fix manually
                                if (t_old.splits().count() == 2) {
                                    s.setShares(-s1.shares());
                                    s.setValue(-s1.value());
                                    s.setMemo(s1.memo());
                                }
                                MyMoneyAccount splitAccount = file->account(s.accountId());
                                qDebug("Adding second split to %s(%s)",
                                       qPrintable(splitAccount.name()),
                                       qPrintable(s.accountId()));
                                d->setupPrice(s, splitAccount, d->m_account, statementTransactionUnderImport.m_datePosted);
                                transactionUnderImport.addSplit(s);

                                // check for vat categories
                                if (t_old.splits().count() == 3) {
                                    if (!splitAccount.value(QLatin1String("VatAccount")).isEmpty()) {
                                        newVatAccountId = splitAccount.value(QLatin1String("VatAccount"));
                                        categorySplit = s;
                                    } else {
                                        taxSplit = s;
                                        oldVatAccountId = split.accountId();
                                    }
                                }
                            }
                        }

                        // now check if we have to remove the tax split. This is
                        // the case when newVatAccountId is set and differs from
                        // oldVatAccountId.
                        if (!newVatAccountId.isEmpty()) {
                            if (newVatAccountId.compare(oldVatAccountId)) {
                                // remove the tax split
                                transactionUnderImport.removeSplit(taxSplit);
                                // and update the value of the remaining split
                                categorySplit.setShares(-s1.shares());
                                categorySplit.setValue(-s1.value());
                                transactionUnderImport.modifySplit(categorySplit);
                            }
                        }
                    }
                }
            }
        }
    }

    s1.setReconcileFlag(statementTransactionUnderImport.m_reconcile);

    // Add the 'account' split if it's needed
    if (! transfervalue.isZero()) {
        // in case the transaction has a reference to the brokerage account, we use it
        // but if brokerageactid has already been set, keep that.
        if (!statementTransactionUnderImport.m_strBrokerageAccount.isEmpty() && brokerageactid.isEmpty()) {
            brokerageactid = file->nameToAccount(statementTransactionUnderImport.m_strBrokerageAccount);
        }
        if (brokerageactid.isEmpty()) {
            brokerageactid = file->accountByName(statementTransactionUnderImport.m_strBrokerageAccount).id();
        }
//  There is no BrokerageAccount so have to nowhere to put this split.
        if (!brokerageactid.isEmpty()) {
            sBrokerage.setMemo(statementTransactionUnderImport.m_strMemo);
            sBrokerage.setValue(transfervalue);
            sBrokerage.setShares(transfervalue);
            sBrokerage.setAccountId(brokerageactid);
            sBrokerage.setReconcileFlag(statementTransactionUnderImport.m_reconcile);
            MyMoneyAccount splitAccount = file->account(sBrokerage.accountId());
            d->setupPrice(sBrokerage, splitAccount, d->m_account, statementTransactionUnderImport.m_datePosted);
        }
    }

    if (!(sBrokerage == MyMoneySplit()))
        transactionUnderImport.addSplit(sBrokerage);

    if (!(sFees == MyMoneySplit()))
        transactionUnderImport.addSplit(sFees);

    if (!(s2 == MyMoneySplit()))
        transactionUnderImport.addSplit(s2);

    transactionUnderImport.addSplit(s1);

    // check if we need to add/update a VAT assignment
    file->updateVAT(transactionUnderImport);

    if ((statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::ReinvestDividend) && (statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::CashDividend) && (statementTransactionUnderImport.m_eAction != eMyMoney::Transaction::Action::Interest)
       ) {
        //******************************************
        //                   process splits
        //******************************************

        QList<MyMoneyStatement::Split>::const_iterator it_s;
        for (it_s = statementTransactionUnderImport.m_listSplits.begin(); it_s != statementTransactionUnderImport.m_listSplits.end(); ++it_s) {
            MyMoneySplit s3;
            s3.setAccountId((*it_s).m_accountId);
            MyMoneyAccount acc = file->account(s3.accountId());
            s3.setPayeeId(s1.payeeId());
            s3.setMemo((*it_s).m_strMemo);
            s3.setShares((*it_s).m_amount);
            s3.setValue((*it_s).m_amount);
            s3.setReconcileFlag((*it_s).m_reconcile);
            d->setupPrice(s3, acc, d->m_account, statementTransactionUnderImport.m_datePosted);
            transactionUnderImport.addSplit(s3);
        }
    }

    // Add the transaction
    try {
        // check for matches already stored in the engine
        TransactionMatchFinder::MatchResult result;
        d->transactionsCount++;

        ExistingTransactionMatchFinder existingTrMatchFinder(KMyMoneySettings::matchInterval());
        result = existingTrMatchFinder.findMatch(transactionUnderImport, s1);
        if (result != TransactionMatchFinder::MatchNotFound) {
            MyMoneyTransaction matchedTransaction = existingTrMatchFinder.getMatchedTransaction();
            if (result == TransactionMatchFinder::MatchDuplicate
                    || !matchedTransaction.isImported()
                    || result == TransactionMatchFinder::MatchPrecise) { // don't match with just imported transaction
                MyMoneySplit matchedSplit = existingTrMatchFinder.getMatchedSplit();
                handleMatchingOfExistingTransaction(matchedTransaction, matchedSplit, transactionUnderImport, s1, result);
                return;
            }
        }

        addTransaction(transactionUnderImport);
        ScheduledTransactionMatchFinder scheduledTrMatchFinder(thisaccount, KMyMoneySettings::matchInterval());
        result = scheduledTrMatchFinder.findMatch(transactionUnderImport, s1);
        if (result != TransactionMatchFinder::MatchNotFound) {
            MyMoneySplit matchedSplit = scheduledTrMatchFinder.getMatchedSplit();
            MyMoneySchedule matchedSchedule = scheduledTrMatchFinder.getMatchedSchedule();

            handleMatchingOfScheduledTransaction(matchedSchedule, matchedSplit, transactionUnderImport, s1);
            return;
        }

    } catch (const MyMoneyException &e) {
        QString message(i18n("Problem adding or matching imported transaction with id '%1': %2", statementTransactionUnderImport.m_strBankID, e.what()));
        qDebug("%s", qPrintable(message));

        int result = KMessageBox::warningContinueCancel(0, message);
        if (result == KMessageBox::Cancel)
            throw MYMONEYEXCEPTION_CSTRING("USERABORT");
    }
}

QString MyMoneyStatementReader::SelectBrokerageAccount()
{
    if (d->m_brokerageAccount.id().isEmpty()) {
        d->m_brokerageAccount.setAccountType(Account::Type::Checkings);
        if (!m_userAbort)
            m_userAbort = ! selectOrCreateAccount(Select, d->m_brokerageAccount);
    }
    return d->m_brokerageAccount.id();
}

bool MyMoneyStatementReader::selectOrCreateAccount(const SelectCreateMode /*mode*/, MyMoneyAccount& account)
{
    bool result = false;

    MyMoneyFile* file = MyMoneyFile::instance();

    QString accountId;

    // Try to find an existing account in the engine which matches this one.
    // There are two ways to be a "matching account".  The account number can
    // match the statement account OR the "StatementKey" property can match.
    // Either way, we'll update the "StatementKey" property for next time.

    QString accountNumber = account.number();
    if (! accountNumber.isEmpty()) {
        // Get a list of all accounts
        QList<MyMoneyAccount> accounts;
        file->accountList(accounts);

        // Iterate through them
        QList<MyMoneyAccount>::const_iterator it_account = accounts.constBegin();
        while (it_account != accounts.constEnd()) {
            if (
                ((*it_account).value("StatementKey") == accountNumber) ||
                ((*it_account).number() == accountNumber)
            ) {
                MyMoneyAccount newAccount((*it_account).id(), account);
                account = newAccount;
                accountId = (*it_account).id();
                break;
            }

            ++it_account;
        }
    }

    // keep a copy for later use
    const QString originalAccountId(accountId);

    QString msg = i18n("<b>You have downloaded a statement for the following account:</b><br/><br/>");
    msg += i18n(" - Account Name: %1", account.name()) + "<br/>";
    msg += i18n(" - Account Type: %1", MyMoneyAccount::accountTypeToString(account.accountType())) + "<br/>";
    msg += i18n(" - Account Number: %1", account.number()) + "<br/>";
    msg += "<br/>";

    if (!account.name().isEmpty()) {
        if (!accountId.isEmpty())
            msg += i18n("Do you want to import transactions to this account?");

        else
            msg += i18n("KMyMoney cannot determine which of your accounts to use.  You can "
                        "create a new account by pressing the <b>Create</b> button "
                        "or select another one manually from the selection box below.");
    } else {
        msg += i18n("No account information has been found in the selected statement file. "
                    "Please select an account using the selection box in the dialog or "
                    "create a new account by pressing the <b>Create</b> button.");
    }

    eDialogs::Category type;
#if 0
    if (account.accountType() == Account::Type::Checkings) {
        type = eDialogs::Category::checking;
    } else if (account.accountType() == Account::Type::Savings) {
        type = eDialogs::Category::savings;
    } else if (account.accountType() == Account::Type::Investment) {
        type = eDialogs::Category::investment;
    } else if (account.accountType() == Account::Type::CreditCard) {
        type = eDialogs::Category::creditCard;
    } else {
        type = static_cast<eDialogs::Category>(eDialogs::Category::asset | eDialogs::Category::liability);
    }
#endif
    // FIXME: This is a quick fix to show all accounts in the account selection combo box
    // of the KAccountSelectDlg. This allows to select any asset or liability account during
    // statement import.
    // The real fix would be to detect the account type here
    // and add an option to show all accounts in the dialog.
    type = static_cast<eDialogs::Category>(eDialogs::Category::asset | eDialogs::Category::liability);

    QPointer<KAccountSelectDlg> accountSelect = new KAccountSelectDlg(type, "StatementImport", 0);
    connect(accountSelect, &KAccountSelectDlg::createAccount, this, &MyMoneyStatementReader::slotNewAccount);

    accountSelect->setHeader(i18n("Import transactions"));
    accountSelect->setDescription(msg);
    accountSelect->setAccount(account, accountId);
    accountSelect->setMode(false);
    accountSelect->showAbortButton(true);
    accountSelect->hideQifEntry();
    bool done = false;
    while (!done) {
        if (accountSelect->exec() == QDialog::Accepted && !accountSelect->selectedAccount().isEmpty()) {
            result = true;
            done = true;
            // update account data (current and previous)
            accountId = accountSelect->selectedAccount();
            account = file->account(accountId);
            MyMoneyAccount originalAccount;
            if (!originalAccountId.isEmpty()) {
                originalAccount = file->account(originalAccountId);
            }

            // if we have an account number and it differs
            // from the one we have as reference on file
            if (! accountNumber.isEmpty() && account.value("StatementKey") != accountNumber) {
                // update it on the account and remove it from the previous one
                account.setValue("StatementKey", accountNumber);
                originalAccount.deletePair(QLatin1String("StatementKey"));

                MyMoneyFileTransaction ft;
                try {
                    MyMoneyFile::instance()->modifyAccount(account);
                    if (!originalAccountId.isEmpty()) {
                        MyMoneyFile::instance()->modifyAccount(originalAccount);
                    }
                    ft.commit();
                } catch (const MyMoneyException &) {
                    qDebug("Updating account in MyMoneyStatementReader::selectOrCreateAccount failed");
                }
            }
        } else {
            if (accountSelect->aborted())
                //throw MYMONEYEXCEPTION_CSTRING("USERABORT");
                done = true;
            else
                KMessageBox::error(0, QLatin1String("<html>") + i18n("You must select an account, create a new one, or press the <b>Abort</b> button.") + QLatin1String("</html>"));
        }
    }
    delete accountSelect;

    return result;
}

const MyMoneyAccount& MyMoneyStatementReader::account() const {
    return d->m_account;
}

void MyMoneyStatementReader::setProgressCallback(void(*callback)(int, int, const QString&))
{
    m_progressCallback = callback;
}

void MyMoneyStatementReader::signalProgress(int current, int total, const QString& msg)
{
    if (m_progressCallback != 0)
        (*m_progressCallback)(current, total, msg);
}

void MyMoneyStatementReader::handleMatchingOfExistingTransaction(MyMoneyTransaction matchedTransaction,
                                                                 MyMoneySplit matchedSplit,
                                                                 MyMoneyTransaction& importedTransaction,
                                                                 const MyMoneySplit& importedSplit,
                                                                 const TransactionMatchFinder::MatchResult& matchResult)
{
    TransactionMatcher matcher;

    switch (matchResult) {
    case TransactionMatchFinder::MatchNotFound:
        break;
    case TransactionMatchFinder::MatchDuplicate:
        d->transactionsDuplicate++;
        qDebug() << "Detected transaction duplicate";
        break;
    case TransactionMatchFinder::MatchImprecise:
    case TransactionMatchFinder::MatchPrecise:
        addTransaction(importedTransaction);
        qDebug() << "Detected as match to transaction" << matchedTransaction.id();
        matcher.match(matchedTransaction, matchedSplit, importedTransaction, importedSplit, true);
        d->transactionsMatched++;
        break;
    }
}

void MyMoneyStatementReader::handleMatchingOfScheduledTransaction(MyMoneySchedule schedule,
                                                                  MyMoneySplit matchedSplit,
                                                                  const MyMoneyTransaction& importedTransaction,
                                                                  const MyMoneySplit& importedSplit)
{
    const auto file = MyMoneyFile::instance();

    if (askUserToEnterScheduleForMatching(schedule, importedSplit, importedTransaction)) {
        const auto origDueDate = schedule.nextDueDate();

        MyMoneyTransaction t = schedule.transaction();
        // in case the amounts of the scheduled transaction and the
        // imported transaction differ, we need to update the amount
        // using the transaction editor.
        if (matchedSplit.shares() != importedSplit.shares() && !schedule.isFixed()) {
            matchedSplit.setShares(importedSplit.shares());
            matchedSplit.setValue(importedSplit.value());
            t.modifySplit(matchedSplit);
            // don't forget to update the counter split
            if (t.splitCount() == 2) {
                for (const auto& split : t.splits()) {
                    if (split.id().compare(matchedSplit.id())) {
                        auto newSplit(split);
                        newSplit.setShares(-matchedSplit.shares());
                        newSplit.setValue(-matchedSplit.value());
                        t.modifySplit(newSplit);
                        break;
                    }
                }
            } else {
                file->updateVAT(t);
            }
        }

        MyMoneyFileTransaction ft;
        try {
            file->addTransaction(t);

            // we should not need this because addTransaction() does
            // update the data, but we want to stay on the safe side
            if (!t.id().isEmpty()) {
                t = MyMoneyFile::instance()->transaction(t.id());
                schedule.setLastPayment(t.postDate());
            }

            // in case the next due date is invalid, the schedule is finished
            // we mark it as such by setting the next due date to one day past the end
            QDate nextDueDate = schedule.nextPayment(origDueDate);
            if (!nextDueDate.isValid()) {
                schedule.setNextDueDate(schedule.endDate().addDays(1));
            } else {
                schedule.setNextDueDate(nextDueDate);
            }
            MyMoneyFile::instance()->modifySchedule(schedule);

            // now match the two transactions
            TransactionMatcher matcher;
            matcher.match(t, matchedSplit, importedTransaction, importedSplit);
            d->transactionsMatched++;

            ft.commit();

        } catch (const MyMoneyException& e) {
            QWidget* parent = QApplication::activeWindow();
            KMessageBox::detailedError(parent, i18n("Unable to enter scheduled transaction '%1'", schedule.name()), e.what());
        }
    }
}

void MyMoneyStatementReader::addTransaction(MyMoneyTransaction& transaction)
{
    MyMoneyFile* file = MyMoneyFile::instance();

    file->addTransaction(transaction);
    d->transactionsAdded++;
}

bool MyMoneyStatementReader::askUserToEnterScheduleForMatching(const MyMoneySchedule& matchedSchedule, const MyMoneySplit& importedSplit, const MyMoneyTransaction & importedTransaction) const
{
    QString scheduleName = matchedSchedule.name();
    int currencyDenom = d->m_account.fraction(MyMoneyFile::instance()->currency(d->m_account.currencyId()));
    QString splitValue = importedSplit.value().formatMoney(currencyDenom);
    QString payeeName = MyMoneyFile::instance()->payee(importedSplit.payeeId()).name();

    QString questionMsg = i18n("KMyMoney has found a scheduled transaction which matches an imported transaction.<br/>"
                               "Schedule name: <b>%1</b><br/>"
                               "Transaction: <i>%2 %3</i><br/>"
                               "Do you want KMyMoney to enter this schedule now so that the transaction can be matched?",
                               scheduleName, splitValue, payeeName);

    // check that dates are within user's setting
    const auto gap = static_cast<int>(qAbs(matchedSchedule.transaction().postDate().toJulianDay() - importedTransaction.postDate().toJulianDay()));
    if (gap > KMyMoneySettings::matchInterval())
        questionMsg = i18np("KMyMoney has found a scheduled transaction which matches an imported transaction.<br/>"
                            "Schedule name: <b>%2</b><br/>"
                            "Transaction: <i>%3 %4</i><br/>"
                            "The transaction dates are one day apart.<br/>"
                            "Do you want KMyMoney to enter this schedule now so that the transaction can be matched?",
                            "KMyMoney has found a scheduled transaction which matches an imported transaction.<br/>"
                            "Schedule name: <b>%2</b><br/>"
                            "Transaction: <i>%3 %4</i><br/>"
                            "The transaction dates are %1 days apart.<br/>"
                            "Do you want KMyMoney to enter this schedule now so that the transaction can be matched?",
                            gap,scheduleName, splitValue, payeeName);

    const int userAnswer = KMessageBox::questionTwoActions(0,
                                                           QLatin1String("<html>") + questionMsg + QLatin1String("</html>"),
                                                           i18n("Schedule found"),
                                                           KMMYesNo::yes(),
                                                           KMMYesNo::no());

    return (userAnswer == KMessageBox::PrimaryAction);
}

void MyMoneyStatementReader::slotNewAccount(const MyMoneyAccount& acc)
{
    auto newAcc = acc;
    NewAccountWizard::Wizard::newAccount(newAcc);
}

void MyMoneyStatementReader::clearResultMessages()
{
    globalResultMessages()->clear();
}

QStringList MyMoneyStatementReader::resultMessages()
{
    return *globalResultMessages();
}
