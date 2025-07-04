/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyfile.h"

#include <utility>

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QBitArray>
#include <QDebug>
#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QTimer>
#include <QUuid>
#include <QtTest/QAbstractItemModelTester>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KDescendantsProxyModel>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneyaccountloan.h"
#include "mymoneybalancecache.h"
#include "mymoneybudget.h"
#include "mymoneycostcenter.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyforecast.h"
#include "mymoneyinstitution.h"
#include "mymoneypayee.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include "onlinejob.h"
#include "storageenums.h"

// the models
#include "accountsmodel.h"
#include "budgetsmodel.h"
#include "costcentermodel.h"
#include "institutionsmodel.h"
#include "journalmodel.h"
#include "onlinejobsmodel.h"
#include "parametersmodel.h"
#include "payeesmodel.h"
#include "pricemodel.h"
#include "reconciliationmodel.h"
#include "reportsmodel.h"
#include "schedulesjournalmodel.h"
#include "schedulesmodel.h"
#include "securitiesmodel.h"
#include "specialdatesmodel.h"
#include "statusmodel.h"
#include "tagsmodel.h"
/// @note add new models here

#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifiertyped.h"

// include the following line to get a 'cout' for debug purposes
// #include <iostream>

using namespace eMyMoney;

const QString MyMoneyFile::AccountSeparator = QChar(':');

typedef QList<std::pair<QString, QDate> > BalanceNotifyList;
typedef QMap<QString, bool> CacheNotifyList;

class MyMoneyNotification
{
public:

    MyMoneyNotification(File::Mode mode, File::Object objType, const QString& id) :
        m_objType(objType),
        m_notificationMode(mode),
        m_id(id) {
    }

    File::Object objectType() const {
        return m_objType;
    }
    File::Mode notificationMode() const {
        return m_notificationMode;
    }
    const QString& id() const {
        return m_id;
    }

protected:
    MyMoneyNotification(File::Object obj,
                        File::Mode mode,
                        const QString& id) :
        m_objType(obj),
        m_notificationMode(mode),
        m_id(id) {}

private:
    File::Object   m_objType;
    File::Mode     m_notificationMode;
    QString        m_id;
};

class KMM_MYMONEY_EXPORT MyMoneyFileUndoStack : public QUndoStack
{
    Q_OBJECT
public Q_SLOTS:
    void undo()
    {
        const auto file = MyMoneyFile::instance();
        const auto hasTransaction = file->hasTransaction();
        if (!hasTransaction) {
            Q_EMIT file->storageTransactionStarted(true);
        }
        QUndoStack::undo();
        if (!hasTransaction) {
            Q_EMIT file->storageTransactionEnded(true);
        }
    }
    void redo()
    {
        const auto file = MyMoneyFile::instance();
        const auto hasTransaction = file->hasTransaction();
        if (!hasTransaction) {
            Q_EMIT file->storageTransactionStarted(true);
        }
        QUndoStack::redo();
        if (!hasTransaction) {
            Q_EMIT file->storageTransactionEnded(true);
        }
    }
};

class MyMoneyFile::Private
{
public:
    Private(MyMoneyFile* qq)
        : m_file(qq)
        , m_dirty(false)
        , m_inTransaction(false)
        , m_journalBlocking(false)
        , payeesModel(qq, &undoStack)
        , userModel(qq, &undoStack)
        , costCenterModel(qq, &undoStack)
        , schedulesModel(qq, &undoStack)
        , tagsModel(qq, &undoStack)
        , securitiesModel(qq, &undoStack)
        , currenciesModel(qq, &undoStack)
        , budgetsModel(qq, &undoStack)
        , accountsModel(qq, &undoStack)
        , institutionsModel(&accountsModel, qq, &undoStack)
        , journalModel(qq, &undoStack)
        , priceModel(qq, &undoStack)
        , parametersModel(qq, &undoStack)
        , fileInfoModel(qq, &undoStack)
        , onlineJobsModel(qq, &undoStack)
        , reportsModel(qq, &undoStack)
        , specialDatesModel(qq, &undoStack)
        , schedulesJournalModel(qq, &undoStack)
        , statusModel(qq)
        /// @note add new models here
        , flatAccountsModel(qq)
    {
#ifdef KMM_MODELTEST
        new QAbstractItemModelTester(&payeesModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&userModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&costCenterModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&schedulesModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&tagsModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&securitiesModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&currenciesModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&budgetsModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&accountsModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&institutionsModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&journalModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&priceModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&parametersModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&fileInfoModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&onlineJobsModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&reportsModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&specialDatesModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        new QAbstractItemModelTester(&schedulesJournalModel, QAbstractItemModelTester::FailureReportingMode::Warning);
        /// @note add new models here
#endif
        flatAccountsModel.setSourceModel(&accountsModel);

        qq->connect(qq, &MyMoneyFile::modelsReadyToUse, &journalModel, &JournalModel::updateBalances);
        qq->connect(qq, &MyMoneyFile::modelsReadyToUse, qq, &MyMoneyFile::finalizeFileOpen);
        qq->connect(&journalModel, &JournalModel::balancesChanged, &accountsModel, &AccountsModel::updateAccountBalances);
        qq->connect(&schedulesModel, &SchedulesModel::dataChanged, &schedulesJournalModel, static_cast<void (SchedulesJournalModel::*)()>(&SchedulesJournalModel::updateData));
        qq->connect(&schedulesModel, &SchedulesModel::modelReset, &schedulesJournalModel, &SchedulesJournalModel::updateData);
        qq->connect(&schedulesModel, &SchedulesModel::rowsAboutToBeRemoved, &schedulesJournalModel, &SchedulesJournalModel::updateData);
        qq->connect(&accountsModel, &AccountsModel::reconciliationInfoChanged, &reconciliationModel, &ReconciliationModel::updateData);
        qq->connect(&accountsModel, &AccountsModel::reparentAccountRequest, qq, &MyMoneyFile::reparentAccountByIds);

        // provide access for schedules to calculate balances
        schedulesJournalModel.setJournalModel(&journalModel);
    }

    ~Private()
    {
    }

    bool anyModelDirty() const
    {
        return payeesModel.isDirty()
               || userModel.isDirty()
               || costCenterModel.isDirty()
               || schedulesModel.isDirty()
               || tagsModel.isDirty()
               || securitiesModel.isDirty()
               || currenciesModel.isDirty()
               || budgetsModel.isDirty()
               || accountsModel.isDirty()
               || institutionsModel.isDirty()
               || journalModel.isDirty()
               || priceModel.isDirty()
               || parametersModel.isDirty()
               || onlineJobsModel.isDirty()
               || reportsModel.isDirty()
               || specialDatesModel.isDirty();
        /// @note add new models here
    }

    void markModelsAsClean()
    {
        schedulesModel.setDirty(false);
        costCenterModel.setDirty(false);
        payeesModel.setDirty(false);
        userModel.setDirty(false);
        tagsModel.setDirty(false);
        securitiesModel.setDirty(false);
        currenciesModel.setDirty(false);
        budgetsModel.setDirty(false);
        accountsModel.setDirty(false);
        institutionsModel.setDirty(false);
        journalModel.setDirty(false);
        priceModel.setDirty(false);
        parametersModel.setDirty(false);
        onlineJobsModel.setDirty(false);
        reportsModel.setDirty(false);
        specialDatesModel.setDirty(false);
        schedulesJournalModel.setDirty(false);
        statusModel.setDirty(false);
        /// @note add new models here
    }

    /**
      * This method is used to add an id to the list of objects
      * to be removed from the cache. If id is empty, then nothing is added to the list.
      *
      * @param id id of object to be notified
      * @param reload reload the object (@c true) or not (@c false). The default is @c true
      * @see attach, detach
      */
    void addCacheNotification(const QString& id, const QDate& date) {
        if (!id.isEmpty())
            m_balanceNotifyList.append(std::make_pair(id, date));
    }

    /**
      * This method is used to clear the notification list
      */
    void clearCacheNotification() {
        // reset list to be empty
        m_balanceNotifyList.clear();
    }

    /**
      * This method is used to clear all
      * objects mentioned in m_notificationList from the cache.
      */
    void notify() {
        for (const BalanceNotifyList::value_type& i : qAsConst(m_balanceNotifyList)) {
            m_balanceChangedSet.insert(i.first);
            if (i.second.isValid()) {
                m_balanceCache.clear(i.first, i.second);
            } else {
                m_balanceCache.clear(i.first);
            }
        }

        clearCacheNotification();
    }

    /**
      * This method checks that a transaction has been started with
      * startTransaction() and throws an exception otherwise.
      */
    void checkTransaction(const char* txt) const {
        if (!m_inTransaction)
            throw MYMONEYEXCEPTION(QString::fromLatin1("No transaction started for %1").arg(QString::fromLatin1(txt)));
    }

    void priceChanged(const MyMoneyPrice price) {
        // get all affected accounts and add them to the m_valueChangedSet
        QList<MyMoneyAccount> accList;
        m_file->accountList(accList);
        QList<MyMoneyAccount>::const_iterator account_it;
        for (account_it = accList.cbegin(); account_it != accList.cend(); ++account_it) {
            QString currencyId = account_it->currencyId();
            if (currencyId != m_file->baseCurrency().id() && (currencyId == price.from() || currencyId == price.to())) {
                // this account is not in the base currency and the price affects it's value
                m_valueChangedSet.insert(account_it->id());
            }
        }
    }

    void adjustSplitSign(MyMoneyTransaction& transaction)
    {
        // in case the sign of shares and value of a split differs,
        // invert the sign of the value to make them equal
        const auto splits = transaction.splits();
        for (const auto& split : splits) {
            if (split.shares().isNegative() != split.value().isNegative()) {
                auto newSplit(split);
                newSplit.setValue(-newSplit.value());
                transaction.modifySplit(newSplit);
            }
        }
    }

    bool applyFileFixes(bool expertMode)
    {
        MyMoneyFileTransaction ft;
        try {
            // Check if we have to modify the file before we allow to work with it
            while (m_file->fileFixVersion() < availableFixVersion()) {
                qDebug() << "testing fileFixVersion" << m_file->fileFixVersion() << "<" << availableFixVersion();
                switch (m_file->fileFixVersion()) {
                case 0:
                    fixFile_0();
                    m_file->setFileFixVersion(1);
                    break;

                case 1:
                    fixFile_1(expertMode);
                    m_file->setFileFixVersion(2);
                    break;

                case 2:
                    fixFile_2();
                    m_file->setFileFixVersion(3);
                    break;

                case 3:
                    fixFile_3();
                    m_file->setFileFixVersion(4);
                    break;

                case 4:
                    fixFile_4();
                    m_file->setFileFixVersion(5);
                    break;

                case 5:
                    fixFile_5();
                    m_file->setFileFixVersion(6);
                    break;

                case 6:
                    fixFile_6();
                    m_file->setFileFixVersion(7);
                    break;

                case 7:
                    fixFile_7();
                    m_file->setFileFixVersion(8);
                    break;

                case 8:
                    fixFile_8();
                    m_file->setFileFixVersion(9);
                    break;

                // add new levels above. Don't forget to increase availableFixVersion() for all
                // the storage backends this fix applies to
                default:
                    throw MYMONEYEXCEPTION(QString::fromLatin1("Unknown fix level in input file"));
                }
            }
            ft.commit();
        } catch (const MyMoneyException&) {
            return false;
        }
        return true;
    }

    /* DO NOT ADD code to this function or any of it's called ones.
       Instead, create a new function, fixFile_n, and modify the applyFileFixes()
       logic above to call it */

    void fixFile_8()
    {
        const auto file = MyMoneyFile::instance();
        const auto model = file->reportsModel();
        const auto hasPrices = file->priceModel()->rowCount() > 0;

        // we only need to remove the price column if there are no prices
        if (!hasPrices) {
            auto count = 0;
            const auto rows = model->rowCount();
            for (int row = 0; row < rows; ++row) {
                const auto idx = model->index(row, 0);
                MyMoneyReport report = model->itemByIndex(idx);
                unsigned qc = report.queryColumns();
                if (!report.isInvestmentsOnly() && !report.isLoansOnly() && report.isConvertCurrency()) {
                    qc &= ~eMyMoney::Report::QueryColumn::Price;
                    report.setQueryColumns((eMyMoney::Report::QueryColumn)qc);
                    file->modifyReport(report);
                    ++count;
                }
            }
            qDebug() << count << "reports(s) fixed in" << __FUNCTION__;
        }
    }

    void fixFile_7()
    {
        const auto file = MyMoneyFile::instance();
        const auto model = file->reportsModel();

        // scan the reports and add price column if currency conversion is activated
        auto count = 0;
        const auto rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            MyMoneyReport report = model->itemByIndex(idx);
            unsigned qc = report.queryColumns();
            if (report.isInvestmentsOnly() || report.isLoansOnly()) {
                if (report.isConvertCurrency() && (qc & eMyMoney::Report::QueryColumn::Price)) {
                    qc &= ~eMyMoney::Report::QueryColumn::Price;
                    report.setQueryColumns((eMyMoney::Report::QueryColumn)qc);
                    file->modifyReport(report);
                    ++count;
                }
            } else if (report.isConvertCurrency() && !(qc & eMyMoney::Report::QueryColumn::Price)) {
                qc |= eMyMoney::Report::QueryColumn::Price;
                report.setQueryColumns((eMyMoney::Report::QueryColumn)qc);
                file->modifyReport(report);
                ++count;
            }
        }
        qDebug() << count << "reports(s) fixed in" << __FUNCTION__;
    }

    void fixFile_6()
    {
        const auto file = MyMoneyFile::instance();
        const auto model = file->reportsModel();

        // scan the reports and add price column if currency conversion is activated
        auto count = 0;
        const auto rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            MyMoneyReport report = model->itemByIndex(idx);
            unsigned qc = report.queryColumns();
            if (report.isConvertCurrency() && !(qc & eMyMoney::Report::QueryColumn::Price)) {
                qc |= eMyMoney::Report::QueryColumn::Price;
                report.setQueryColumns((eMyMoney::Report::QueryColumn)qc);
            }
            file->modifyReport(report);
            ++count;
        }
        qDebug() << count << "reports(s) fixed in" << __FUNCTION__;
    }

    void fixFile_5()
    {
        const auto file = MyMoneyFile::instance();
        const auto model = file->journalModel();

        // scan the transactions and modify investment transactions with
        // "Add/Remove shares" action and a price which differs from 0
        auto count = 0;
        const auto rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            const auto splitCount = idx.data(eMyMoney::Model::TransactionSplitCountRole).toInt();
            if (splitCount == 1) {
                MyMoneyTransaction transaction = model->itemByIndex(idx).transaction();
                MyMoneySplit split = transaction.splits().at(0);
                if (!split.price().isZero() && (split.actionStringToAction(split.action()) == eMyMoney::Split::Action::AddShares)) {
                    split.setPrice(MyMoneyMoney());
                    transaction.modifySplit(split);
                    file->modifyTransaction(transaction);
                    ++count;
                }
            }
            row += splitCount;
        }
        qDebug() << count << "transaction(s) fixed in" << __FUNCTION__;
    }

    void fixFile_4()
    {
        auto file = MyMoneyFile::instance();
        const QList<MyMoneySecurity> currencies = file->currencyList();
        static const QStringList symbols = {
            QStringLiteral("XAU"),
            QStringLiteral("XPD"),
            QStringLiteral("XPT"),
            QStringLiteral("XAG"),
        };

        for (auto currency : currencies) {
            if (symbols.contains(currency.id())) {
                if (currency.smallestAccountFraction() != currency.smallestCashFraction()) {
                    currency.setSmallestAccountFraction(currency.smallestCashFraction());
                    file->modifyCurrency(currency);
                }
            }
        }
    }

    void fixFile_3()
    {
        // make sure each storage object contains a (unique) id
        MyMoneyFile::instance()->storageId();
    }

    void fixFile_2()
    {
        auto file = MyMoneyFile::instance();
        MyMoneyTransactionFilter filter;
        filter.setReportAllSplits(false);
        QList<MyMoneyTransaction> transactionList;
        file->transactionList(transactionList, filter);

        // scan the transactions and modify transactions with two splits
        // which reference an account and a category to have the memo text
        // of the account.
        auto count = 0;
        for (const auto& transaction : qAsConst(transactionList)) {
            if (transaction.splitCount() == 2) {
                QString accountId;
                QString categoryId;
                QString accountMemo;
                QString categoryMemo;
                const auto splits = transaction.splits();
                for (const auto& split : splits) {
                    auto acc = file->account(split.accountId());
                    if (acc.isIncomeExpense()) {
                        categoryId = split.id();
                        categoryMemo = split.memo();
                    } else {
                        accountId = split.id();
                        accountMemo = split.memo();
                    }
                }

                if (!accountId.isEmpty() && !categoryId.isEmpty() && accountMemo != categoryMemo) {
                    MyMoneyTransaction t(transaction);
                    MyMoneySplit s(t.splitById(categoryId));
                    s.setMemo(accountMemo);
                    t.modifySplit(s);
                    file->modifyTransaction(t);
                    ++count;
                }
            }
        }
        qDebug("%d transactions fixed in fixFile_2", count);
    }

    void fixFile_1(bool expertMode)
    {
        // we need to fix reports. If the account filter list contains
        // investment accounts, we need to add the stock accounts to the list
        // as well if we don't have the expert mode enabled
        if (!expertMode) {
            try {
                QList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();
                QList<MyMoneyReport>::iterator it_r;
                for (it_r = reports.begin(); it_r != reports.end(); ++it_r) {
                    QStringList list;
                    (*it_r).accounts(list);
                    QStringList missing;
                    QStringList::const_iterator it_a;
                    for (it_a = list.cbegin(); it_a != list.cend(); ++it_a) {
                        const auto acc = MyMoneyFile::instance()->account(*it_a);
                        if (acc.accountType() == eMyMoney::Account::Type::Investment) {
                            const auto accountList = acc.accountList();
                            for (const auto& accountID : accountList) {
                                if (!list.contains(accountID)) {
                                    missing.append(accountID);
                                }
                            }
                        }
                    }
                    if (!missing.isEmpty()) {
                        (*it_r).addAccount(missing);
                        MyMoneyFile::instance()->modifyReport(*it_r);
                    }
                }
            } catch (const MyMoneyException&) {
            }
        }
    }

    void fixFile_0()
    {
        /* (Ace) I am on a crusade against file fixups.  Whenever we have to fix the
         * file, it is really a warning.  So I'm going to print a debug warning, and
         * then go track them down when I see them to figure out how they got saved
         * out needing fixing anyway.
         */

        auto file = MyMoneyFile::instance();
        QList<MyMoneyAccount> accountList;
        file->accountList(accountList);
        QList<MyMoneyAccount>::Iterator it_a;
        QList<MyMoneySchedule> scheduleList = file->scheduleList();
        QList<MyMoneySchedule>::Iterator it_s;

        MyMoneyAccount equity = file->equity();
        MyMoneyAccount asset = file->asset();
        bool equityListEmpty = equity.accountList().isEmpty();

        for (it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
            if ((*it_a).accountType() == eMyMoney::Account::Type::Loan || (*it_a).accountType() == eMyMoney::Account::Type::AssetLoan) {
                fixLoanAccount_0(*it_a);
            }
            // until early before 0.8 release, the equity account was not saved to
            // the file. If we have an equity account with no sub-accounts but
            // find and equity account that has equity() as it's parent, we reparent
            // this account. Need to move it to asset() first, because otherwise
            // MyMoneyFile::reparent would act as NOP.
            if (equityListEmpty && (*it_a).accountType() == eMyMoney::Account::Type::Equity) {
                if ((*it_a).parentAccountId() == equity.id()) {
                    auto acc = *it_a;
                    // tricky, force parent account to be empty so that we really
                    // can re-parent it
                    acc.setParentAccountId(QString());
                    file->reparentAccount(acc, equity);
                    qDebug() << Q_FUNC_INFO << " fixed account " << acc.id() << " reparented to " << equity.id();
                }
            }
        }

        for (it_s = scheduleList.begin(); it_s != scheduleList.end(); ++it_s) {
            fixSchedule_0(*it_s);
        }

        fixTransactions_0();
    }

    void fixSchedule_0(MyMoneySchedule sched)
    {
        MyMoneyTransaction t = sched.transaction();
        QList<MyMoneySplit> splitList = t.splits();
        QList<MyMoneySplit>::const_iterator it_s;

        try {
            bool updated = false;
            // Check if the splits contain valid data and set it to
            // be valid.
            for (it_s = splitList.cbegin(); it_s != splitList.cend(); ++it_s) {
                // the first split is always the account on which this transaction operates
                // and if the transaction commodity is not set, we take this
                if (it_s == splitList.cbegin() && t.commodity().isEmpty()) {
                    qDebug() << Q_FUNC_INFO << " " << t.id() << " has no commodity";
                    try {
                        auto acc = MyMoneyFile::instance()->account((*it_s).accountId());
                        t.setCommodity(acc.currencyId());
                        updated = true;
                    } catch (const MyMoneyException&) {
                    }
                }
                // make sure the account exists. If not, remove the split
                try {
                    MyMoneyFile::instance()->account((*it_s).accountId());
                } catch (const MyMoneyException&) {
                    qDebug() << Q_FUNC_INFO << " " << sched.id() << " " << (*it_s).id() << " removed, because account '" << (*it_s).accountId()
                             << "' does not exist.";
                    t.removeSplit(*it_s);
                    updated = true;
                }
                if ((*it_s).reconcileFlag() != eMyMoney::Split::State::NotReconciled) {
                    qDebug() << Q_FUNC_INFO << " " << sched.id() << " " << (*it_s).id() << " should be 'not reconciled'";
                    MyMoneySplit split = *it_s;
                    split.setReconcileDate(QDate());
                    split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                    t.modifySplit(split);
                    updated = true;
                }
                // the schedule logic used to operate only on the value field.
                // This is now obsolete.
                if ((*it_s).shares().isZero() && !(*it_s).value().isZero()) {
                    MyMoneySplit split = *it_s;
                    split.setShares(split.value());
                    t.modifySplit(split);
                    updated = true;
                }
            }

            // If there have been changes, update the schedule and
            // the engine data.
            if (updated) {
                sched.setTransaction(t);
                MyMoneyFile::instance()->modifySchedule(sched);
            }
        } catch (const MyMoneyException& e) {
            qWarning("Unable to update broken schedule: %s", e.what());
        }
    }

    void fixLoanAccount_0(MyMoneyAccount acc)
    {
        if (acc.value("final-payment").isEmpty() //
            || acc.value("term").isEmpty() //
            || acc.value("periodic-payment").isEmpty() //
            || acc.value("loan-amount").isEmpty() //
            || acc.value("interest-calculation").isEmpty() //
            || acc.value("schedule").isEmpty() //
            || acc.value("fixed-interest").isEmpty()) {
            throw MYMONEYEXCEPTION_CSTRING("Fix LoanAccount0 not supported anymore");
        }
    }

    void fixTransactions_0()
    {
        auto file = MyMoneyFile::instance();

        QList<MyMoneySchedule> scheduleList = file->scheduleList();
        MyMoneyTransactionFilter filter;
        filter.setReportAllSplits(false);
        QList<MyMoneyTransaction> transactionList;
        file->transactionList(transactionList, filter);

        QList<MyMoneySchedule>::Iterator it_x;
        QStringList interestAccounts;

        qDebug() << i18n("Fix transactions");

        int cnt = 0;
        // scan the schedules to find interest accounts
        for (it_x = scheduleList.begin(); it_x != scheduleList.end(); ++it_x) {
            MyMoneyTransaction t = (*it_x).transaction();
            QStringList accounts;
            bool hasDuplicateAccounts = false;

            const auto splits = t.splits();
            for (const auto& split : splits) {
                if (accounts.contains(split.accountId())) {
                    hasDuplicateAccounts = true;
                    qDebug() << Q_FUNC_INFO << " " << t.id() << " has multiple splits with account " << split.accountId();
                } else {
                    accounts << split.accountId();
                }

                if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
                    if (interestAccounts.contains(split.accountId()) == 0) {
                        interestAccounts << split.accountId();
                    }
                }
            }
            if (hasDuplicateAccounts) {
                fixDuplicateAccounts_0(t);
            }
            ++cnt;
            if (!(cnt % 10))
                qDebug() << cnt;
        }

        // scan the transactions and modify loan transactions
        for (auto& transaction : transactionList) {
            QString defaultAction;
            const QList<MyMoneySplit> splits = transaction.splits();
            QStringList accounts;

            // check if base commodity is set. if not, set baseCurrency
            if (transaction.commodity().isEmpty()) {
                qDebug() << Q_FUNC_INFO << " " << transaction.id() << " has no base currency";
                transaction.setCommodity(file->baseCurrency().id());
                file->modifyTransaction(transaction);
            }

            // Determine default action
            if (transaction.splitCount() == 2) {
                // check for transfer
                int accountCount = 0;
                bool isLoan = false;
                MyMoneyMoney val;
                for (const auto& split : splits) {
                    auto acc = file->account(split.accountId());
                    if (acc.accountGroup() == eMyMoney::Account::Type::Asset //
                        || acc.accountGroup() == eMyMoney::Account::Type::Liability) {
                        val = split.value();
                        accountCount++;
                        if (acc.accountType() == eMyMoney::Account::Type::Loan //
                            || acc.accountType() == eMyMoney::Account::Type::AssetLoan)
                            isLoan = true;
                    } else
                        break;
                }
                if (accountCount == 2) {
                    if (isLoan)
                        defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization);
                    else
                        defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer);
                } else {
                    if (val.isNegative())
                        defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal);
                    else
                        defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit);
                }
            }

            for (const auto& split : splits) {
                auto acc = file->account(split.accountId());
                MyMoneyMoney val = split.value();
                if (acc.accountGroup() == eMyMoney::Account::Type::Asset || acc.accountGroup() == eMyMoney::Account::Type::Liability) {
                    if (!val.isPositive()) {
                        defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal);
                        break;
                    } else {
                        defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit);
                        break;
                    }
                }
            }

            // Check for correct assignment of ActionInterest in all splits
            // and check if there are any duplicates in this transactions
            for (auto split : splits) {
                MyMoneyAccount splitAccount = file->account(split.accountId());
                if (!accounts.contains(split.accountId())) {
                    accounts << split.accountId();
                }
                // if this split references an interest account, the action
                // must be of type ActionInterest
                if (interestAccounts.contains(split.accountId())) {
                    if (split.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
                        qDebug() << Q_FUNC_INFO << " " << transaction.id() << " contains an interest account (" << split.accountId()
                                 << ") but does not have ActionInterest";
                        split.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Interest));
                        transaction.modifySplit(split);
                        file->modifyTransaction(transaction);
                        qDebug("Fixed interest action in %s", qPrintable(transaction.id()));
                    }
                    // if it does not reference an interest account, it must not be
                    // of type ActionInterest
                } else {
                    if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
                        qDebug() << Q_FUNC_INFO << " " << transaction.id() << " does not contain an interest account so it should not have ActionInterest";
                        split.setAction(defaultAction);
                        transaction.modifySplit(split);
                        file->modifyTransaction(transaction);
                        qDebug("Fixed interest action in %s", qPrintable(transaction.id()));
                    }
                }

                // check that for splits referencing an account that has
                // the same currency as the transactions commodity the value
                // and shares field are the same.
                if (transaction.commodity() == splitAccount.currencyId() && split.value() != split.shares()) {
                    qDebug() << Q_FUNC_INFO << " " << transaction.id() << " " << split.id() << " uses the transaction currency, but shares != value";
                    split.setShares(split.value());
                    transaction.modifySplit(split);
                    file->modifyTransaction(transaction);
                }

                // fix the shares and values to have the correct fraction
                if (!splitAccount.isInvest()) {
                    try {
                        int fract = splitAccount.fraction();
                        if (split.shares() != split.shares().convert(fract)) {
                            qDebug("adjusting fraction in %s,%s", qPrintable(transaction.id()), qPrintable(split.id()));
                            split.setShares(split.shares().convert(fract));
                            split.setValue(split.value().convert(fract));
                            transaction.modifySplit(split);
                            file->modifyTransaction(transaction);
                        }
                    } catch (const MyMoneyException&) {
                        qDebug("Missing security '%s', split not altered", qPrintable(splitAccount.currencyId()));
                    }
                }
            }

            ++cnt;
            if (!(cnt % 10))
                qDebug() << cnt;
        }
    }

    void fixDuplicateAccounts_0(const MyMoneyTransaction& t)
    {
        qDebug("Duplicate account in transaction %s", qPrintable(t.id()));
    }

    MyMoneyFile* m_file;
    bool m_dirty;
    bool m_inTransaction;
    bool m_journalBlocking;
    MyMoneySecurity m_baseCurrency;

    /**
     * @brief Cache for MyMoneyObjects
     *
     * It is also used to emit the objectAdded() and objectModified() signals.
     * => If one of these signals is used, you must use this cache.
     */
    MyMoneyPriceList       m_priceCache;
    MyMoneyBalanceCache    m_balanceCache;

    /**
      * This member keeps a list of account ids to notify
      * after a single operation is completed. The balance cache
      * is cleared for that account and all dates on or after
      * the one supplied. If the date is invalid, the entire
      * balance cache is cleared for that account.
      */
    BalanceNotifyList m_balanceNotifyList;

    /**
      * This member keeps a list of account ids for which
      * a balanceChanged() signal needs to be emitted when
      * a set of operations has been committed.
      *
      * @sa MyMoneyFile::commitTransaction()
      */
    KMMStringSet m_balanceChangedSet;

    /**
      * This member keeps a list of account ids for which
      * a valueChanged() signal needs to be emitted when
      * a set of operations has been committed.
      *
      * @sa MyMoneyFile::commitTransaction()
      */
    KMMStringSet m_valueChangedSet;

    /**
      * This member keeps the list of changes in the engine
      * in historical order. The type can be 'added', 'modified'
      * or removed.
      */
    QList<MyMoneyNotification> m_changeSet;

    // the engine's undo stack
    MyMoneyFileUndoStack undoStack;

    /**
     * The various models
     */
    PayeesModel         payeesModel;
    PayeesModel         userModel;
    CostCenterModel     costCenterModel;
    SchedulesModel      schedulesModel;
    TagsModel           tagsModel;
    SecuritiesModel     securitiesModel;
    SecuritiesModel     currenciesModel;
    BudgetsModel        budgetsModel;
    AccountsModel       accountsModel;
    InstitutionsModel   institutionsModel;
    JournalModel        journalModel;
    PriceModel          priceModel;
    ParametersModel     parametersModel;
    ParametersModel     fileInfoModel;
    OnlineJobsModel     onlineJobsModel;
    ReportsModel        reportsModel;
    SpecialDatesModel   specialDatesModel;
    SchedulesJournalModel schedulesJournalModel;
    StatusModel         statusModel;
    ReconciliationModel reconciliationModel;
    /// @note add new models here

    /**
     * Special proxy models
     */
    KDescendantsProxyModel flatAccountsModel;
};


class MyMoneyNotifier
{
public:
    MyMoneyNotifier(MyMoneyFile::Private* file) {
        m_file = file;
        m_file->clearCacheNotification();
    }
    ~MyMoneyNotifier() {
        m_file->notify();
    }
private:
    MyMoneyFile::Private* m_file;
};



MyMoneyFile::MyMoneyFile() :
    d(new Private(this))
{
    reloadSpecialDates();
    connect(&d->journalModel, &JournalModel::balanceChanged, &d->m_balanceCache, QOverload<const QString&>::of(&MyMoneyBalanceCache::clear));
}

MyMoneyFile::~MyMoneyFile()
{
    delete d;
}

const QString& MyMoneyFile::fixedKey(FixedKey key) const
{
    static QVector<QString> fixedKeys = {
        QStringLiteral("CreationDate"),
        QStringLiteral("LastModificationDate"),
        QStringLiteral("FixVersion"),
        QStringLiteral("P000001"),
    };
    static QString null;

    if ((key < 0) || (key >= fixedKeys.count())) {
        qDebug() << "Invalid key" << key << "for MyMoneyFile::fixedKey";
        return null;
    }
    return fixedKeys[key];
}

MyMoneyFile* MyMoneyFile::instance()
{
    static MyMoneyFile file;
    return &file;
}

MyMoneyModelBase* MyMoneyFile::baseModel()
{
    return instance()->accountsModel();
}

void MyMoneyFile::finalizeFileOpen()
{
    d->institutionsModel.slotLoadAccountsWithoutInstitutions(d->accountsModel.accountsWithoutInstitutions());
    d->reconciliationModel.updateData();

    // remove any undo activities generated during loading
    d->undoStack.clear();
}

void MyMoneyFile::unload()
{
    d->schedulesModel.unload();
    d->payeesModel.unload();
    d->userModel.unload();
    d->costCenterModel.unload();
    d->tagsModel.unload();
    d->securitiesModel.unload();
    d->currenciesModel.unload();
    d->budgetsModel.unload();
    d->accountsModel.unload();
    d->institutionsModel.unload();
    d->journalModel.unload();
    d->priceModel.unload();
    d->parametersModel.unload();
    d->onlineJobsModel.unload();
    d->reportsModel.unload();
    // specialdatesmodel not unloaded here on purpose
    d->schedulesJournalModel.unload();
    d->reconciliationModel.unload();
    /// @note add new models here
    d->m_baseCurrency = MyMoneySecurity();
    d->m_balanceCache.clear();
    d->m_priceCache.clear();
    d->undoStack.clear();
    d->m_dirty = false;
}

int MyMoneyFile::fileFixVersion() const
{
    QString version = d->fileInfoModel.itemById(fixedKey(FileFixVersion)).value();
    if (version.isEmpty()) {
        return availableFixVersion();
    }
    return version.toInt();
}

void MyMoneyFile::setFileFixVersion(int version)
{
    if (version > availableFixVersion())
        version = availableFixVersion();
    d->fileInfoModel.addItem(fixedKey(FileFixVersion), QString("%1").arg(version));
}

#if 0
void MyMoneyFile::attachStorage(MyMoneyStorageMgr* const storage)
{
    if (d->m_storage != 0)
        throw MYMONEYEXCEPTION_CSTRING("Storage already attached");

    if (storage == 0)
        throw MYMONEYEXCEPTION_CSTRING("Storage must not be 0");

    d->m_storage = storage;

    // force reload of base currency
    d->m_baseCurrency = MyMoneySecurity();

    // and the whole cache
    d->m_balanceCache.clear();
    d->m_priceCache.clear();

    // notify application about new data availability
    Q_EMIT beginChangeNotification();
    Q_EMIT dataChanged();
    Q_EMIT endChangeNotification();
}

void MyMoneyFile::detachStorage(MyMoneyStorageMgr* const /* storage */)
{
    d->m_balanceCache.clear();
    d->m_priceCache.clear();
    d->m_storage = nullptr;
}

MyMoneyStorageMgr* MyMoneyFile::storage() const
{
    return d->m_storage;
}

bool MyMoneyFile::storageAttached() const
{
    return d->m_storage != 0;
}
#endif

void MyMoneyFile::startTransaction(const QString& undoActionText, bool journalBlocking)
{
    if (d->m_inTransaction) {
        throw MYMONEYEXCEPTION_CSTRING("Already started a transaction!");
    }

    d->undoStack.beginMacro(undoActionText);
    d->m_inTransaction = true;
    d->m_journalBlocking = journalBlocking;
    d->m_changeSet.clear();
    Q_EMIT storageTransactionStarted(journalBlocking);
}

bool MyMoneyFile::hasTransaction() const
{
    return d->m_inTransaction;
}

void MyMoneyFile::commitTransaction()
{
    d->checkTransaction(Q_FUNC_INFO);

    // commit the transaction in the storage
    d->undoStack.endMacro();
    auto changed = false;
    d->m_inTransaction = false;

    // collect notifications about removed objects
    QStringList removedObjects;
    const auto& set = d->m_changeSet;
    for (const auto& change : set) {
        switch (change.notificationMode()) {
        case File::Mode::Remove:
            removedObjects += change.id();
            break;
        default:
            break;
        }
    }

    // inform the outside world about the beginning of notifications
    Q_EMIT beginChangeNotification();

    // Now it's time to send out some signals to the outside world
    // First we go through the d->m_changeSet and emit respective
    // signals about addition, modification and removal of engine objects
    const auto& changes = d->m_changeSet;
    for (const auto& change : changes) {
        // turn on the global changed flag for model based objects
        switch(change.objectType()) {
        /// @note add new models here
        case eMyMoney::File::Object::Payee:
        case eMyMoney::File::Object::CostCenter:
        case eMyMoney::File::Object::Schedule:
        case eMyMoney::File::Object::Tag:
        case eMyMoney::File::Object::Security:
        case eMyMoney::File::Object::Currency:
        case eMyMoney::File::Object::Budget:
        case eMyMoney::File::Object::Account:
        case eMyMoney::File::Object::Institution:
        case eMyMoney::File::Object::Transaction:
        case eMyMoney::File::Object::Price:
        case eMyMoney::File::Object::Parameter:
        case eMyMoney::File::Object::OnlineJob:
        case eMyMoney::File::Object::Report:
        case eMyMoney::File::Object::BaseCurrency:
            changed = true;
            break;
        default:
            break;
        }

        switch (change.notificationMode()) {
        case File::Mode::Remove:
            Q_EMIT objectRemoved(change.objectType(), change.id());
            // if there is a balance change recorded for this account remove it since the account itself will be removed
            // this can happen when deleting categories that have transactions and the reassign category feature was used
            d->m_balanceChangedSet.erase(change.id());
            break;
        case File::Mode::Add:
            if (!removedObjects.contains(change.id())) {
                Q_EMIT objectAdded(change.objectType(), change.id());
            }
            break;
        case File::Mode::Modify:
            if (!removedObjects.contains(change.id())) {
                Q_EMIT objectModified(change.objectType(), change.id());
            }
            break;
        }
    }

    // we're done with the change set, so we clear it
    d->m_changeSet.clear();

    // now send out the balanceChanged signal for all those
    // accounts for which we have an indication about a possible
    // change.
    const auto& balanceChanges = d->m_balanceChangedSet;
    for (const auto& id : balanceChanges) {
        if (!removedObjects.contains(id)) {
            // if we notify about balance change we don't need to notify about value change
            // for the same account since a balance change implies a value change
            d->m_valueChangedSet.erase(id);
            Q_EMIT balanceChanged(account(id));
        }
    }
    d->m_balanceChangedSet.clear();

    // now notify about the remaining value changes
    const auto& m_valueChanges = d->m_valueChangedSet;
    for (const auto& id : m_valueChanges) {
        if (!removedObjects.contains(id)) {
            changed = true;
            Q_EMIT valueChanged(account(id));
        }
    }

    if (!d->m_valueChangedSet.isEmpty()) {
        d->journalModel.updateBalances();
    }

    d->m_valueChangedSet.clear();

    // as a last action, update the last modification date and send out the global dataChanged signal
    if (changed) {
        d->fileInfoModel.addItem(fixedKey(MyMoneyFile::LastModificationDate), MyMoneyUtils::dateTimeToIsoString(QDateTime::currentDateTime()));

        Q_EMIT dataChanged();
    }

    // inform the outside world about the end of notifications
    Q_EMIT endChangeNotification();

    Q_EMIT storageTransactionEnded(d->m_journalBlocking);
}

void MyMoneyFile::rollbackTransaction()
{
    d->checkTransaction(Q_FUNC_INFO);

    /// @todo add rollback of undo stack here
    // finish the sequence
    d->undoStack.endMacro();
    qDebug() << "Rollback transaction with now" << d->undoStack.count() << "commands on stack at index" << d->undoStack.index();
    // and undo it immediately
    d->undoStack.undo();
    qDebug() << "Rolled back transaction with now" << d->undoStack.count() << "commands on stack at index" << d->undoStack.index();

    d->m_inTransaction = false;
    d->m_balanceChangedSet.clear();
    d->m_valueChangedSet.clear();
    d->m_changeSet.clear();

    Q_EMIT storageTransactionEnded(d->m_journalBlocking);
}

void MyMoneyFile::addInstitution(MyMoneyInstitution& institution)
{
    // perform some checks to see that the institution stuff is OK. For
    // now we assume that the institution must have a name, the ID is not set
    // and it does not have a parent (MyMoneyFile).

    if (institution.name().isEmpty()
            || !institution.id().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("Not a new institution");

    d->checkTransaction(Q_FUNC_INFO);
    d->institutionsModel.addItem(institution);

    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Institution, institution.id());
}

void MyMoneyFile::modifyInstitution(const MyMoneyInstitution& institution)
{
    d->checkTransaction(Q_FUNC_INFO);

    const auto idx = d->institutionsModel.indexById(institution.id());
    if (!idx.isValid()) {
        throw MYMONEYEXCEPTION_CSTRING("Unknown institution");
    }

    d->institutionsModel.modifyItem(institution);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Institution, institution.id());
}

void MyMoneyFile::removeInstitution(const MyMoneyInstitution& institution)
{
    d->checkTransaction(Q_FUNC_INFO);

    MyMoneyInstitution inst = d->institutionsModel.itemById(institution.id());

    if (inst.id().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("Unknown institution");

    QSignalBlocker blocker(this);
    const auto accounts = inst.accountList();
    for (const auto& accountId : accounts) {
        auto a = account(accountId);
        a.setInstitutionId(QString());
        modifyAccount(a);
        d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, a.id());
    }

    d->institutionsModel.removeItem(institution);

    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Institution, institution.id());
}

QList<MyMoneyInstitution> MyMoneyFile::institutionList() const
{
    return d->institutionsModel.itemList();
}




void MyMoneyFile::modifyTransaction(const MyMoneyTransaction& transaction)
{
    d->checkTransaction(Q_FUNC_INFO);

    MyMoneyTransaction tCopy(transaction);

    // first perform all the checks
    if (transaction.id().isEmpty()
            || !transaction.postDate().isValid())
        throw MYMONEYEXCEPTION_CSTRING("invalid transaction to be modified");

    // now check the splits
    bool loanAccountAffected = false;
    const auto splits1 = transaction.splits();
    for (const auto& split : splits1) {
        // the following line will throw an exception if the
        // account does not exist
        auto acc = MyMoneyFile::account(split.accountId());
        if (acc.id().isEmpty())
            throw MYMONEYEXCEPTION_CSTRING("Cannot store split with no account assigned");
        if (isStandardAccount(split.accountId()))
            throw MYMONEYEXCEPTION_CSTRING("Cannot store split referencing standard account");
        if (acc.isLoan() && (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer)))
            loanAccountAffected = true;
        if (!split.payeeId().isEmpty()) {
            if (payee(split.payeeId()).id().isEmpty()) {
                throw MYMONEYEXCEPTION_CSTRING("Cannot add split referencing unknown payee");
            }
        }
        const auto tagIdList = split.tagIdList();
        for (const auto& tagId : tagIdList) {
            if (!tagId.isEmpty())
                tag(tagId);
        }
    }

    // change transfer splits between asset/liability and loan accounts
    // into amortization splits
    if (loanAccountAffected) {
        const auto splits = transaction.splits();
        for (const auto& split : splits) {
            if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer)) {
                auto acc = MyMoneyFile::account(split.accountId());

                if (acc.isAssetLiability()) {
                    MyMoneySplit s = split;
                    s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
                    tCopy.modifySplit(s);
                }
            }
        }
    }

    // clear all changed objects from cache
    MyMoneyNotifier notifier(d);

    // get the current setting of this transaction
    MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());

    // scan the splits again to update notification list
    // and mark all accounts that are referenced
    const auto splits2 = tr.splits();
    for (const auto& split : splits2)
        d->addCacheNotification(split.accountId(), tr.postDate());

    // make sure the value is rounded to the accounts precision
    fixSplitPrecision(tCopy);

    d->adjustSplitSign(tCopy);

    d->journalModel.modifyTransaction(tCopy);

    // and mark all accounts that are referenced
    const auto splits3 = tCopy.splits();
    for (const auto& split : splits3)
        d->addCacheNotification(split.accountId(), tCopy.postDate());

    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Transaction, transaction.id());
}

void MyMoneyFile::modifyAccount(const MyMoneyAccount& _account)
{
    d->checkTransaction(Q_FUNC_INFO);

    MyMoneyAccount account(_account);

    QModelIndex idx = d->accountsModel.indexById(account.id());
    if (!idx.isValid())
        throw MYMONEYEXCEPTION_CSTRING("Unknown account");

    auto acc = d->accountsModel.itemByIndex(idx);

    // check that for standard accounts only specific parameters are changed
    if (isStandardAccount(account.id())) {
        // make sure to use the stuff we found on file
        account = acc;

        // and only use the changes that are allowed
        account.setName(_account.name());
        account.setCurrencyId(_account.currencyId());

        // now check that it is the same
        if (!(account == _account))
            throw MYMONEYEXCEPTION_CSTRING("Unable to modify the standard account groups");
    }

    if (account.accountType() != acc.accountType() //
        && (!account.isLiquidAsset() || !acc.isLiquidAsset()))
        throw MYMONEYEXCEPTION_CSTRING("Unable to change account type");

    // make sure that all the referenced objects exist
    if (!account.institutionId().isEmpty())
        institution(account.institutionId());

    const auto subAccountList = account.accountList();
    for (const auto& sAccount : qAsConst(subAccountList))
        this->account(sAccount);

    // if the account was moved to another institution, we notify
    // the old one as well as the new one and the structure change
    if (acc.institutionId() != account.institutionId()) {
        MyMoneyInstitution inst;
        d->institutionsModel.removeAccount(acc.institutionId(), acc.id());
        if (!acc.institutionId().isEmpty()) {
            inst = institution(acc.institutionId());
            inst.removeAccountId(acc.id());
            modifyInstitution(inst);
        }
        if (!account.institutionId().isEmpty()) {
            inst = institution(account.institutionId());
            inst.addAccountId(acc.id());
            modifyInstitution(inst);

            // don't forget the entry in the institution model for asset, liability and equity
            if (!account.isIncomeExpense()) {
                d->institutionsModel.addAccount(account.institutionId(), account.id());
            }
        }
    }

    // check if account can be closed
    if (account.isClosed() && !acc.isClosed()) {
        // balance must be zero
        if (!account.balance().isZero())
            throw MYMONEYEXCEPTION_CSTRING("Cannot close account with balance unequal to zero");
        if (account.hasOnlineMapping())
            throw MYMONEYEXCEPTION_CSTRING("Cannot close account with active online mapping");

        // all children must be closed already
        const auto accList = account.accountList();
        for (const auto& sAccount : accList) {
            const auto subAccount = MyMoneyFile::instance()->account(sAccount);
            if (!subAccount.isClosed()) {
                throw MYMONEYEXCEPTION_CSTRING("Cannot close account with open sub-account");
            }
        }

        // there must be no unfinished schedule referencing the account
        QList<MyMoneySchedule> list = scheduleList();
        QList<MyMoneySchedule>::const_iterator it_l;
        for (it_l = list.cbegin(); it_l != list.cend(); ++it_l) {
            if ((*it_l).isFinished())
                continue;
            if ((*it_l).hasReferenceTo(acc.id())) {
                throw MYMONEYEXCEPTION_CSTRING("Cannot close account referenced in schedule");
            }
        }
    }

    d->accountsModel.modifyItem(account);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, account.id());
}

void MyMoneyFile::reparentAccountByIds(const QString& accountId, const QString& newParentId)
{
    MyMoneyFileTransaction ft;
    try {
        auto acc = account(accountId);
        auto newParent = account(newParentId);
        reparentAccount(acc, newParent);
        ft.commit();
    } catch (MyMoneyException& e) {
        qDebug() << e.what();
    }
}

void MyMoneyFile::reparentAccount(MyMoneyAccount &acc, MyMoneyAccount& parent)
{
    d->checkTransaction(Q_FUNC_INFO);

    // check that it's not one of the standard account groups
    if (isStandardAccount(acc.id()))
        throw MYMONEYEXCEPTION_CSTRING("Unable to reparent the standard account groups");

    if (acc.accountGroup() == parent.accountGroup()
            || (acc.accountType() == Account::Type::Income && parent.accountType() == Account::Type::Expense)
            || (acc.accountType() == Account::Type::Expense && parent.accountType() == Account::Type::Income)) {

        if (acc.isInvest() && parent.accountType() != Account::Type::Investment)
            throw MYMONEYEXCEPTION_CSTRING("Unable to reparent Stock to non-investment account");

        if (parent.accountType() == Account::Type::Investment && !acc.isInvest())
            throw MYMONEYEXCEPTION_CSTRING("Unable to reparent non-stock to investment account");

        // keep a notification of the current parent
        MyMoneyAccount curParent = account(acc.parentAccountId());

        if (!d->accountsModel.indexById(acc.id()).isValid())
            throw MYMONEYEXCEPTION_CSTRING("Unable to reparent non existent account");

        // reparent in model
        d->accountsModel.reparentAccount(acc.id(), parent.id());

        // update data in references
        acc = d->accountsModel.itemById(acc.id());
        parent = d->accountsModel.itemById(parent.id());

        d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, curParent.id());
        d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, parent.id());
        d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, acc.id());

    } else
        throw MYMONEYEXCEPTION_CSTRING("Unable to reparent to different account type");
}

MyMoneyInstitution MyMoneyFile::institution(const QString& id) const
{
    if (Q_UNLIKELY(id.isEmpty())) // FIXME: Stop requesting accounts with empty id
        return MyMoneyInstitution();

    const auto idx = d->institutionsModel.indexById(id);
    if (idx.isValid())
        return d->institutionsModel.itemByIndex(idx);

    throw MYMONEYEXCEPTION_CSTRING("Unknown institution");
}

MyMoneyAccount MyMoneyFile::account(const QString& id) const
{
    if (Q_UNLIKELY(id.isEmpty()) || Q_UNLIKELY(journalModel()->fakeId().compare(id) == 0)) // FIXME: Stop requesting accounts with empty id
        return MyMoneyAccount();

    const auto idx = d->accountsModel.indexById(id);
    if (idx.isValid())
        return d->accountsModel.itemByIndex(idx);

    throw MYMONEYEXCEPTION_CSTRING("Unknown account");
}

MyMoneyAccount MyMoneyFile::subAccountByName(const MyMoneyAccount& account, const QString& name) const
{
    const auto accounts = account.accountList();
    for (const auto& acc : accounts) {
        const auto sacc = MyMoneyFile::account(acc);
        if (sacc.name().compare(name) == 0)
            return sacc;
    }
    return {};
}

MyMoneyAccount MyMoneyFile::accountByName(const QString& name) const
{
    try {
        auto indexList = d->accountsModel.indexListByName(name);
        if (indexList.isEmpty()) {
            return {};
        }
        return d->accountsModel.itemByIndex(indexList.first());
    } catch (const MyMoneyException &) {
    }
    return {};
}

void MyMoneyFile::removeTransaction(const MyMoneyTransaction& transaction)
{
    d->checkTransaction(Q_FUNC_INFO);

    // clear all changed objects from cache
    MyMoneyNotifier notifier(d);

    // get the engine's idea about this transaction
    MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());

    // scan the splits again to update notification list
    const auto splits = tr.splits();
    for (const auto& split : splits) {
        auto acc = account(split.accountId());
        if (acc.isClosed())
            throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot remove transaction that references a closed account."));
        d->addCacheNotification(split.accountId(), tr.postDate());
        //FIXME-ALEX Do I need to add d->addCacheNotification(split.tagList()); ??
    }

    d->journalModel.removeTransaction(transaction);

    // remove a possible notification of that same object from the changeSet
    QList<MyMoneyNotification>::iterator it;
    for(it = d->m_changeSet.begin(); it != d->m_changeSet.end();) {
        if((*it).id() == transaction.id()) {
            it = d->m_changeSet.erase(it);
        } else {
            ++it;
        }
    }

    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Transaction, transaction.id());
}


bool MyMoneyFile::hasActiveSplits(const QString& id) const
{
    return d->journalModel.hasReferenceTo(id);
}

bool MyMoneyFile::isStandardAccount(const QString& id) const
{
    return id == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Liability) //
           || id == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset) //
           || id == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense) //
           || id == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Income) //
           || id == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Equity);
}

bool MyMoneyFile::isInvestmentTransaction(const MyMoneyTransaction& t) const
{
    const auto splits = t.splits();
    for (const auto& split : qAsConst(splits)) {
        auto acc = d->accountsModel.itemById(split.accountId());
        if (!acc.id().isEmpty()) {
            if (acc.isInvest() && (split.investmentTransactionType() != eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType)) {
                return true;
            }
        }
    }
    return false;
}

void MyMoneyFile::removeAccount(const MyMoneyAccount& account)
{
    d->checkTransaction(Q_FUNC_INFO);

    MyMoneyAccount parent;
    MyMoneyAccount acc;

    // check that the account and its parent exist
    // this will throw an exception if the id is unknown
    auto idx = d->accountsModel.indexById(account.id());
    if (!idx.isValid())
        throw MYMONEYEXCEPTION_CSTRING("Unable to remove not existing account");

    acc = d->accountsModel.itemByIndex(idx);

    parent = d->accountsModel.itemById(account.parentAccountId());

    // check that it's not one of the standard account groups
    if (isStandardAccount(account.id()))
        throw MYMONEYEXCEPTION_CSTRING("Unable to remove the standard account groups");

    if (hasActiveSplits(account.id())) {
        throw MYMONEYEXCEPTION_CSTRING("Unable to remove account with active splits");
    }

    // re-parent all sub-ordinate accounts to the parent of the account
    // to be deleted. First round check that all accounts exist, second
    // round do the re-parenting.
    auto subAccountList = account.accountList();
    for (const auto& accountId : qAsConst(subAccountList)) {
        this->account(accountId);
    }

    // if one of the accounts did not exist, an exception had been
    // thrown and we would not make it until here.
    auto newParent = d->accountsModel.itemById(acc.parentAccountId());
    subAccountList = acc.accountList();
    for (const auto& accountId : qAsConst(subAccountList)) {
        auto accountToMove = d->accountsModel.itemById(accountId);
        reparentAccount(accountToMove, newParent);
        d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, accountToMove.id());
    }

    // don't forget the a possible institution
    if (!acc.institutionId().isEmpty()) {
        MyMoneyInstitution institution = d->institutionsModel.itemById(acc.institutionId());
        institution.removeAccountId(acc.id());
        modifyInstitution(institution);
    }
    // don't forget the entry in the institution model for asset, liability and equity
    if (!acc.isIncomeExpense()) {
        d->institutionsModel.removeAccount(acc.institutionId(), acc.id());
    }

    acc.setInstitutionId(QString());

    // get the index again as it might have changed
    idx = d->accountsModel.indexById(acc.id());
    d->accountsModel.removeItem(idx);

    d->m_balanceCache.clear(acc.id());

    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, parent.id());
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Account, acc.id());
}

void MyMoneyFile::removeAccountList(const QStringList& account_list, unsigned int level)
{
    if (level > 100)
        throw MYMONEYEXCEPTION_CSTRING("Too deep recursion in [MyMoneyFile::removeAccountList]!");

    d->checkTransaction(Q_FUNC_INFO);

    // upon entry, we check that we could proceed with the operation
    if (!level) {
        if (!hasOnlyUnusedAccounts(account_list, 0)) {
            throw MYMONEYEXCEPTION_CSTRING("One or more accounts cannot be removed");
        }
    }

    // process all accounts in the list and test if they have transactions assigned
    for (const auto& sAccount : account_list) {
        auto a = d->accountsModel.itemById(sAccount);
        //qDebug() << "Deleting account '"<< a.name() << "'";

        // first remove all sub-accounts
        if (!a.accountList().isEmpty()) {
            removeAccountList(a.accountList(), level + 1);

            // then remove account itself, but we first have to get
            // rid of the account list that is still stored in
            // the MyMoneyAccount object. Easiest way is to get a fresh copy.
            a = d->accountsModel.itemById(sAccount);
        }

        // make sure to remove the item from the cache
        removeAccount(a);
    }
}

bool MyMoneyFile::hasOnlyUnusedAccounts(const QStringList& account_list, unsigned int level)
{
    if (level > 100)
        throw MYMONEYEXCEPTION_CSTRING("Too deep recursion in [MyMoneyFile::hasOnlyUnusedAccounts]!");
    // process all accounts in the list and test if they have transactions assigned
    for (const auto& sAccount : account_list) {
        if (transactionCount(sAccount) != 0)
            return false; // the current account has a transaction assigned
        if (!hasOnlyUnusedAccounts(account(sAccount).accountList(), level + 1))
            return false; // some sub-account has a transaction assigned
    }
    return true; // all subaccounts unused
}


void MyMoneyFile::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
{
    // make sure we have a currency. If none is assigned, we assume base currency
    if (newAccount.currencyId().isEmpty())
        newAccount.setCurrencyId(baseCurrency().id());

    MyMoneyFileTransaction ft;
    try {
        int pos;
        // check for ':' in the name and use it as separator for a hierarchy
        while ((pos = newAccount.name().indexOf(MyMoneyFile::AccountSeparator)) != -1) {
            QString part = newAccount.name().left(pos);
            QString remainder = newAccount.name().mid(pos + 1);
            const MyMoneyAccount& existingAccount = subAccountByName(parentAccount, part);
            if (existingAccount.id().isEmpty()) {
                newAccount.setName(part);

                addAccount(newAccount, parentAccount);
                parentAccount = newAccount;
            } else {
                parentAccount = existingAccount;
            }
            newAccount.setParentAccountId(QString());  // make sure, there's no parent
            newAccount.clearId();                       // and no id set for adding
            newAccount.removeAccountIds();              // and no sub-account ids
            newAccount.setName(remainder);
        }

        addAccount(newAccount, parentAccount);

        // in case of a loan account, we add the initial payment
        if ((newAccount.accountType() == Account::Type::Loan
                || newAccount.accountType() == Account::Type::AssetLoan)
                && !newAccount.value("kmm-loan-payment-acc").isEmpty()
                && !newAccount.value("kmm-loan-payment-date").isEmpty()) {
            MyMoneyAccountLoan acc(newAccount);
            MyMoneyTransaction t;
            MyMoneySplit a, b;
            a.setAccountId(acc.id());
            b.setAccountId(acc.value("kmm-loan-payment-acc"));
            a.setValue(acc.loanAmount());
            if (acc.accountType() == Account::Type::Loan)
                a.setValue(-a.value());

            a.setShares(a.value());
            b.setValue(-a.value());
            b.setShares(b.value());
            a.setMemo(i18n("Loan payout"));
            b.setMemo(i18n("Loan payout"));
            t.setPostDate(QDate::fromString(acc.value("kmm-loan-payment-date"), Qt::ISODate));
            newAccount.deletePair("kmm-loan-payment-acc");
            newAccount.deletePair("kmm-loan-payment-date");
            MyMoneyFile::instance()->modifyAccount(newAccount);

            t.addSplit(a);
            t.addSplit(b);
            addTransaction(t);
            createOpeningBalanceTransaction(newAccount, openingBal);

            // in case of an investment account we check if we should create
            // a brokerage account
        } else if (newAccount.accountType() == Account::Type::Investment
                   && !brokerageAccount.name().isEmpty()) {
            addAccount(brokerageAccount, parentAccount);

            // set a link from the investment account to the brokerage account
            modifyAccount(newAccount);
            createOpeningBalanceTransaction(brokerageAccount, openingBal);

        } else
            createOpeningBalanceTransaction(newAccount, openingBal);

        ft.commit();
    } catch (const MyMoneyException &e) {
        qWarning("Unable to create account: %s", e.what());
        throw;
    }
}

void MyMoneyFile::addAccount(MyMoneyAccount& account, MyMoneyAccount& parent)
{
    d->checkTransaction(Q_FUNC_INFO);

    // perform some checks to see that the account stuff is OK. For
    // now we assume that the account must have a name, has no
    // transaction and sub-accounts and parent account
    // it's own ID is not set and it does not have a pointer to (MyMoneyFile)

    if (account.name().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("Account has no name");

    if (!account.id().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("New account must have no id");

    if (!account.accountList().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("New account must have no sub-accounts");

    if (!account.parentAccountId().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("New account must have no parent-id");

    if (account.accountType() == Account::Type::Unknown)
        throw MYMONEYEXCEPTION_CSTRING("Account has invalid type");

    // make sure, that the parent account exists
    // if not, an exception is thrown. If it exists,
    // get a copy of the current data
    auto acc = d->accountsModel.itemById(parent.id());

    if (acc.id().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("Parent account does not exist");

    // FIXME: make sure, that the parent has the same type
    // I left it out here because I don't know, if there is
    // a tight coupling between e.g. checking accounts and the
    // class asset. It certainly does not make sense to create an
    // expense account under an income account. Maybe it does, I don't know.

    // We enforce, that a stock account can never be a parent and
    // that the parent for a stock account must be an investment. Also,
    // an investment cannot have another investment account as it's parent
    if (parent.isInvest())
        throw MYMONEYEXCEPTION_CSTRING("Stock account cannot be parent account");

    if (account.isInvest() && parent.accountType() != Account::Type::Investment)
        throw MYMONEYEXCEPTION_CSTRING("Stock account must have investment account as parent ");

    if (!account.isInvest() && parent.accountType() == Account::Type::Investment)
        throw MYMONEYEXCEPTION_CSTRING("Investment account can only have stock accounts as children");

    // if an institution is set, verify that it exists
    MyMoneyInstitution institution;
    if (!account.institutionId().isEmpty()) {
        // check the presence of the institution. if it
        // does not exist, an exception is thrown
        institution = MyMoneyFile::institution(account.institutionId());
        if (institution.id().isEmpty())
            throw MYMONEYEXCEPTION_CSTRING("Institution not found");
    }

    // if we don't have a valid opening date use today
    if (!account.openingDate().isValid()) {
        account.setOpeningDate(QDate::currentDate());
    }

    // make sure to set the opening date for categories to a
    // fixed date (1900-1-1). See #313793 on b.k.o for details
    if (account.isIncomeExpense()) {
        account.setOpeningDate(QDate(1900, 1, 1));
    }

    // if we don't have a currency assigned use the base currency
    if (account.currencyId().isEmpty()) {
        account.setCurrencyId(baseCurrency().id());
    }

    // make sure the currency exists
    auto currency = security(account.currencyId());
    if (currency.id().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("Currency not found");

    // setup fraction
    account.fraction(currency);

    // make sure the parent id is setup
    account.setParentAccountId(parent.id());

    d->accountsModel.addItem(account);

    // d->m_storage->addAccount(account);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Account, account.id());

    parent.addAccountId(account.id());
    d->accountsModel.modifyItem(parent);

    // d->m_storage->addAccount(parent, account);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Account, parent.id());

    if (!account.institutionId().isEmpty()) {
        institution.addAccountId(account.id());
        d->institutionsModel.modifyItem(institution);
        // d->m_storage->modifyInstitution(institution);
        d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Institution, institution.id());
    }

    // don't forget the entry in the institution model for asset, liability and equity
    if (!account.isIncomeExpense()) {
        d->institutionsModel.addAccount(account.institutionId(), account.id());
    }
}

MyMoneyTransaction MyMoneyFile::createOpeningBalanceTransaction(const MyMoneyAccount& acc, const MyMoneyMoney& balance)
{
    MyMoneyTransaction t;
    // if the opening balance is not zero, we need
    // to create the respective transaction
    if (!balance.isZero()) {
        d->checkTransaction(Q_FUNC_INFO);

        MyMoneySecurity currency = security(acc.currencyId());
        MyMoneyAccount openAcc = openingBalanceAccount(currency);

        if (openAcc.openingDate() > acc.openingDate()) {
            openAcc.setOpeningDate(acc.openingDate());
            modifyAccount(openAcc);
        }

        MyMoneySplit s;

        t.setPostDate(acc.openingDate());
        t.setCommodity(acc.currencyId());

        s.setAccountId(acc.id());
        s.setShares(balance);
        s.setValue(balance);
        t.addSplit(s);

        s.clearId();
        s.setAccountId(openAcc.id());
        s.setShares(-balance);
        s.setValue(-balance);
        t.addSplit(s);

        addTransaction(t);
    }
    return t;
}

QString MyMoneyFile::openingBalanceTransaction(const MyMoneyAccount& acc) const
{
    MyMoneyAccount openAcc;

    try {
        openAcc = openingBalanceAccount(security(acc.currencyId()));
    } catch (const MyMoneyException &) {
        return QString();
    }

    // Iterate over all transactions starting at the opening date
    const auto start = d->journalModel.MyMoneyModelBase::lowerBound(d->journalModel.keyForDate(acc.openingDate())).row();
    const auto end = d->journalModel.rowCount();

    // look for a transaction with two splits, one referencing
    // acc.id(), the other openAcc.id()
    int matchCount = 0;
    QString lastTxId;
    QString txId;
    QString splitAccoountId;
    QModelIndex idx;
    for (int row = start; row < end; ++row) {
        idx = d->journalModel.index(row, 0);
        txId = idx.data(eMyMoney::Model::JournalTransactionIdRole).toString();
        if (lastTxId != txId) {
            matchCount = 0;
            lastTxId = txId;
        }
        splitAccoountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        if (splitAccoountId == acc.id())
            ++matchCount;
        else if(splitAccoountId == openAcc.id())
            ++matchCount;

        // if we found both accounts in a transaction we have a match
        if (matchCount == 2) {
            return txId;
        }
    }
    // no opening balance transaction found
    return QString();
}

MyMoneyAccount MyMoneyFile::openingBalanceAccount(const MyMoneySecurity& security)
{
    if (!security.isCurrency())
        throw MYMONEYEXCEPTION_CSTRING("Opening balance for non currencies not supported");

    try {
        return openingBalanceAccount_internal(security);
    } catch (const MyMoneyException &) {
        MyMoneyFileTransaction ft;
        MyMoneyAccount acc;

        try {
            acc = createOpeningBalanceAccount(security);
            ft.commit();

        } catch (const MyMoneyException &) {
            qDebug("Unable to create opening balance account for security %s", qPrintable(security.id()));
        }
        return acc;
    }
}

MyMoneyAccount MyMoneyFile::openingBalanceAccount(const MyMoneySecurity& security) const
{
    return openingBalanceAccount_internal(security);
}

MyMoneyAccount MyMoneyFile::openingBalanceAccount_internal(const MyMoneySecurity& security) const
{
    if (!security.isCurrency())
        throw MYMONEYEXCEPTION_CSTRING("Opening balance for non currencies not supported");

    MyMoneyAccount acc;
    QList<MyMoneyAccount> accounts;
    QList<MyMoneyAccount>::const_iterator it;

    accountList(accounts, equity().accountList(), true);

    // If an account is clearly marked as an opening balance account
    // for a specific currency we use it
    for (it = accounts.cbegin(); it != accounts.cend(); ++it) {
        if ((it->value("OpeningBalanceAccount") == QLatin1String("Yes")) && (it->currencyId() == security.id())) {
            acc = *it;
            break;
        }
    }

    // If we did not find one, then we look for one with a
    // name starting with the opening balances prefix.
    if (acc.id().isEmpty()) {
        for (it = accounts.cbegin(); it != accounts.cend(); ++it) {
            if (it->name().startsWith(MyMoneyFile::openingBalancesPrefix()) && (it->currencyId() == security.id())) {
                acc = *it;
                break;
            }
        }
    }

    // If we still don't have an opening balances account, we
    // see if we can find one of type equity with the currency
    // in question. This is needed, in case we have an old file
    // which does not have the marker of step 1 above because
    // is not (yet) present, the name did not match because it
    // has been replaced (e.g. anonymization or the language
    // of the application has been changed)
    if (acc.id().isEmpty()) {
        for (it = accounts.cbegin(); it != accounts.cend(); ++it) {
            if ((it->accountType() == eMyMoney::Account::Type::Equity) && (it->currencyId() == security.id())) {
                acc = *it;
                break;
            }
        }
    }

    if (acc.id().isEmpty())
        throw MYMONEYEXCEPTION(QString::fromLatin1("No opening balance account for %1").arg(security.tradingSymbol()));

    return acc;
}

MyMoneyAccount MyMoneyFile::createOpeningBalanceAccount(const MyMoneySecurity& security)
{
    d->checkTransaction(Q_FUNC_INFO);

    MyMoneyAccount acc;
    QList<MyMoneyAccount> accounts;
    QList<MyMoneyAccount>::const_iterator it;

    accountList(accounts, equity().accountList(), true);

    // find present opening balance accounts without containing '('
    QString name;
    QString parentAccountId;
    static const QRegularExpression currencyExp(QLatin1String("\\([A-Z]{3}\\)"));

    for (it = accounts.cbegin(); it != accounts.cend(); ++it) {
        const auto currencyMatch(currencyExp.match(it->name()));
        if (it->value("OpeningBalanceAccount") == QLatin1String("Yes") && currencyMatch.hasMatch()) {
            name = it->name();
            parentAccountId = it->parentAccountId();
            break;
        }
    }

    if (name.isEmpty())
        name = MyMoneyFile::openingBalancesPrefix();
    if (security.id() != baseCurrency().id()) {
        name += QString(" (%1)").arg(security.id());
    }
    acc.setName(name);
    acc.setAccountType(Account::Type::Equity);
    acc.setCurrencyId(security.id());
    acc.setValue("OpeningBalanceAccount", "Yes");

    MyMoneyAccount parent = !parentAccountId.isEmpty() ? account(parentAccountId) : equity();
    this->addAccount(acc, parent);
    return acc;
}

void MyMoneyFile::addTransaction(MyMoneyTransaction& transaction)
{
    d->checkTransaction(Q_FUNC_INFO);

    // clear all changed objects from cache
    MyMoneyNotifier notifier(d);

    // perform some checks to see that the transaction stuff is OK. For
    // now we assume that
    // * no ids are assigned
    // * the date valid (must not be empty)
    // * the referenced accounts in the splits exist

    // first perform all the checks
    if (!transaction.id().isEmpty())
        throw MYMONEYEXCEPTION_CSTRING("Unable to add transaction with id set");
    if (!transaction.postDate().isValid())
        throw MYMONEYEXCEPTION_CSTRING("Unable to add transaction with invalid postdate");

    // now check the splits
    auto loanAccountAffected = false;
    const auto splits1 = transaction.splits();
    for (const auto& split : splits1) {
        // the following line will throw an exception if the
        // account does not exist or is one of the standard accounts
        auto acc = MyMoneyFile::account(split.accountId());
        if (acc.id().isEmpty())
            throw MYMONEYEXCEPTION_CSTRING("Cannot add split with no account assigned");
        if (acc.isLoan())
            loanAccountAffected = true;
        if (isStandardAccount(split.accountId()))
            throw MYMONEYEXCEPTION_CSTRING("Cannot add split referencing standard account");
        if (!split.payeeId().isEmpty()) {
            if (payee(split.payeeId()).id().isEmpty()) {
                throw MYMONEYEXCEPTION_CSTRING("Cannot add split referencing unknown payee");
            }
        }
        const auto tagIdList = split.tagIdList();
        for (const auto& tagId : tagIdList) {
            if (!tagId.isEmpty())
                tag(tagId);
        }
    }

    // change transfer splits between asset/liability and loan accounts
    // into amortization splits
    if (loanAccountAffected) {
        for (const auto& split : qAsConst(transaction.splits())) {
            if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer)) {
                auto acc = MyMoneyFile::account(split.accountId());

                if (acc.isAssetLiability()) {
                    MyMoneySplit s = split;
                    s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
                    transaction.modifySplit(s);
                }
            }
        }
    }

    // check that we have a commodity
    if (transaction.commodity().isEmpty()) {
        transaction.setCommodity(baseCurrency().id());
    }

    // make sure the value is rounded to the accounts precision
    fixSplitPrecision(transaction);

    d->adjustSplitSign(transaction);

    // then add the transaction to the file global pool
    d->journalModel.addTransaction(transaction);

    // scan the splits again to update last account access and notification list
    const auto splits2 = transaction.splits();
    for (const auto& split : splits2) {
        d->accountsModel.touchAccountById(split.accountId());
        d->addCacheNotification(split.accountId(), transaction.postDate());
    }

    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Transaction, transaction.id());
}
MyMoneyTransaction MyMoneyFile::transaction(const QString& id) const
{
    MyMoneyTransaction t(d->journalModel.transactionById(id));
    if (t.id().isEmpty()) {
        throw MYMONEYEXCEPTION_CSTRING("Selected transaction not found");
    }
    return t;
}


MyMoneyTransaction MyMoneyFile::transaction(const QString& accountId, const int idx) const
{
    auto acc = account(accountId);
    MyMoneyTransactionFilter filter;

    if (acc.accountGroup() == eMyMoney::Account::Type::Income //
            || acc.accountGroup() == eMyMoney::Account::Type::Expense)
        filter.addCategory(accountId);
    else
        filter.addAccount(accountId);

    QList<MyMoneyTransaction> list;
    transactionList(list, filter);
    if (idx < 0 || idx >= static_cast<int>(list.count()))
        throw MYMONEYEXCEPTION_CSTRING("Unknown idx for transaction");

    return transaction(list[idx].id());
}

PayeesModel * MyMoneyFile::payeesModel() const
{
    return &d->payeesModel;
}

CostCenterModel* MyMoneyFile::costCenterModel() const
{
    return &d->costCenterModel;
}

SchedulesModel * MyMoneyFile::schedulesModel() const
{
    return &d->schedulesModel;
}

TagsModel* MyMoneyFile::tagsModel() const
{
    return &d->tagsModel;
}

SecuritiesModel* MyMoneyFile::securitiesModel() const
{
    return &d->securitiesModel;
}

SecuritiesModel* MyMoneyFile::currenciesModel() const
{
    return &d->currenciesModel;
}

BudgetsModel* MyMoneyFile::budgetsModel() const
{
    return &d->budgetsModel;
}

AccountsModel* MyMoneyFile::accountsModel() const
{
    return &d->accountsModel;
}

KDescendantsProxyModel* MyMoneyFile::flatAccountsModel() const
{
    return &d->flatAccountsModel;
}

InstitutionsModel* MyMoneyFile::institutionsModel() const
{
    return &d->institutionsModel;
}

JournalModel* MyMoneyFile::journalModel() const
{
    return &d->journalModel;
}

PriceModel* MyMoneyFile::priceModel() const
{
    return &d->priceModel;
}

ParametersModel* MyMoneyFile::parametersModel() const
{
    return &d->parametersModel;
}

ParametersModel* MyMoneyFile::fileInfoModel() const
{
    return &d->fileInfoModel;
}

OnlineJobsModel* MyMoneyFile::onlineJobsModel() const
{
    return &d->onlineJobsModel;
}

ReportsModel* MyMoneyFile::reportsModel() const
{
    return &d->reportsModel;
}

PayeesModel* MyMoneyFile::userModel() const
{
    return &d->userModel;
}

SpecialDatesModel* MyMoneyFile::specialDatesModel() const
{
    return &d->specialDatesModel;
}

SchedulesJournalModel* MyMoneyFile::schedulesJournalModel() const
{
    return &d->schedulesJournalModel;
}

StatusModel* MyMoneyFile::statusModel() const
{
    return &d->statusModel;
}

ReconciliationModel* MyMoneyFile::reconciliationModel() const
{
    return &d->reconciliationModel;
}

/// @note add new models here

void MyMoneyFile::addPayee(MyMoneyPayee& payee)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->payeesModel.addItem(payee);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Payee, payee.id());
}

MyMoneyPayee MyMoneyFile::payee(const QString& id) const
{
    if (Q_UNLIKELY(id.isEmpty()))
        return MyMoneyPayee();

    const auto idx = d->payeesModel.indexById(id);
    if (idx.isValid())
        return d->payeesModel.itemByIndex(idx);

    throw MYMONEYEXCEPTION(QString::fromLatin1("Unknown payee ID: %1").arg(id));
}

MyMoneyPayee MyMoneyFile::payeeByName(const QString& name) const
{
    return d->payeesModel.itemByName(name);
}

void MyMoneyFile::modifyPayee(const MyMoneyPayee& payee)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->payeesModel.modifyItem(payee);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Payee, payee.id());
}

void MyMoneyFile::removePayee(const MyMoneyPayee& payee)
{
    d->checkTransaction(Q_FUNC_INFO);

    if (isReferenced(payee.id())) {
        throw MYMONEYEXCEPTION(QStringLiteral("Payee %1 is still referenced and cannot be deleted").arg(payee.name()));
    }
    d->payeesModel.removeItem(payee);
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Payee, payee.id());
}

void MyMoneyFile::addTag(MyMoneyTag& tag)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->tagsModel.addItem(tag);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Tag, tag.id());
}

MyMoneyTag MyMoneyFile::tag(const QString& id) const
{
    return d->tagsModel.itemById(id);
}

MyMoneyTag MyMoneyFile::tagByName(const QString& name) const
{
    return d->tagsModel.itemByName(name);
}

void MyMoneyFile::modifyTag(const MyMoneyTag& tag)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->tagsModel.modifyItem(tag);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Tag, tag.id());
}

void MyMoneyFile::removeTag(const MyMoneyTag& tag)
{
    d->checkTransaction(Q_FUNC_INFO);

    // FIXME we need to make sure, that the tag is not referenced anymore
    d->tagsModel.removeItem(tag);
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Tag, tag.id());
}

void MyMoneyFile::accountList(QList<MyMoneyAccount>& list, const QStringList& idlist, const bool recursive) const
{
    if (idlist.isEmpty()) {
        list = d->accountsModel.itemList();

        QList<MyMoneyAccount>::Iterator it;
        for (it = list.begin(); it != list.end();) {
            if (isStandardAccount((*it).id())) {
                it = list.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        QList<MyMoneyAccount>::const_iterator it;
        QList<MyMoneyAccount> list_a = d->accountsModel.itemList();

        for (it = list_a.cbegin(); it != list_a.cend(); ++it) {
            if (!isStandardAccount((*it).id())) {
                if (idlist.indexOf((*it).id()) != -1) {
                    list.append(*it);
                    if (recursive == true && !(*it).accountList().isEmpty()) {
                        accountList(list, (*it).accountList(), true);
                    }
                }
            }
        }
    }
}

// general get functions
MyMoneyPayee MyMoneyFile::user() const
{
    return d->userModel.itemById(fixedKey(MyMoneyFile::UserID));
}

// general set functions
void MyMoneyFile::setUser(const MyMoneyPayee& user)
{
    d->checkTransaction(Q_FUNC_INFO);

    auto payee = MyMoneyPayee(fixedKey(MyMoneyFile::UserID), user);
    if (d->userModel.rowCount() == 0) {
        d->userModel.addItem(payee);
    } else {
        d->userModel.modifyItem(payee);
    }
}

bool MyMoneyFile::dirty() const
{
    return d->m_dirty || d->anyModelDirty();
}

void MyMoneyFile::setDirty(bool dirty) const
{
    if (!dirty) {
        d->markModelsAsClean();
    }
    d->m_dirty = dirty;
}

#if 0
unsigned int MyMoneyFile::accountCount() const
{
    // Don't forget the
    return d->accountsModel.itemList().count() + 5;
}
#endif

void MyMoneyFile::ensureDefaultCurrency(MyMoneyAccount& acc) const
{
    if (acc.currencyId().isEmpty()) {
        if (!baseCurrency().id().isEmpty())
            acc.setCurrencyId(baseCurrency().id());
    }
}

MyMoneyAccount MyMoneyFile::liability() const
{
    return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Liability));
}

MyMoneyAccount MyMoneyFile::asset() const
{
    return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset));
}

MyMoneyAccount MyMoneyFile::expense() const
{
    return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense));
}

MyMoneyAccount MyMoneyFile::income() const
{
    return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Income));
}

MyMoneyAccount MyMoneyFile::equity() const
{
    return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Equity));
}

unsigned int MyMoneyFile::transactionCount(const QString& accountId) const
{
    return d->journalModel.transactionCount(accountId);
}

unsigned int MyMoneyFile::institutionCount() const
{
    return d->institutionsModel.itemList().count();
}

MyMoneyMoney MyMoneyFile::balance(const QString& id, const QDate& date) const
{
    if (date.isValid()) {
        MyMoneyBalanceCacheItem bal = d->m_balanceCache.balance(id, date);
        if (bal.isValid())
            return bal.balance();
    }

    if (!d->accountsModel.indexById(id).isValid()) {
        throw MYMONEYEXCEPTION_CSTRING("Cannot retrieve balance for unknown account");
    }

    const auto returnValue = d->journalModel.balance(id, date);

    if (date.isValid()) {
        d->m_balanceCache.insert(id, date, returnValue);
    }

    return returnValue;
}

MyMoneyMoney MyMoneyFile::balance(const QString& id) const
{
    return balance(id, QDate());
}

MyMoneyMoney MyMoneyFile::clearedBalance(const QString &id, const QDate& date) const
{
    MyMoneyMoney cleared;
    QList<MyMoneyTransaction> list;

    cleared = balance(id, date);

    MyMoneyAccount account = this->account(id);
    MyMoneyMoney factor(1, 1);
    if (account.accountGroup() == Account::Type::Liability || account.accountGroup() == Account::Type::Equity)
        factor = -factor;

    MyMoneyTransactionFilter filter;
    filter.addAccount(id);
    filter.setDateFilter(QDate(), date);
    filter.setReportAllSplits(false);
    filter.addState((int)TransactionFilter::State::NotReconciled);
    transactionList(list, filter);

    for (QList<MyMoneyTransaction>::const_iterator it_t = list.cbegin(); it_t != list.cend(); ++it_t) {
        const QList<MyMoneySplit>& splits = (*it_t).splits();
        for (QList<MyMoneySplit>::const_iterator it_s = splits.cbegin(); it_s != splits.cend(); ++it_s) {
            const MyMoneySplit &split = (*it_s);
            if (split.accountId() != id)
                continue;
            cleared -= split.shares();
        }
    }
    return cleared * factor;
}

MyMoneyMoney MyMoneyFile::totalBalance(const QString& id, const QDate& date) const
{

    MyMoneyMoney result(balance(id, date));

    const auto subAccountList = account(id).accountList();
    for (const auto& sAccount : qAsConst(subAccountList))
        result += totalBalance(sAccount, date);

    return result;
}

MyMoneyMoney MyMoneyFile::totalBalance(const QString& id) const
{
    return totalBalance(id, QDate());
}

void MyMoneyFile::warningMissingRate(const QString& fromId, const QString& toId) const
{
    MyMoneySecurity from, to;
    try {
        from = security(fromId);
        to = security(toId);
        qWarning("Missing price info for conversion from %s to %s", qPrintable(from.name()), qPrintable(to.name()));

    } catch (const MyMoneyException &e) {
        qWarning("Missing security caught in MyMoneyFile::warningMissingRate(). %s", e.what());
    }
}

QStringList MyMoneyFile::journalEntryIds(MyMoneyTransactionFilter filter) const
{
    QStringList list;
    const auto rows = d->journalModel.rowCount();
    for (int row = 0; row < rows;) {
        auto idx = d->journalModel.index(row, 0);
        const auto cnt = idx.data(eMyMoney::Model::TransactionSplitCountRole).toInt();
        if (d->journalModel.matchTransaction(idx, filter)) {
            for (int i = 0; i < cnt; ++i) {
                idx = d->journalModel.index(row + i, 0);
                list.append(idx.data(eMyMoney::Model::IdRole).toString());
            }
        }
        // we can skip to the first journalEntry of the next transaction directly
        row += cnt;
    }
    return list;
}

void MyMoneyFile::transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
    d->journalModel.transactionList(list, filter);
}

void MyMoneyFile::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
    d->journalModel.transactionList(list, filter);
}

QList<MyMoneyPayee> MyMoneyFile::payeeList() const
{
    return d->payeesModel.itemList();
}

QList<MyMoneyTag> MyMoneyFile::tagList() const
{
    return d->tagsModel.itemList();
}

QString MyMoneyFile::accountToCategory(const QString& accountId, bool includeStandardAccounts) const
{
    return d->accountsModel.accountIdToHierarchicalName(accountId, includeStandardAccounts);
}

QString MyMoneyFile::categoryToAccount(const QString& category, Account::Type type) const
{
    return d->accountsModel.accountNameToId(category, type);
}

QString MyMoneyFile::categoryToAccount(const QString& category) const
{
    return categoryToAccount(category, Account::Type::Unknown);
}

QString MyMoneyFile::nameToAccount(const QString& name) const
{

    QString id;

    // search the category in the asset accounts and if it is not found, try
    // to locate it in the liability accounts
    id = locateSubAccount(MyMoneyFile::instance()->asset(), name);
    if (id.isEmpty())
        id = locateSubAccount(MyMoneyFile::instance()->liability(), name);

    return id;
}

QString MyMoneyFile::parentName(const QString& name) const
{
    return name.section(MyMoneyAccount::accountSeparator(), 0, -2);
}

QString MyMoneyFile::locateSubAccount(const MyMoneyAccount& base, const QString& category) const
{
    MyMoneyAccount nextBase;
    QString level, remainder;
    level = category.section(AccountSeparator, 0, 0);
    remainder = category.section(AccountSeparator, 1);

    const auto accountList = base.accountList();
    for (const auto& sAccount : accountList) {
        nextBase = account(sAccount);
        if (nextBase.name() == level) {
            if (remainder.isEmpty()) {
                return nextBase.id();
            }
            return locateSubAccount(nextBase, remainder);
        }
    }
    return QString();
}

QString MyMoneyFile::value(const QString& key) const
{
    return d->parametersModel.itemById(key).value();
}

void MyMoneyFile::setValue(const QString& key, const QString& val)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->parametersModel.addItem(key, val);
}

void MyMoneyFile::deletePair(const QString& key)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->parametersModel.deleteItem(key);
}

void MyMoneyFile::addSchedule(MyMoneySchedule& sched)
{
    d->checkTransaction(Q_FUNC_INFO);

    if (sched.type() == eMyMoney::Schedule::Type::Any)
        throw MYMONEYEXCEPTION_CSTRING("Cannot store schedule without type");

    auto t = sched.transaction();

    const auto splits = t.splits();
    for (auto split : splits) {
        // the following line will throw an exception if the
        // account does not exist or is one of the standard accounts
        const auto acc = account(split.accountId());
        if (acc.id().isEmpty())
            throw MYMONEYEXCEPTION_CSTRING("Cannot add split with no account assigned");
        if (isStandardAccount(split.accountId()))
            throw MYMONEYEXCEPTION_CSTRING("Cannot add split referencing standard account");

        // If we have match information in a split don't copy it into the schedule
        if (split.isMatched()) {
            split.removeMatch();
            t.modifySplit(split);
        }
    }

    // Reset the imported flag of the transaction in any case
    t.setImported(false);
    sched.setTransaction(t);

    d->schedulesModel.addItem(sched);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Schedule, sched.id());
}

void MyMoneyFile::modifySchedule(const MyMoneySchedule& sched)
{
    d->checkTransaction(Q_FUNC_INFO);

    if (sched.type() == eMyMoney::Schedule::Type::Any)
        throw MYMONEYEXCEPTION_CSTRING("Cannot store schedule without type");

    const auto splits = sched.transaction().splits();
    for (const auto& split : splits) {
        // the following line will throw an exception if the
        // account does not exist or is one of the standard accounts
        auto acc = MyMoneyFile::account(split.accountId());
        if (acc.id().isEmpty())
            throw MYMONEYEXCEPTION_CSTRING("Cannot store split with no account assigned");
        if (isStandardAccount(split.accountId()))
            throw MYMONEYEXCEPTION_CSTRING("Cannot store split referencing standard account");
    }

    d->schedulesModel.modifyItem(sched);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Schedule, sched.id());
}

void MyMoneyFile::removeSchedule(const MyMoneySchedule& sched)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->schedulesModel.removeItem(sched);
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Schedule, sched.id());
}

MyMoneySchedule MyMoneyFile::schedule(const QString& id) const
{
    const auto schedule = d->schedulesModel.itemById(id);
    if (schedule.id().isEmpty()) {
        throw MYMONEYEXCEPTION_SSTRING(std::string("Schedule not found for id") + id.toStdString());
    }
    return schedule;
}

QList<MyMoneySchedule> MyMoneyFile::scheduleList(
    const QString& accountId,
    const Schedule::Type type,
    const Schedule::Occurrence occurrence,
    const Schedule::PaymentType paymentType,
    const QDate& startDate,
    const QDate& endDate,
    const bool overdue) const
{
    return d->schedulesModel.scheduleList(accountId, type, occurrence, paymentType, startDate, endDate, overdue);
}

QList<MyMoneySchedule> MyMoneyFile::scheduleList(
    const QString& accountId) const
{
    return scheduleList(accountId, Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                        QDate(), QDate(), false);
}

QList<MyMoneySchedule> MyMoneyFile::scheduleList() const
{
    return scheduleList(QString(), Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                        QDate(), QDate(), false);
}

MyMoneyTransaction MyMoneyFile::scheduledTransaction(const MyMoneySchedule& schedule, const AccountBalanceCache& balances) const
{
    MyMoneyTransaction t = schedule.transaction();

    if (schedule.type() == eMyMoney::Schedule::Type::LoanPayment) {
        try {
            MyMoneyForecast::calculateAutoLoan(schedule, t, balances);
        } catch (const MyMoneyException& e) {
            qDebug() << "Unable to load schedule details" << QString::fromLatin1(e.what());
        }
    }

    t.clearId();
    t.setEntryDate(QDate());
    return t;
}

QStringList MyMoneyFile::consistencyCheck()
{
    QStringList rc;

    /// @todo port to new model code
    QList<MyMoneyAccount> list;
    QList<MyMoneyAccount>::Iterator it_a;
    QList<MyMoneySchedule>::Iterator it_sch;
    QList<MyMoneyPayee>::Iterator it_p;
    QList<MyMoneyReport>::Iterator it_r;
    QStringList accountRebuild;

    QMap<QString, bool> interestAccounts;
    QMap<QString, MyMoneyTransaction> firstTransactionInAccount;

    MyMoneyAccount parent;
    MyMoneyAccount child;
    MyMoneyAccount toplevel;

    QString parentId;

    int problemCount = 0;
    int unfixedCount = 0;
    QString problemAccount;

    // check that we have a storage object
    d->checkTransaction(Q_FUNC_INFO);

    // get the current list of accounts
    accountList(list);
    // add the standard accounts
    list << MyMoneyFile::instance()->asset();
    list << MyMoneyFile::instance()->liability();
    list << MyMoneyFile::instance()->income();
    list << MyMoneyFile::instance()->expense();

    for (it_a = list.begin(); it_a != list.end(); ++it_a) {
        // no more checks for standard accounts
        if (isStandardAccount((*it_a).id())) {
            continue;
        }

        switch ((*it_a).accountGroup()) {
        case Account::Type::Asset:
            toplevel = asset();
            break;
        case Account::Type::Liability:
            toplevel = liability();
            break;
        case Account::Type::Expense:
            toplevel = expense();
            break;
        case Account::Type::Income:
            toplevel = income();
            break;
        case Account::Type::Equity:
            toplevel = equity();
            break;
        default:
            qWarning("%s:%d This should never happen!", __FILE__, __LINE__);
            break;
        }

        // check for loops in the hierarchy
        parentId = (*it_a).parentAccountId();
        try {
            bool dropOut = false;
            while (!isStandardAccount(parentId) && !dropOut) {
                parent = account(parentId);
                if (parent.id() == (*it_a).id()) {
                    // parent loops, so we need to re-parent to toplevel account
                    // find parent account in our list
                    problemCount++;
                    QList<MyMoneyAccount>::Iterator it_b;
                    for (it_b = list.begin(); it_b != list.end(); ++it_b) {
                        if ((*it_b).id() == parent.id()) {
                            if (problemAccount != (*it_a).name()) {
                                problemAccount = (*it_a).name();
                                rc << i18n("* Problem with account '%1'", problemAccount);
                                rc << i18n("  * Loop detected between this account and account '%1'.", (*it_b).name());
                                rc << i18n("    Reparenting account '%2' to top level account '%1'.", toplevel.name(), (*it_a).name());
                                (*it_a).setParentAccountId(toplevel.id());
                                if (accountRebuild.contains(toplevel.id()) == 0)
                                    accountRebuild << toplevel.id();
                                if (accountRebuild.contains((*it_a).id()) == 0)
                                    accountRebuild << (*it_a).id();
                                dropOut = true;
                                break;
                            }
                        }
                    }
                }
                parentId = parent.parentAccountId();
            }

        } catch (const MyMoneyException &) {
            // if we don't know about a parent, we catch it later
        }

        // check that the parent exists
        parentId = (*it_a).parentAccountId();
        try {
            parent = account(parentId);
            if ((*it_a).accountGroup() != parent.accountGroup()) {
                problemCount++;
                if (problemAccount != (*it_a).name()) {
                    problemAccount = (*it_a).name();
                    rc << i18n("* Problem with account '%1'", problemAccount);
                }
                // the parent belongs to a different group, so we reconnect to the
                // master group account (asset, liability, etc) to which this account
                // should belong and update it in the engine.
                rc << i18n("  * Parent account '%1' belongs to a different group.", parent.name());
                rc << i18n("    New parent account is the top level account '%1'.", toplevel.name());
                (*it_a).setParentAccountId(toplevel.id());

                // make sure to rebuild the sub-accounts of the top account
                // and the one we removed this account from
                if (accountRebuild.contains(toplevel.id()) == 0)
                    accountRebuild << toplevel.id();
                if (accountRebuild.contains(parent.id()) == 0)
                    accountRebuild << parent.id();
            } else if (!parent.accountList().contains((*it_a).id())) {
                problemCount++;
                if (problemAccount != (*it_a).name()) {
                    problemAccount = (*it_a).name();
                    rc << i18n("* Problem with account '%1'", problemAccount);
                }
                // parent exists, but does not have a reference to the account
                rc << i18n("  * Parent account '%1' does not contain '%2' as sub-account.", parent.name(), problemAccount);
                if (accountRebuild.contains(parent.id()) == 0)
                    accountRebuild << parent.id();
            }
        } catch (const MyMoneyException &) {
            // apparently, the parent does not exist anymore. we reconnect to the
            // master group account (asset, liability, etc) to which this account
            // should belong and update it in the engine.
            problemCount++;
            if (problemAccount != (*it_a).name()) {
                problemAccount = (*it_a).name();
                rc << i18n("* Problem with account '%1'", problemAccount);
            }
            rc << i18n("  * The parent with id %1 does not exist anymore.", parentId);
            rc << i18n("    New parent account is the top level account '%1'.", toplevel.name());
            (*it_a).setParentAccountId(toplevel.id());

            // make sure to rebuild the sub-accounts of the top account
            if (accountRebuild.contains(toplevel.id()) == 0)
                accountRebuild << toplevel.id();
        }

        // now check that all the children exist and have the correct type
        const auto accountList = (*it_a).accountList();
        for (const auto& accountID : accountList) {
            // check that the child exists
            try {
                child = account(accountID);
                if (child.parentAccountId() != (*it_a).id()) {
                    throw MYMONEYEXCEPTION_CSTRING("Child account has a different parent");
                }
            } catch (const MyMoneyException &) {
                problemCount++;
                if (problemAccount != (*it_a).name()) {
                    problemAccount = (*it_a).name();
                    rc << i18n("* Problem with account '%1'", problemAccount);
                }
                rc << i18n("  * Child account with id %1 does not exist anymore.", accountID);
                rc << i18n("    The child account list will be reconstructed.");
                if (accountRebuild.contains((*it_a).id()) == 0)
                    accountRebuild << (*it_a).id();
            }
        }

        // see if it is a loan account. if so, remember the assigned interest account
        if ((*it_a).isLoan()) {
            MyMoneyAccountLoan loan(*it_a);
            if (!loan.interestAccountId().isEmpty()) {
                interestAccounts[loan.interestAccountId()] = true;
            }
            try {
                payee(loan.payee());
            } catch (const MyMoneyException &) {
                problemCount++;
                if (problemAccount != (*it_a).name()) {
                    problemAccount = (*it_a).name();
                    rc << i18n("* Problem with account '%1'", problemAccount);
                }
                rc << i18n("  * The payee with id %1 referenced by the loan does not exist anymore.", loan.payee());
                rc << i18n("    The payee will be removed.");
                // remove the payee - the account will be modified in the engine later
                (*it_a).deletePair("payee");
            }
        }

        // check if it is a category and set the date to 1900-01-01 if different
        if ((*it_a).isIncomeExpense()) {
            if (((*it_a).openingDate().isValid() == false) || ((*it_a).openingDate() != QDate(1900, 1, 1))) {
                (*it_a).setOpeningDate(QDate(1900, 1, 1));
            }
        }

        // check for clear text online password in the online settings
        if (!(*it_a).onlineBankingSettings().value("password").isEmpty()) {
            if (problemAccount != (*it_a).name()) {
                problemAccount = (*it_a).name();
                rc << i18n("* Problem with account '%1'", problemAccount);
            }
            rc << i18n("  * Older versions of KMyMoney stored an OFX password for this account in cleartext.");
            rc << i18n("    Please open it in the account editor (Account/Edit account) once and press OK.");
            rc << i18n("    This will store the password in the KDE wallet and remove the cleartext version.");
            ++unfixedCount;
        }

        // if the account was modified, we need to update it in the engine
        const auto acc = d->accountsModel.itemById((*it_a).id());
        if (!(acc == (*it_a))) {
            if (!acc.id().isEmpty()) {
                d->accountsModel.modifyItem(*it_a);
            } else {
                rc << i18n("  * Unable to update account data in engine.");
                return rc;
            }
        }
    }

    if (!accountRebuild.isEmpty()) {
        rc << i18n("* Reconstructing the child lists for");
    }

    // clear the affected lists
    for (it_a = list.begin(); it_a != list.end(); ++it_a) {
        if (accountRebuild.contains((*it_a).id())) {
            rc << QString("  %1").arg((*it_a).name());
            // clear the account list
            (*it_a).removeAccountIds();
        }
    }

    // reconstruct the lists
    for (it_a = list.begin(); it_a != list.end(); ++it_a) {
        QList<MyMoneyAccount>::Iterator it;
        parentId = (*it_a).parentAccountId();
        if (accountRebuild.contains(parentId)) {
            for (it = list.begin(); it != list.end(); ++it) {
                if ((*it).id() == parentId) {
                    (*it).addAccountId((*it_a).id());
                    break;
                }
            }
        }
    }

    // update the engine objects
    for (it_a = list.begin(); it_a != list.end(); ++it_a) {
        const auto acc = d->accountsModel.itemById((*it_a).id());
        if (accountRebuild.contains((*it_a).id())) {
            if (!acc.id().isEmpty()) {
                d->accountsModel.modifyItem(*it_a);
            } else {
                rc << i18n("  * Unable to update account data for account %1 in engine", (*it_a).name());
            }
        }
    }

    // For some reason, files exist with invalid ids. This has been found in the payee id
    // so we fix them here
    QList<MyMoneyPayee> pList = payeeList();
    QMap<QString, QString>payeeConversionMap;

    auto checkPayeeAccountInfo = [&](const MyMoneyPayee& p) {
        QList<payeeIdentifier> idents = p.payeeIdentifiers();
        // Store ids which have to be stored in the map table
        for (auto& ident : idents) {
            if (ident->payeeIdentifierId() == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid()) {
                const auto payeeIdentifier = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(ident);
                const auto bic = payeeIdentifier->fullStoredBic();
                if (!bic.isEmpty()) {
                    if (bic.length() != 11) {
                        rc << i18n("  * Payee %1 has BIC with invalid length '%2'.\n    Must be 11 chars and is %3", p.name(), bic, bic.length());
                        ++problemCount;
                        ++unfixedCount;
                    }
                }
            }
        }
    };

    for (it_p = pList.begin(); it_p != pList.end(); ++it_p) {
        if ((*it_p).id().length() > 7) {
            // found one of those with an invalid ids
            // create a new one and store it in the map.
            MyMoneyPayee payee = (*it_p);
            payee.clearId();
            d->payeesModel.addItem(payee);
            payeeConversionMap[(*it_p).id()] = payee.id();
            rc << i18n("  * Payee %1 recreated with fixed id", payee.name());
            ++problemCount;
            checkPayeeAccountInfo(payee);
        } else {
            checkPayeeAccountInfo(*it_p);
        }
    }

    // Fix the transactions
    MyMoneyTransactionFilter filter;
    filter.setReportAllSplits(false);
    QList<MyMoneyTransaction> tList;
    transactionList(tList, filter);

    // Generate the list of interest accounts
    for (const auto& transaction : qAsConst(tList)) {
        const auto splits = transaction.splits();
        for (const auto& split : splits) {
            if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest))
                interestAccounts[split.accountId()] = true;
        }
    }
    KMMSet<Account::Type> supportedAccountTypes;
    supportedAccountTypes.insert(
        {Account::Type::Checkings, Account::Type::Savings, Account::Type::Cash, Account::Type::CreditCard, Account::Type::Asset, Account::Type::Liability});
    KMMStringSet reportedUnsupportedAccounts;

    const auto txProblemHeader(i18n("* Problems with transactions"));
    rc << txProblemHeader;

    for (const auto& transaction : qAsConst(tList)) {
        MyMoneyTransaction t = transaction;
        bool tChanged = false;
        QDate accountOpeningDate;
        QStringList accountList;
        if (!(t.value(QStringLiteral("kmm-matched-tx")).isEmpty() && t.value(QStringLiteral("kmm-match-split")).isEmpty())) {
            t.deletePair(QStringLiteral("kmm-matched-tx"));
            t.deletePair(QStringLiteral("kmm-match-split"));
            rc << i18n("  * Removed unused match information from transaction '%1'.", t.id());
            ++problemCount;
            tChanged = true;
        }

        const auto splits = t.splits();
        if (!t.splitSum().isZero()) {
            rc << i18n("  * Sum of splits in transaction '%1' posted on %2 is not zero.", t.id(), MyMoneyUtils::formatDate(t.postDate()));
            for (const auto& split : splits) {
                const auto accIdx = d->accountsModel.indexById(split.accountId());
                const auto name = accIdx.data(eMyMoney::Model::AccountFullHierarchyNameRole).toString();
                const auto acc = d->accountsModel.itemByIndex(accIdx);
                const auto security = MyMoneyFile::instance()->security(acc.currencyId());
                rc << i18n("    Account: %1, Amount: %2", name, MyMoneyUtils::formatMoney(split.shares(), security));
            }
            ++unfixedCount;
        }

        for (const auto& split : splits) {
            bool sChanged = false;
            MyMoneySplit s = split;
            if (payeeConversionMap.find(split.payeeId()) != payeeConversionMap.end()) {
                s.setPayeeId(payeeConversionMap[s.payeeId()]);
                sChanged = true;
                rc << i18n("  * Payee id updated in split of transaction '%1'.", t.id());
                ++problemCount;
            }

            try {
                const auto acc = d->accountsModel.itemById(s.accountId());
                // compute the newest opening date of all accounts involved in the transaction
                // in case the newest opening date is newer than the transaction post date, do one
                // of the following:
                //
                // a) for category and stock accounts: update the opening date of the account
                // b) for account types where the user cannot modify the opening date through
                //    the UI issue a warning (for each account only once)
                // c) others will be caught later
                if (!acc.isIncomeExpense() && !acc.isInvest()) {
                    if (acc.openingDate() > t.postDate()) {
                        if (!accountOpeningDate.isValid() || acc.openingDate() > accountOpeningDate) {
                            accountOpeningDate = acc.openingDate();
                        }
                        accountList << this->accountToCategory(acc.id());
                        if (!supportedAccountTypes.contains(acc.accountType())
                                && !reportedUnsupportedAccounts.contains(acc.id())) {
                            rc << i18n("  * Opening date of Account '%1' cannot be changed to support transaction '%2' post date.",
                                       this->accountToCategory(acc.id()), t.id());
                            reportedUnsupportedAccounts.insert(acc.id());
                            ++unfixedCount;
                        }
                    }
                } else {
                    if (acc.openingDate() > t.postDate()) {
                        rc << i18n("  * Transaction '%1' post date '%2' is older than opening date '%4' of account '%3'.",
                                   t.id(),
                                   MyMoneyUtils::formatDate(t.postDate()),
                                   acc.name(),
                                   MyMoneyUtils::formatDate(acc.openingDate()));

                        rc << i18n("    Account opening date updated.");
                        MyMoneyAccount newAcc = acc;
                        newAcc.setOpeningDate(t.postDate());
                        this->modifyAccount(newAcc);
                        ++problemCount;
                    }
                }

                // make sure, that shares and value have the same number if they
                // represent the same currency.
                if (t.commodity() == acc.currencyId() && s.shares().reduce() != s.value().reduce()) {
                    // use the value as master if the transaction is balanced
                    if (t.splitSum().isZero()) {
                        s.setShares(s.value());
                        rc << i18n("  * shares set to value in split of transaction '%1'.", t.id());
                    } else {
                        s.setValue(s.shares());
                        rc << i18n("  * value set to shares in split of transaction '%1'.", t.id());
                    }
                    sChanged = true;
                    ++problemCount;
                }

                // make sure that shares and value have the same sign
                if (!(s.shares().isZero() || s.value().isZero())) {
                    if ((s.shares().isNegative() ^ s.value().isNegative()) || (s.shares().isPositive() ^ s.value().isPositive())) {
                        rc << i18n("  * Split %2 in transaction '%1' contains different signs for shares and value. Please fix manually.", t.id(), split.id());
                        ++unfixedCount;
                    }
                }

                // keep a trace of the first usage of accounts
                // by remembering the first transaction
                if (!firstTransactionInAccount.contains(s.accountId())) {
                    firstTransactionInAccount.insert(s.accountId(), t);
                }

            } catch (const MyMoneyException &) {
                rc << i18n("  * Split %2 in transaction '%1' contains a reference to invalid account %3. Please fix manually.", t.id(), split.id(), split.accountId());
                ++unfixedCount;
            }

            // make sure the interest splits are marked correct as such
            if (interestAccounts.find(s.accountId()) != interestAccounts.end()
                    && s.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
                s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Interest));
                sChanged = true;
                rc << i18n("  * action marked as interest in split of transaction '%1'.", t.id());
                ++problemCount;
            }

            if (sChanged) {
                tChanged = true;
                t.modifySplit(s);
            }
        }

        // make sure that the transaction's post date is valid
        if (!t.postDate().isValid()) {
            tChanged = true;
            t.setPostDate(t.entryDate().isValid() ? t.entryDate() : QDate::currentDate());
            rc << i18n("  * Transaction '%1' has an invalid post date.", t.id());
            rc << i18n("    The post date was updated to '%1'.", MyMoneyUtils::formatDate(t.postDate()));
            ++problemCount;
        }
        // check if the transaction's post date is after the opening date
        // of all accounts involved in the transaction. In case it is not,
        // issue a warning with the details about the transaction incl.
        // the account names and dates involved
        if (accountOpeningDate.isValid() && t.postDate() < accountOpeningDate) {
            QDate originalPostDate = t.postDate();
#if 0
            // for now we do not activate the logic to move the post date to a later
            // point in time. This could cause some severe trouble if you have lots
            // of ancient data collected with older versions of KMyMoney that did not
            // enforce certain conditions like we do now.
            t.setPostDate(accountOpeningDate);
            tChanged = true;
            // copy the price information for investments to the new date
            QList<MyMoneySplit>::const_iterator it_t;
            for (const auto& split : qAsConst(t.splits())) {
                if ((split.action() != "Buy") &&
                        (split.action() != "Reinvest")) {
                    continue;
                }
                QString id = split.accountId();
                auto acc = this->account(id);
                MyMoneySecurity sec = this->security(acc.currencyId());
                MyMoneyPrice price(acc.currencyId(),
                                   sec.tradingCurrency(),
                                   t.postDate(),
                                   split.price(), "Transaction");
                this->addPrice(price);
                break;
            }
#endif
            rc << i18n("  * Transaction '%1' has a post date '%2' before one of the referenced account's opening date.",
                       t.id(),
                       MyMoneyUtils::formatDate(originalPostDate));
            rc << i18n("    Referenced accounts: %1", accountList.join(","));
            rc << i18n("    The post date was not updated to '%1'.", MyMoneyUtils::formatDate(accountOpeningDate));
            ++unfixedCount;
        }

        if (tChanged) {
            d->journalModel.modifyTransaction(t);
        }
    }

    // if no problems were reported, we remove the header again
    if (rc.constLast() == txProblemHeader) {
        rc.takeLast();
    }

    // Fix the schedules
    QList<MyMoneySchedule> schList = scheduleList();
    for (it_sch = schList.begin(); it_sch != schList.end(); ++it_sch) {
        MyMoneySchedule sch = (*it_sch);
        MyMoneyTransaction t = sch.transaction();
        auto tChanged = false;
        for (const auto& split : qAsConst(t.splits())) {
            MyMoneySplit s = split;
            bool sChanged = false;
            if (payeeConversionMap.find(split.payeeId()) != payeeConversionMap.end()) {
                s.setPayeeId(payeeConversionMap[s.payeeId()]);
                sChanged = true;
                rc << i18n("  * Payee id updated in split of schedule '%1'.", (*it_sch).name());
                ++problemCount;
            }
            if (!split.value().isZero() && split.shares().isZero()) {
                s.setShares(s.value());
                sChanged = true;
                rc << i18n("  * Split in scheduled transaction '%1' contained value != 0 and shares == 0.", (*it_sch).name());
                rc << i18n("    Shares set to value.");
                ++problemCount;
            }

            // make sure, we don't have a bankid stored with a split in a schedule
            if (!split.bankID().isEmpty()) {
                s.setBankID(QString());
                sChanged = true;
                rc << i18n("  * Removed bankid from split in scheduled transaction '%1'.", (*it_sch).name());
                ++problemCount;
            }

            // make sure, that shares and value have the same number if they
            // represent the same currency.
            try {
                const auto acc = this->account(s.accountId());
                if (t.commodity() == acc.currencyId()
                        && s.shares().reduce() != s.value().reduce()) {
                    // use the value as master if the transaction is balanced
                    if (t.splitSum().isZero()) {
                        s.setShares(s.value());
                        rc << i18n("  * shares set to value in split in schedule '%1'.", (*it_sch).name());
                    } else {
                        s.setValue(s.shares());
                        rc << i18n("  * value set to shares in split in schedule '%1'.", (*it_sch).name());
                    }
                    sChanged = true;
                    ++problemCount;
                }
            } catch (const MyMoneyException &) {
                rc << i18n("  * Split %2 in schedule '%1' contains a reference to invalid account %3. Please fix manually.", (*it_sch).name(), split.id(), split.accountId());
                ++unfixedCount;
            }
            if (sChanged) {
                t.modifySplit(s);
                tChanged = true;
            }
        }
        if (t.isImported()) {
            rc << i18n("  * Imported flag removed from schedule '%1'", (*it_sch).name());
            t.setImported(false);
            tChanged = true;
            ++problemCount;
        }
        if (tChanged) {
            sch.setTransaction(t);
            d->schedulesModel.modifyItem(sch);
        }
    }

    // Fix the reports
    QList<MyMoneyReport> rList = reportList();
    for (it_r = rList.begin(); it_r != rList.end(); ++it_r) {
        MyMoneyReport r = *it_r;
        QStringList payeeList;
        (*it_r).payees(payeeList);
        bool rChanged = false;
        for (auto it_payee = payeeList.begin(); it_payee != payeeList.end(); ++it_payee) {
            if (payeeConversionMap.find(*it_payee) != payeeConversionMap.end()) {
                rc << i18n("  * Payee id updated in report '%1'.", (*it_r).name());
                ++problemCount;
                r.removeReference(*it_payee);
                r.addPayee(payeeConversionMap[*it_payee]);
                rChanged = true;
            }
        }
        if (rChanged) {
            d->reportsModel.modifyItem(r);
        }
    }

    // erase old payee ids
    QMap<QString, QString>::Iterator it_m;
    for (it_m = payeeConversionMap.begin(); it_m != payeeConversionMap.end(); ++it_m) {
        MyMoneyPayee payee = this->payee(it_m.key());
        removePayee(payee);
        rc << i18n("  * Payee '%1' removed.", payee.id());
        ++problemCount;
    }

    // look for accounts which have currencies other than the base currency but no price on the date of first use
    // all accounts using base currency are excluded, since that's the base used for foreign currency calculation
    // thus it is considered as always present
    // accounts that represent Income/Expense categories are also excluded as price is irrelevant for their
    // fake opening date since a forex rate is required for all multi-currency transactions

    //get all currencies in use
    QStringList currencyList;
    QList<MyMoneyAccount> accountForeignCurrency;
    QList<MyMoneyAccount> accList;
    accountList(accList);
    QList<MyMoneyAccount>::const_iterator account_it;
    for (account_it = accList.cbegin(); account_it != accList.cend(); ++account_it) {
        MyMoneyAccount account = *account_it;
        if (!account.isIncomeExpense()
                && !currencyList.contains(account.currencyId())
                && account.currencyId() != baseCurrency().id()
                && !account.currencyId().isEmpty()) {
            //add the currency and the account-currency pair
            currencyList.append(account.currencyId());
            accountForeignCurrency.append(account);
        }
    }

    MyMoneyPriceList pricesList = priceList();
    QMap<MyMoneySecurityPair, QDate> securityPriceDate;

    //get the first date of the price for each security
    MyMoneyPriceList::const_iterator prices_it;
    for (prices_it = pricesList.cbegin(); prices_it != pricesList.cend(); ++prices_it) {
        MyMoneyPrice firstPrice = (*((*prices_it).cbegin()));

        //only check the price if the currency is in use
        if (currencyList.contains(firstPrice.from()) || currencyList.contains(firstPrice.to())) {
            // check the security in the from field
            // if it is there, use it since it is the first one
            QPair<QString, QString> pricePair = qMakePair(firstPrice.from(), firstPrice.to());
            securityPriceDate[pricePair] = firstPrice.date();
        }
    }

    // compare the dates with the dates of the first use of each currency
    QList<MyMoneyAccount>::const_iterator accForeignList_it;
    bool firstInvProblem = true;
    for (accForeignList_it = accountForeignCurrency.cbegin(); accForeignList_it != accountForeignCurrency.cend(); ++accForeignList_it) {
        // setup the price pair correctly
        QPair<QString, QString> pricePair;
        //setup the reverse, which can also be used for rate conversion
        QPair<QString, QString> reversePricePair;
        if ((*accForeignList_it).isInvest()) {
            //if it is a stock, we have to search for a price from its stock to the currency of the account
            QString securityId = (*accForeignList_it).currencyId();
            QString tradingCurrencyId = security(securityId).tradingCurrency();
            pricePair = qMakePair(securityId, tradingCurrencyId);
            reversePricePair = qMakePair(tradingCurrencyId, securityId);
        } else {
            //if it is a regular account we search for a price from the currency of the account to the base currency
            QString currency = (*accForeignList_it).currencyId();
            QString baseCurrencyId = baseCurrency().id();
            pricePair = qMakePair(currency, baseCurrencyId);
            reversePricePair = qMakePair(baseCurrencyId, currency);
        }

        // check if the account is in use and if a price is available for the date of use
        if (firstTransactionInAccount.contains((*accForeignList_it).id())) {
            const auto firstTransaction = firstTransactionInAccount.value((*accForeignList_it).id());
            if ((!securityPriceDate.contains(pricePair) || securityPriceDate.value(pricePair) > firstTransaction.postDate())
                && (!securityPriceDate.contains(reversePricePair) || securityPriceDate.value(reversePricePair) > firstTransaction.postDate())) {
                if (firstInvProblem) {
                    firstInvProblem = false;
                    rc << i18nc("@info consistency check", "* Potential problem with securities/currencies");
                }
                MyMoneySecurity secError = security((*accForeignList_it).currencyId());
                if (!(*accForeignList_it).isInvest()) {
                    rc << i18nc("@info consistency check",
                                "  * The account '%1' in currency '%2' has no price set for the date of its first usage on '%3'.",
                                (*accForeignList_it).name(),
                                secError.name(),
                                MyMoneyUtils::formatDate(firstTransaction.postDate()));
                    rc << i18nc("@info consistency check",
                                "    Please enter a price for the currency on or before '%1'.",
                                MyMoneyUtils::formatDate(firstTransaction.postDate()));
                    ++unfixedCount;
                } else {
                    rc << i18nc("@info consistency check",
                                "  * The security '%1' has no price set for the first use on '%2'.",
                                (*accForeignList_it).name(),
                                MyMoneyUtils::formatDate(firstTransaction.postDate()));
                    const auto split = firstTransaction.splitByAccount((*accForeignList_it).id());
                    if (((split.action() == QLatin1String("Buy")) || (split.action() == QLatin1String("Sell"))) && (!split.shares().isZero())) {
                        MyMoneyPrice pr(pricePair.first,
                                        pricePair.second,
                                        firstTransaction.postDate(),
                                        split.value() / split.shares(),
                                        i18nc("@info price source", "User"));
                        addPrice(pr);
                        rc << i18nc("@info consistency check", "    Price for the security has been extracted from transaction and added to price history.");
                        ++problemCount;
                    } else {
                        rc << i18nc("@info consistency check", "    Please enter a price for the security on or before this date.");
                        ++unfixedCount;
                    }
                }
            }
        }
    }

    // Fix the budgets that somehow still reference invalid accounts
    QString problemBudget;
    QList<MyMoneyBudget> bList = budgetList();
    for (QList<MyMoneyBudget>::const_iterator it_b = bList.cbegin(); it_b != bList.cend(); ++it_b) {
        MyMoneyBudget b = *it_b;
        QList<MyMoneyBudget::AccountGroup> baccounts = b.getaccounts();
        bool bChanged = false;
        for (QList<MyMoneyBudget::AccountGroup>::const_iterator it_bacc = baccounts.cbegin(); it_bacc != baccounts.cend(); ++it_bacc) {
            try {
                account((*it_bacc).id());
            } catch (const MyMoneyException &) {
                problemCount++;
                if (problemBudget != b.name()) {
                    problemBudget = b.name();
                    rc << i18n("* Problem with budget '%1'", problemBudget);
                }
                rc << i18n("  * The account with id %1 referenced by the budget does not exist anymore.", (*it_bacc).id());
                rc << i18n("    The account reference will be removed.");
                // remove the reference to the account
                b.removeReference((*it_bacc).id());
                bChanged = true;
            }
        }
        if (bChanged) {
            d->budgetsModel.modifyItem(b);
        }
    }

    // add more checks here

    if (problemCount == 0 && unfixedCount == 0) {
        rc << i18n("Finished: data is consistent.");
    } else {
        const QString problemsCorrected = i18np("%1 problem corrected.", "%1 problems corrected.", problemCount);
        const QString problemsRemaining = i18np("%1 problem still present.", "%1 problems still present.", unfixedCount);

        rc << QString();
        rc << i18nc("%1 is a string, e.g. 7 problems corrected; %2 is a string, e.g. 3 problems still present", "Finished: %1 %2", problemsCorrected, problemsRemaining);
    }
    return rc;
}

QString MyMoneyFile::createCategory(const MyMoneyAccount& base, const QString& name)
{
    d->checkTransaction(Q_FUNC_INFO);

    MyMoneyAccount parent = base;
    QString categoryText;

    if (base.id() != expense().id() && base.id() != income().id())
        throw MYMONEYEXCEPTION_CSTRING("Invalid base category");

    QStringList subAccounts = name.split(AccountSeparator);
    QStringList::Iterator it;
    for (it = subAccounts.begin(); it != subAccounts.end(); ++it) {
        MyMoneyAccount categoryAccount;

        categoryAccount.setName(*it);
        categoryAccount.setAccountType(base.accountType());

        if (it == subAccounts.begin())
            categoryText += *it;
        else
            categoryText += (AccountSeparator + *it);

        // Only create the account if it doesn't exist
        try {
            QString categoryId = categoryToAccount(categoryText);
            if (categoryId.isEmpty())
                addAccount(categoryAccount, parent);
            else {
                categoryAccount = account(categoryId);
            }
        } catch (const MyMoneyException &e) {
            qDebug("Unable to add account %s, %s, %s: %s",
                   qPrintable(categoryAccount.name()),
                   qPrintable(parent.name()),
                   qPrintable(categoryText),
                   e.what());
        }

        parent = categoryAccount;
    }

    return categoryToAccount(name);
}

QString MyMoneyFile::checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2)
{
    QString accountId;
    MyMoneyAccount newAccount;
    bool found = true;

    if (!name.isEmpty()) {
        // The category might be constructed with an arbitrary depth (number of
        // colon delimited fields). We try to find a parent account within this
        // hierarchy by searching the following sequence:
        //
        //    aaaa:bbbb:cccc:ddddd
        //
        // 1. search aaaa:bbbb:cccc:dddd, create nothing
        // 2. search aaaa:bbbb:cccc     , create dddd
        // 3. search aaaa:bbbb          , create cccc:dddd
        // 4. search aaaa               , create bbbb:cccc:dddd
        // 5. don't search              , create aaaa:bbbb:cccc:dddd

        newAccount.setName(name);
        QString accName;      // part to be created (right side in above list)
        QString parent(name); // a possible parent part (left side in above list)
        do {
            accountId = categoryToAccount(parent);
            if (accountId.isEmpty()) {
                found = false;
                // prepare next step
                if (!accName.isEmpty())
                    accName.prepend(':');
                accName.prepend(parent.section(':', -1));
                newAccount.setName(accName);
                parent = parent.section(':', 0, -2);
            } else if (!accName.isEmpty()) {
                newAccount.setParentAccountId(accountId);
            }
        } while (!parent.isEmpty() && accountId.isEmpty());

        // if we did not find the category,
        // but have a name for it, we create it
        if (!found && !accName.isEmpty()) {
            MyMoneyAccount parentAccount;
            if (newAccount.parentAccountId().isEmpty()) {
                if (!value.isNegative() && value2.isNegative())
                    parentAccount = income();
                else
                    parentAccount = expense();
            } else {
                parentAccount = account(newAccount.parentAccountId());
            }
            newAccount.setAccountType((!value.isNegative() && value2.isNegative()) ? Account::Type::Income : Account::Type::Expense);
            MyMoneyAccount brokerage;
            // clear out the parent id, because createAccount() does not like that
            newAccount.setParentAccountId(QString());
            createAccount(newAccount, parentAccount, brokerage, MyMoneyMoney());
            accountId = newAccount.id();
        }
    }

    return accountId;
}

void MyMoneyFile::addSecurity(MyMoneySecurity& security)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->securitiesModel.addItem(security);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Security, security.id());
}

void MyMoneyFile::modifySecurity(const MyMoneySecurity& security)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->securitiesModel.modifyItem(security);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Security, security.id());
}

void MyMoneyFile::removeSecurity(const MyMoneySecurity& security)
{
    d->checkTransaction(Q_FUNC_INFO);

    // FIXME check that security is not referenced by other object

    d->securitiesModel.removeItem(security);
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Security, security.id());
}

MyMoneySecurity MyMoneyFile::security(const QString& id) const
{
    if (Q_UNLIKELY(id.isEmpty()))
        return baseCurrency();

    // in case we don't find the id in the securities,
    // we search in the currencies
    MyMoneySecurity security = d->securitiesModel.itemById(id);
    if (security.id().isEmpty()) {
        security = d->currenciesModel.itemById(id);
        if (security.id().isEmpty()) {
            throw MYMONEYEXCEPTION(QString::fromLatin1("Security '%1' not found.").arg(id));
        }
    }
    return security;
}

QList<MyMoneySecurity> MyMoneyFile::securityList() const
{
    return d->securitiesModel.itemList();
}

void MyMoneyFile::addCurrency(const MyMoneySecurity& currency)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->currenciesModel.addCurrency(currency);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Currency, currency.id());
}

void MyMoneyFile::modifyCurrency(const MyMoneySecurity& currency)
{
    d->checkTransaction(Q_FUNC_INFO);

    // force reload of base currency object
    if (currency.id() == d->m_baseCurrency.id())
        d->m_baseCurrency.clearId();

    d->currenciesModel.modifyItem(currency);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Currency, currency.id());
}

void MyMoneyFile::removeCurrency(const MyMoneySecurity& currency)
{
    d->checkTransaction(Q_FUNC_INFO);

    if (currency.id() == d->m_baseCurrency.id())
        throw MYMONEYEXCEPTION_CSTRING("Cannot delete base currency.");

    // FIXME check that security is not referenced by other object

    d->currenciesModel.removeItem(currency);
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Currency, currency.id());
}

MyMoneySecurity MyMoneyFile::currency(const QString& id) const
{
    if (id.isEmpty())
        return baseCurrency();

    auto currency = d->currenciesModel.itemById(id);
    // in case we don't find a currency with this id, we try a security
    if (currency.id().isEmpty()) {
        currency = d->securitiesModel.itemById(id);
        if (currency.id().isEmpty()) {
            throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot retrieve currency with unknown id '%1'").arg(id));
        }
    }
    return currency;
}

QMap<MyMoneySecurity, MyMoneyPrice> MyMoneyFile::ancientCurrencies() const
{
    QMap<MyMoneySecurity, MyMoneyPrice> ancientCurrencies;
    const auto source = QStringLiteral("KMyMoney");

    ancientCurrencies.insert(MyMoneySecurity("ATS", i18n("Austrian Schilling"), QString::fromUtf8("ÖS")),
                             MyMoneyPrice("ATS", "EUR", QDate(1998, 12, 31), MyMoneyMoney(10000, 137603), source));
    ancientCurrencies.insert(MyMoneySecurity("DEM", i18n("German Mark"), "DM"),
                             MyMoneyPrice("DEM", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 195583), source));
    ancientCurrencies.insert(MyMoneySecurity("FRF", i18n("French Franc"), "FF"),
                             MyMoneyPrice("FRF", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 655957), source));
    ancientCurrencies.insert(MyMoneySecurity("ITL", i18n("Italian Lira"), QChar(0x20A4)),
                             MyMoneyPrice("ITL", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100, 193627), source));
    ancientCurrencies.insert(MyMoneySecurity("ESP", i18n("Spanish Peseta"), QString()),
                             MyMoneyPrice("ESP", "EUR", QDate(1998, 12, 31), MyMoneyMoney(1000, 166386), source));
    ancientCurrencies.insert(MyMoneySecurity("NLG", i18n("Dutch Guilder"), QString()),
                             MyMoneyPrice("NLG", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 220371), source));
    ancientCurrencies.insert(MyMoneySecurity("BEF", i18n("Belgian Franc"), "Fr"),
                             MyMoneyPrice("BEF", "EUR", QDate(1998, 12, 31), MyMoneyMoney(10000, 403399), source));
    ancientCurrencies.insert(MyMoneySecurity("LUF", i18n("Luxembourg Franc"), "Fr"),
                             MyMoneyPrice("LUF", "EUR", QDate(1998, 12, 31), MyMoneyMoney(10000, 403399), source));
    ancientCurrencies.insert(MyMoneySecurity("PTE", i18n("Portuguese Escudo"), QString()),
                             MyMoneyPrice("PTE", "EUR", QDate(1998, 12, 31), MyMoneyMoney(1000, 200482), source));
    ancientCurrencies.insert(MyMoneySecurity("IEP", i18n("Irish Pound"), QChar(0x00A3)),
                             MyMoneyPrice("IEP", "EUR", QDate(1998, 12, 31), MyMoneyMoney(1000000, 787564), source));
    ancientCurrencies.insert(MyMoneySecurity("FIM", i18n("Finnish Markka"), QString()),
                             MyMoneyPrice("FIM", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 594573), source));
    ancientCurrencies.insert(MyMoneySecurity("GRD", i18n("Greek Drachma"), QChar(0x20AF)),
                             MyMoneyPrice("GRD", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100, 34075), source));

    // https://en.wikipedia.org/wiki/Bulgarian_lev
    ancientCurrencies.insert(MyMoneySecurity("BGL", i18n("Bulgarian Lev"), "BGL"),
                             MyMoneyPrice("BGL", "BGN", QDate(1999, 7, 5), MyMoneyMoney(1, 1000), source));

    ancientCurrencies.insert(MyMoneySecurity("ROL", i18n("Romanian Leu"), "ROL"),
                             MyMoneyPrice("ROL", "RON", QDate(2005, 6, 30), MyMoneyMoney(1, 10000), source));

    ancientCurrencies.insert(MyMoneySecurity("RUR", i18n("Russian Ruble (old)"), "RUR"),
                             MyMoneyPrice("RUR", "RUB", QDate(1998, 1, 1), MyMoneyMoney(1, 1000), source));

    ancientCurrencies.insert(MyMoneySecurity("SIT", i18n("Slovenian Tolar"), "SIT"),
                             MyMoneyPrice("SIT", "EUR", QDate(2006, 12, 31), MyMoneyMoney(1, 23964), source));

    // Source: https://en.wikipedia.org/wiki/Turkish_lira
    ancientCurrencies.insert(MyMoneySecurity("TRL", i18n("Turkish Lira (old)"), "TL"),
                             MyMoneyPrice("TRL", "TRY", QDate(2004, 12, 31), MyMoneyMoney(1, 1000000), source));

    // Source: https://www.focus.de/finanzen/news/malta-und-zypern_aid_66058.html
    ancientCurrencies.insert(MyMoneySecurity("MTL", i18n("Maltese Lira"), "MTL"),
                             MyMoneyPrice("MTL", "EUR", QDate(2008, 1, 1), MyMoneyMoney(429300, 1000000), source));
    ancientCurrencies.insert(MyMoneySecurity("CYP", i18n("Cyprus Pound"), QString("C%1").arg(QChar(0x00A3))),
                             MyMoneyPrice("CYP", "EUR", QDate(2008, 1, 1), MyMoneyMoney(585274, 1000000), source));

    // Source: https://www.focus.de/finanzen/news/waehrungszone-slowakei-ist-neuer-euro-staat_aid_359025.html
    ancientCurrencies.insert(MyMoneySecurity("SKK", i18n("Slovak Koruna"), "SKK"),
                             MyMoneyPrice("SKK", "EUR", QDate(2008, 12, 31), MyMoneyMoney(1000, 30126), source));

    // Source: https://en.wikipedia.org/wiki/Mozambican_metical
    ancientCurrencies.insert(MyMoneySecurity("MZM", i18n("Mozambique Metical"), "MT"),
                             MyMoneyPrice("MZM", "MZN", QDate(2006, 7, 1), MyMoneyMoney(1, 1000), source));

    // Source: https://en.wikipedia.org/wiki/Azerbaijani_manat
    ancientCurrencies.insert(MyMoneySecurity("AZM", i18n("Azerbaijani Manat"), "m."),
                             MyMoneyPrice("AZM", "AZN", QDate(2006, 1, 1), MyMoneyMoney(1, 5000), source));

    // Source: https://en.wikipedia.org/wiki/Litas
    ancientCurrencies.insert(MyMoneySecurity("LTL", i18n("Lithuanian Litas"), "Lt"),
                             MyMoneyPrice("LTL", "EUR", QDate(2015, 1, 1), MyMoneyMoney(100000, 345280), source));

    // Source: https://en.wikipedia.org/wiki/Belarusian_ruble
    ancientCurrencies.insert(MyMoneySecurity("BYR", i18n("Belarusian Ruble (old)"), "BYR"),
                             MyMoneyPrice("BYR", "BYN", QDate(2016, 7, 1), MyMoneyMoney(1, 10000), source));

    // Source: https://en.wikipedia.org/wiki/Zambian_kwacha, triggered by b.k.o ticket #425530
    ancientCurrencies.insert(MyMoneySecurity("ZMK", i18n("Zambian Kwacha (old)"), "K"),
                             MyMoneyPrice("ZMK", "ZMW", QDate(2013, 1, 1), MyMoneyMoney(1, 1000), source));

    // Source: https://www.ecb.europa.eu/press/pr/date/2022/html/ecb.pr220712~b97dd38de3.en.html
    ancientCurrencies.insert(MyMoneySecurity("HRK", i18n("Croatian Kuna")),
                             MyMoneyPrice("HRK", "EUR", QDate(2023, 1, 1), MyMoneyMoney(100000, 753450), source));

    return ancientCurrencies;
}

/// @todo move to static function in MyMoneySecurity
QList<MyMoneySecurity> MyMoneyFile::availableCurrencyList() const
{
    QList<MyMoneySecurity> currencyList;
    // clang-format off
    currencyList.append(MyMoneySecurity("AFA", i18n("Afghanistan Afghani")));
    currencyList.append(MyMoneySecurity("ALL", i18n("Albanian Lek")));
    currencyList.append(MyMoneySecurity("ANG", i18n("Netherland Antillian Guilder")));
    currencyList.append(MyMoneySecurity("DZD", i18n("Algerian Dinar")));
    currencyList.append(MyMoneySecurity("ADF", i18n("Andorran Franc")));
    currencyList.append(MyMoneySecurity("ADP", i18n("Andorran Peseta")));
    currencyList.append(MyMoneySecurity("AOA", i18n("Angolan Kwanza"),         "Kz"));
    currencyList.append(MyMoneySecurity("ARS", i18n("Argentine Peso"),         "$"));
    currencyList.append(MyMoneySecurity("AWG", i18n("Aruban Florin")));
    currencyList.append(MyMoneySecurity("AUD", i18n("Australian Dollar"),      "$"));
    currencyList.append(MyMoneySecurity("AZN", i18n("Azerbaijani Manat"),      "m."));
    currencyList.append(MyMoneySecurity("BSD", i18n("Bahamian Dollar"),        "$"));
    currencyList.append(MyMoneySecurity("BHD", i18n("Bahraini Dinar"),         "BHD", 1000));
    currencyList.append(MyMoneySecurity("BDT", i18n("Bangladeshi Taka")));
    currencyList.append(MyMoneySecurity("BBD", i18n("Barbados Dollar"),        "$"));
    currencyList.append(MyMoneySecurity("BTC", i18n("Bitcoin"),                "BTC", 100000000, 100000000, 8));
    currencyList.append(MyMoneySecurity("BYN", i18n("Belarusian Ruble"),       "Br"));
    currencyList.append(MyMoneySecurity("BZD", i18n("Belize Dollar"),          "$"));
    currencyList.append(MyMoneySecurity("BMD", i18n("Bermudian Dollar"),       "$"));
    currencyList.append(MyMoneySecurity("BTN", i18n("Bhutan Ngultrum")));
    currencyList.append(MyMoneySecurity("BOB", i18n("Bolivian Boliviano")));
    currencyList.append(MyMoneySecurity("BAM", i18n("Bosnian Convertible Mark")));
    currencyList.append(MyMoneySecurity("BWP", i18n("Botswana Pula")));
    currencyList.append(MyMoneySecurity("BRL", i18n("Brazilian Real"),         "R$"));
    currencyList.append(MyMoneySecurity("GBP", i18n("British Pound"),          QChar(0x00A3)));
    currencyList.append(MyMoneySecurity("BND", i18n("Brunei Dollar"),          "$"));
    currencyList.append(MyMoneySecurity("BGN", i18n("Bulgarian Lev (new)")));
    currencyList.append(MyMoneySecurity("BIF", i18n("Burundi Franc")));
    currencyList.append(MyMoneySecurity("XAF", i18n("CFA Franc BEAC")));
    currencyList.append(MyMoneySecurity("XOF", i18n("CFA Franc BCEAO")));
    currencyList.append(MyMoneySecurity("XPF", i18n("CFP Franc Pacifique"), "F", 1, 100));
    currencyList.append(MyMoneySecurity("KHR", i18n("Cambodia Riel")));
    currencyList.append(MyMoneySecurity("CAD", i18n("Canadian Dollar"),        "$"));
    currencyList.append(MyMoneySecurity("CVE", i18n("Cape Verde Escudo")));
    currencyList.append(MyMoneySecurity("KYD", i18n("Cayman Islands Dollar"),  "$"));
    currencyList.append(MyMoneySecurity("CLP", i18n("Chilean Peso")));
    currencyList.append(MyMoneySecurity("CNY", i18n("Chinese Yuan Renminbi")));
    currencyList.append(MyMoneySecurity("COP", i18n("Colombian Peso")));
    currencyList.append(MyMoneySecurity("KMF", i18n("Comoros Franc")));
    currencyList.append(MyMoneySecurity("CRC", i18n("Costa Rican Colon"),      QChar(0x20A1)));
    currencyList.append(MyMoneySecurity("CUP", i18n("Cuban Peso")));
    currencyList.append(MyMoneySecurity("CUC", i18n("Cuban Convertible Peso")));
    currencyList.append(MyMoneySecurity("CZK", i18n("Czech Koruna")));
    currencyList.append(MyMoneySecurity("DKK", i18n("Danish Krone"),           "kr"));
    currencyList.append(MyMoneySecurity("DJF", i18n("Djibouti Franc")));
    currencyList.append(MyMoneySecurity("DOP", i18n("Dominican Peso")));
    currencyList.append(MyMoneySecurity("XCD", i18n("East Caribbean Dollar"),  "$"));
    currencyList.append(MyMoneySecurity("EGP", i18n("Egyptian Pound"),         QChar(0x00A3)));
    currencyList.append(MyMoneySecurity("SVC", i18n("El Salvador Colon")));
    currencyList.append(MyMoneySecurity("ERN", i18n("Eritrean Nakfa")));
    currencyList.append(MyMoneySecurity("EEK", i18n("Estonian Kroon")));
    currencyList.append(MyMoneySecurity("ETB", i18n("Ethiopian Birr")));
    currencyList.append(MyMoneySecurity("EUR", i18n("Euro"),                   QChar(0x20ac)));
    currencyList.append(MyMoneySecurity("FKP", i18n("Falkland Islands Pound"), QChar(0x00A3)));
    currencyList.append(MyMoneySecurity("FJD", i18n("Fiji Dollar"),            "$"));
    currencyList.append(MyMoneySecurity("GMD", i18n("Gambian Dalasi")));
    currencyList.append(MyMoneySecurity("GEL", i18n("Georgian Lari")));
    currencyList.append(MyMoneySecurity("GHC", i18n("Ghanaian Cedi")));
    currencyList.append(MyMoneySecurity("GIP", i18n("Gibraltar Pound"),        QChar(0x00A3)));
    currencyList.append(MyMoneySecurity("GTQ", i18n("Guatemalan Quetzal")));
    currencyList.append(MyMoneySecurity("GWP", i18n("Guinea-Bissau Peso")));
    currencyList.append(MyMoneySecurity("GYD", i18n("Guyanan Dollar"),         "$"));
    currencyList.append(MyMoneySecurity("HTG", i18n("Haitian Gourde")));
    currencyList.append(MyMoneySecurity("HNL", i18n("Honduran Lempira")));
    currencyList.append(MyMoneySecurity("HKD", i18n("Hong Kong Dollar"),       "$"));
    currencyList.append(MyMoneySecurity("HUF", i18n("Hungarian Forint"),       "HUF", 1, 100));
    currencyList.append(MyMoneySecurity("ISK", i18n("Iceland Krona")));
    currencyList.append(MyMoneySecurity("INR", i18n("Indian Rupee"),           QChar(0x20B9)));
    currencyList.append(MyMoneySecurity("IDR", i18n("Indonesian Rupiah"),      "IDR", 1, 0, 10));
    currencyList.append(MyMoneySecurity("IRR", i18n("Iranian Rial"),           "IRR", 1));
    currencyList.append(MyMoneySecurity("IQD", i18n("Iraqi Dinar"),            "IQD", 1000));
    currencyList.append(MyMoneySecurity("ILS", i18n("Israeli New Shekel"),     QChar(0x20AA)));
    currencyList.append(MyMoneySecurity("JMD", i18n("Jamaican Dollar"),        "$"));
    currencyList.append(MyMoneySecurity("JPY", i18n("Japanese Yen"),           QChar(0x00A5), 1));
    currencyList.append(MyMoneySecurity("JOD", i18n("Jordanian Dinar"),        "JOD", 1000));
    currencyList.append(MyMoneySecurity("KZT", i18n("Kazakhstan Tenge")));
    currencyList.append(MyMoneySecurity("KES", i18n("Kenyan Shilling")));
    currencyList.append(MyMoneySecurity("KWD", i18n("Kuwaiti Dinar"),          "KWD", 1000));
    currencyList.append(MyMoneySecurity("KGS", i18n("Kyrgyzstan Som")));
    currencyList.append(MyMoneySecurity("LAK", i18n("Laos Kip"),               QChar(0x20AD)));
    currencyList.append(MyMoneySecurity("LVL", i18n("Latvian Lats")));
    currencyList.append(MyMoneySecurity("LBP", i18n("Lebanese Pound"),         QChar(0x00A3)));
    currencyList.append(MyMoneySecurity("LSL", i18n("Lesotho Loti")));
    currencyList.append(MyMoneySecurity("LRD", i18n("Liberian Dollar"),        "$"));
    currencyList.append(MyMoneySecurity("LYD", i18n("Libyan Dinar"),           "LYD", 1000));
    currencyList.append(MyMoneySecurity("MOP", i18n("Macau Pataca")));
    currencyList.append(MyMoneySecurity("MKD", i18n("Macedonian Denar")));
    currencyList.append(MyMoneySecurity("MGF", i18n("Malagasy Franc"),         "MGF", 500));
    currencyList.append(MyMoneySecurity("MWK", i18n("Malawi Kwacha")));
    currencyList.append(MyMoneySecurity("MYR", i18n("Malaysian Ringgit")));
    currencyList.append(MyMoneySecurity("MVR", i18n("Maldive Rufiyaa")));
    currencyList.append(MyMoneySecurity("MLF", i18n("Mali Republic Franc")));
    currencyList.append(MyMoneySecurity("MRO", i18n("Mauritanian Ouguiya"),    "MRO", 5));
    currencyList.append(MyMoneySecurity("MUR", i18n("Mauritius Rupee")));
    currencyList.append(MyMoneySecurity("MXN", i18n("Mexican Peso"),           "$"));
    currencyList.append(MyMoneySecurity("MDL", i18n("Moldavian Leu")));
    currencyList.append(MyMoneySecurity("MNT", i18n("Mongolian Tugrik"),       QChar(0x20AE)));
    currencyList.append(MyMoneySecurity("MAD", i18n("Moroccan Dirham")));
    currencyList.append(MyMoneySecurity("MZN", i18n("Mozambique Metical"),     "MT"));
    currencyList.append(MyMoneySecurity("MMK", i18n("Myanmar Kyat")));
    currencyList.append(MyMoneySecurity("NAD", i18n("Namibian Dollar"),        "$"));
    currencyList.append(MyMoneySecurity("NPR", i18n("Nepalese Rupee")));
    currencyList.append(MyMoneySecurity("NZD", i18n("New Zealand Dollar"),     "$"));
    currencyList.append(MyMoneySecurity("NIC", i18n("Nicaraguan Cordoba Oro")));
    currencyList.append(MyMoneySecurity("NGN", i18n("Nigerian Naira"),         QChar(0x20A6)));
    currencyList.append(MyMoneySecurity("KPW", i18n("North Korean Won"),       QChar(0x20A9)));
    currencyList.append(MyMoneySecurity("NOK", i18n("Norwegian Kroner"),       "kr"));
    currencyList.append(MyMoneySecurity("OMR", i18n("Omani Rial"),             "OMR", 1000));
    currencyList.append(MyMoneySecurity("PKR", i18n("Pakistan Rupee")));
    currencyList.append(MyMoneySecurity("PAB", i18n("Panamanian Balboa")));
    currencyList.append(MyMoneySecurity("PGK", i18n("Papua New Guinea Kina")));
    currencyList.append(MyMoneySecurity("PYG", i18n("Paraguay Guarani")));
    currencyList.append(MyMoneySecurity("PEN", i18n("Peruvian Nuevo Sol")));
    currencyList.append(MyMoneySecurity("PHP", i18n("Philippine Peso"),        QChar(0x20B1)));
    currencyList.append(MyMoneySecurity("PLN", i18n("Polish Zloty")));
    currencyList.append(MyMoneySecurity("QAR", i18n("Qatari Rial")));
    currencyList.append(MyMoneySecurity("RON", i18n("Romanian Leu (new)")));
    currencyList.append(MyMoneySecurity("RUB", i18n("Russian Ruble")));
    currencyList.append(MyMoneySecurity("RWF", i18n("Rwanda Franc")));
    currencyList.append(MyMoneySecurity("WST", i18n("Samoan Tala")));
    currencyList.append(MyMoneySecurity("STD", i18n("Sao Tome and Principe Dobra")));
    currencyList.append(MyMoneySecurity("SAR", i18n("Saudi Riyal")));
    currencyList.append(MyMoneySecurity("RSD", i18n("Serbian Dinar")));
    currencyList.append(MyMoneySecurity("SCR", i18n("Seychelles Rupee")));
    currencyList.append(MyMoneySecurity("SLL", i18n("Sierra Leone Leone")));
    currencyList.append(MyMoneySecurity("SGD", i18n("Singapore Dollar"),       "$"));
    currencyList.append(MyMoneySecurity("SBD", i18n("Solomon Islands Dollar"), "$"));
    currencyList.append(MyMoneySecurity("SOS", i18n("Somali Shilling")));
    currencyList.append(MyMoneySecurity("ZAR", i18n("South African Rand")));
    currencyList.append(MyMoneySecurity("KRW", i18n("South Korean Won"),       QChar(0x20A9), 1));
    currencyList.append(MyMoneySecurity("LKR", i18n("Sri Lanka Rupee")));
    currencyList.append(MyMoneySecurity("SHP", i18n("St. Helena Pound"),       QChar(0x00A3)));
    currencyList.append(MyMoneySecurity("SDD", i18n("Sudanese Dinar")));
    currencyList.append(MyMoneySecurity("SRG", i18n("Suriname Guilder")));
    currencyList.append(MyMoneySecurity("SZL", i18n("Swaziland Lilangeni")));
    currencyList.append(MyMoneySecurity("SEK", i18n("Swedish Krona")));
    currencyList.append(MyMoneySecurity("CHF", i18n("Swiss Franc"),            "SFr"));
    currencyList.append(MyMoneySecurity("SYP", i18n("Syrian Pound"),           QChar(0x00A3)));
    currencyList.append(MyMoneySecurity("TWD", i18n("Taiwan Dollar"),          "$"));
    currencyList.append(MyMoneySecurity("TJS", i18n("Tajikistan Somoni")));
    currencyList.append(MyMoneySecurity("TZS", i18n("Tanzanian Shilling")));
    currencyList.append(MyMoneySecurity("THB", i18n("Thai Baht"),              QChar(0x0E3F)));
    currencyList.append(MyMoneySecurity("TOP", i18n("Tongan Pa'anga")));
    currencyList.append(MyMoneySecurity("TTD", i18n("Trinidad and Tobago Dollar"), "$"));
    currencyList.append(MyMoneySecurity("TND", i18n("Tunisian Dinar"),         "TND", 1000));
    currencyList.append(MyMoneySecurity("TRY", i18n("Turkish Lira"), QChar(0x20BA)));
    currencyList.append(MyMoneySecurity("TMM", i18n("Turkmenistan Manat")));
    currencyList.append(MyMoneySecurity("USD", i18n("US Dollar"),              "$"));
    currencyList.append(MyMoneySecurity("UGX", i18n("Uganda Shilling")));
    currencyList.append(MyMoneySecurity("UAH", i18n("Ukraine Hryvnia")));
    currencyList.append(MyMoneySecurity("CLF", i18n("Unidad de Fometo")));
    currencyList.append(MyMoneySecurity("AED", i18n("United Arab Emirates Dirham")));
    currencyList.append(MyMoneySecurity("UYU", i18n("Uruguayan Peso")));
    currencyList.append(MyMoneySecurity("UZS", i18n("Uzbekistani Sum")));
    currencyList.append(MyMoneySecurity("VUV", i18n("Vanuatu Vatu")));
    currencyList.append(MyMoneySecurity("VEB", i18n("Venezuelan Bolivar")));
    currencyList.append(MyMoneySecurity("VND", i18n("Vietnamese Dong"),        QChar(0x20AB)));
    currencyList.append(MyMoneySecurity("ZMW", i18n("Zambian Kwacha"),         "K"));
    currencyList.append(MyMoneySecurity("ZWD", i18n("Zimbabwe Dollar"),        "$"));
    // clang-format on

    currencyList.append(ancientCurrencies().keys());

    // sort the currencies ...
    std::sort(currencyList.begin(), currencyList.end(),
              [] (const MyMoneySecurity& c1, const MyMoneySecurity& c2)
    {
        return c1.name().compare(c2.name()) < 0;
    });

    // ... and add a few precious metals at the end
    // clang-format off
    currencyList.append(MyMoneySecurity("XAU", i18n("Gold"),       "XAU", 1000000));
    currencyList.append(MyMoneySecurity("XPD", i18n("Palladium"),  "XPD", 1000000));
    currencyList.append(MyMoneySecurity("XPT", i18n("Platinum"),   "XPT", 1000000));
    currencyList.append(MyMoneySecurity("XAG", i18n("Silver"),     "XAG", 1000000));
    // clang-format on

    return currencyList;
}

QList<MyMoneySecurity> MyMoneyFile::currencyList() const
{
    return d->currenciesModel.itemList();
}

QString MyMoneyFile::foreignCurrency(const QString& first, const QString& second) const
{
    if (baseCurrency().id() == second)
        return first;
    return second;
}

MyMoneySecurity MyMoneyFile::baseCurrency() const
{
    if (d->m_baseCurrency.id().isEmpty()) {
        QString id = QString(value("kmm-baseCurrency"));
        if (!id.isEmpty())
            d->m_baseCurrency = currency(id);
    }

    return d->m_baseCurrency;
}

void MyMoneyFile::setBaseCurrency(const MyMoneySecurity& curr)
{
    // make sure the currency exists
    MyMoneySecurity c = currency(curr.id());

    if (c.id() != d->m_baseCurrency.id()) {
        setValue("kmm-baseCurrency", curr.id());
        // force reload of base currency cache
        d->m_baseCurrency = c;
        d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::BaseCurrency, c.id());
    }
}

void MyMoneyFile::addPrice(const MyMoneyPrice& price)
{
    if (price.rate().isZero())
        return;

    d->checkTransaction(Q_FUNC_INFO);

    // store the account's which are affected by this price regarding their value
    d->priceChanged(price);

    d->priceModel.addPrice(price);
}

void MyMoneyFile::removePrice(const MyMoneyPrice& price)
{
    d->checkTransaction(Q_FUNC_INFO);

    // store the account's which are affected by this price regarding their value
    d->priceChanged(price);

    d->priceModel.removePrice(price);
}

MyMoneyPrice MyMoneyFile::price(const QString& fromId, const QString& toId, const QDate& date, const bool exactDate) const
{
    QString to(toId);
    if (to.isEmpty()) {
        to = value("kmm-baseCurrency");
    }
    // if any id is missing at that point,
    // we can safely return an empty price object
    if (fromId.isEmpty() || to.isEmpty())
        return MyMoneyPrice();

    // we don't interrogate our tables if someone asks stupid stuff
    if (fromId == toId) {
        return MyMoneyPrice(fromId, toId, date, MyMoneyMoney::ONE, "KMyMoney");
    }

    // if not asking for exact date, try to find the exact date match first,
    // either the requested price or its reciprocal value. If unsuccessful, it will move
    // on and look for prices of previous dates
    MyMoneyPrice rc = d->priceModel.price(fromId, to, date, true);
    if (!rc.isValid()) {
        // not found, search 'to-from' rate and use reciprocal value
        rc = d->priceModel.price(to, fromId, date, true);

        // not found, search previous dates, if exact date is not needed
        if (!exactDate && !rc.isValid()) {
            // search 'from-to' and 'to-from', select the most recent one
            MyMoneyPrice fromPrice = d->priceModel.price(fromId, to, date, exactDate);
            MyMoneyPrice toPrice = d->priceModel.price(to, fromId, date, exactDate);

            // check first whether both prices are valid
            if (fromPrice.isValid() && toPrice.isValid()) {
                if (fromPrice.date() >= toPrice.date()) {
                    // if 'from-to' is newer or the same date, prefer that one
                    rc = fromPrice;
                } else {
                    // otherwise, use the reciprocal price
                    rc = toPrice;
                }
            } else if (fromPrice.isValid()) { // check if any of the prices is valid, return that one
                rc = fromPrice;
            } else if (toPrice.isValid()) {
                rc = toPrice;
            }
        }
    }
    return rc;
}

MyMoneyPrice MyMoneyFile::price(const QString& fromId, const QString& toId) const
{
    return price(fromId, toId, QDate::currentDate(), false);
}

MyMoneyPrice MyMoneyFile::price(const QString& fromId) const
{
    return price(fromId, QString(), QDate::currentDate(), false);
}


MyMoneyPriceList MyMoneyFile::priceList() const
{
    return d->priceModel.priceList();
}

bool MyMoneyFile::hasAccount(const QString& id, const QString& name) const
{
    const auto accounts = account(id).accountList();
    for (const auto& acc : accounts) {
        if (account(acc).name().compare(name) == 0)
            return true;
    }
    return false;
}

void MyMoneyFile::addReport(MyMoneyReport& report)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->reportsModel.addItem(report);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Report, report.id());
}

void MyMoneyFile::modifyReport(const MyMoneyReport& report)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->reportsModel.modifyItem(report);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Report, report.id());
}

void MyMoneyFile::removeReport(const MyMoneyReport& report)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->reportsModel.removeItem(report);
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Report, report.id());
}

MyMoneyReport MyMoneyFile::report(const QString& id) const
{
    return d->reportsModel.itemById(id);
}

QList<MyMoneyReport> MyMoneyFile::reportList() const
{
    return d->reportsModel.itemList();
}


unsigned MyMoneyFile::countReports() const
{
    return d->reportsModel.rowCount();
}

QList<MyMoneyBudget> MyMoneyFile::budgetList() const
{
    return d->budgetsModel.itemList();
}

void MyMoneyFile::addBudget(MyMoneyBudget &budget)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->budgetsModel.addItem(budget);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::Budget, budget.id());
}

MyMoneyBudget MyMoneyFile::budgetByName(const QString& name) const
{
    MyMoneyBudget budget = d->budgetsModel.itemByName(name);
    if (budget.id().isEmpty()) {
        throw MYMONEYEXCEPTION(QString::fromLatin1("Unknown budget '%1'").arg(name));
    }
    return budget;
}

void MyMoneyFile::modifyBudget(const MyMoneyBudget& budget)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->budgetsModel.modifyItem(budget);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::Budget, budget.id());
}

unsigned MyMoneyFile::countBudgets() const
{
    return d->budgetsModel.rowCount();
}

MyMoneyBudget MyMoneyFile::budget(const QString& id) const
{
    return d->budgetsModel.itemById(id);
}

void MyMoneyFile::removeBudget(const MyMoneyBudget& budget)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->budgetsModel.removeItem(budget);
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::Budget, budget.id());
}

void MyMoneyFile::addOnlineJob(onlineJob& job)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->onlineJobsModel.addItem(job);
    d->m_changeSet += MyMoneyNotification(File::Mode::Add, File::Object::OnlineJob, job.id());
}

void MyMoneyFile::modifyOnlineJob(const onlineJob job)
{
    d->checkTransaction(Q_FUNC_INFO);

    d->onlineJobsModel.modifyItem(job);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, File::Object::OnlineJob, job.id());
}

onlineJob MyMoneyFile::getOnlineJob(const QString &jobId) const
{
    return d->onlineJobsModel.itemById(jobId);
}

QList<onlineJob> MyMoneyFile::onlineJobList() const
{
    return d->onlineJobsModel.itemList();
}

/** @todo improve speed by passing count job to m_storage */
int MyMoneyFile::countOnlineJobs() const
{
    return d->onlineJobsModel.rowCount();
}

/**
 * @brief Remove onlineJob
 * @param job onlineJob to remove
 */
void MyMoneyFile::removeOnlineJob(const onlineJob& job)
{
    d->checkTransaction(Q_FUNC_INFO);

    // clear all changed objects from cache
    if (job.isLocked()) {
        return;
    }
    d->m_changeSet += MyMoneyNotification(File::Mode::Remove, File::Object::OnlineJob, job.id());
    d->onlineJobsModel.removeItem(job);
}

void MyMoneyFile::removeOnlineJob(const QStringList onlineJobIds)
{
    for (const auto& jobId : onlineJobIds) {
        removeOnlineJob(getOnlineJob(jobId));
    }
}

void MyMoneyFile::costCenterList(QList< MyMoneyCostCenter >& list) const
{
    list = d->costCenterModel.itemList();
}

void MyMoneyFile::updateVAT(MyMoneyTransaction& transaction) const
{
    // check if transaction qualifies
    const auto splitCount = transaction.splits().count();
    if (splitCount > 1 && splitCount <= 3) {
        MyMoneyMoney amount;
        MyMoneyAccount assetLiability;
        MyMoneyAccount category;
        MyMoneySplit taxSplit;
        const QString currencyId = transaction.commodity();
        for (const auto& split : qAsConst(transaction.splits())) {
            const auto acc = account(split.accountId());
            // all splits must reference accounts denoted in the same currency
            if (acc.currencyId() != currencyId) {
                return;
            }
            if (acc.isAssetLiability() && assetLiability.id().isEmpty()) {
                amount = split.shares();
                assetLiability = acc;
                continue;
            }
            if (acc.isAssetLiability()) {
                return;
            }
            if (category.id().isEmpty() && !acc.value("VatAccount").isEmpty()) {
                category = acc;
                continue;
            } else if (taxSplit.id().isEmpty() && !acc.isInTaxReports()) {
                taxSplit = split;
                continue;
            }
            return;
        }
        if (!category.id().isEmpty()) {
            // remove a possibly found tax split - we create a new one
            // but only if it is the same tax category
            if (!taxSplit.id().isEmpty()) {
                if (category.value("VatAccount").compare(taxSplit.accountId()))
                    return;
                transaction.removeSplit(taxSplit);
            }
            addVATSplit(transaction, assetLiability, category, amount);
        }
    }
}

bool MyMoneyFile::addVATSplit(MyMoneyTransaction& transaction, const MyMoneyAccount& acc, const MyMoneyAccount& category, const MyMoneyMoney& amount) const
{
    bool rc = false;

    try {
        MyMoneySplit tax;  // tax

        if (category.value("VatAccount").isEmpty())
            return false;
        MyMoneyAccount vatAcc = account(category.value("VatAccount"));
        const MyMoneySecurity& asec = security(acc.currencyId());
        const MyMoneySecurity& csec = security(category.currencyId());
        const MyMoneySecurity& vsec = security(vatAcc.currencyId());
        if (asec.id() != csec.id() || asec.id() != vsec.id()) {
            qDebug("Auto VAT assignment only works if all three accounts use the same currency.");
            return false;
        }

        MyMoneyMoney vatRate(vatAcc.value("VatRate"));
        MyMoneyMoney gv, nv;    // gross value, net value
        int fract = acc.fraction();

        if (!vatRate.isZero()) {

            tax.setAccountId(vatAcc.id());

            // qDebug("vat amount is '%s'", category.value("VatAmount").toLatin1());
            if (category.value("VatAmount").toLower() != QString("net")) {
                // split value is the gross value
                gv = amount;
                nv = (gv / (MyMoneyMoney::ONE + vatRate)).convert(fract);
                MyMoneySplit catSplit = transaction.splitByAccount(acc.id(), false);
                catSplit.setShares(-nv);
                catSplit.setValue(catSplit.shares());
                transaction.modifySplit(catSplit);

            } else {
                // split value is the net value
                nv = amount;
                gv = (nv * (MyMoneyMoney::ONE + vatRate)).convert(fract);
                MyMoneySplit accSplit = transaction.splitByAccount(acc.id());
                accSplit.setValue(gv.convert(fract));
                accSplit.setShares(accSplit.value());
                transaction.modifySplit(accSplit);
            }

            tax.setValue(-(gv - nv).convert(fract));
            tax.setShares(tax.value());
            transaction.addSplit(tax);
            rc = true;
        }
    } catch (const MyMoneyException &) {
    }
    return rc;
}

bool MyMoneyFile::isReferenced(const MyMoneyObject& obj, const QBitArray& skipCheck) const
{
    Q_ASSERT(skipCheck.count() == (int)eStorage::Reference::Count);

    // We delete all references in reports when an object
    // is deleted, so we don't need to check here. See
    // MyMoneyStorageMgr::removeReferences(). In case
    // you miss the report checks in the following lines ;)
    const auto id = obj.id();
    return isReferenced(id, skipCheck);
}

bool MyMoneyFile::isReferenced(const QString& id, const QBitArray& skipCheck) const
{
    // FIXME optimize the list of objects we have to checks
    //       with a bit of knowledge of the internal structure, we
    //       could optimize the number of objects we check for references

    // Scan all engine objects for a reference
    if (!skipCheck.testBit((int)eStorage::Reference::Transaction))
        if (d->journalModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Account))
        if (d->accountsModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Institution))
        if (d->institutionsModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Payee))
        if (d->payeesModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Tag))
        if (d->tagsModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Budget))
        if (d->budgetsModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Schedule))
        if (d->schedulesModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Security))
        if (d->securitiesModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Currency))
        if (d->currenciesModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::CostCenter))
        if (d->costCenterModel.hasReferenceTo(id))
            return true;

    if (!skipCheck.testBit((int)eStorage::Reference::Price)) {
        if (d->priceModel.hasReferenceTo(id))
            return true;
    }

    return false;
}

bool MyMoneyFile::isReferenced(const MyMoneyObject& obj) const
{
    return isReferenced(obj, QBitArray((int)eStorage::Reference::Count));
}

bool MyMoneyFile::isReferenced(const QString& id) const
{
    return isReferenced(id, QBitArray((int)eStorage::Reference::Count));
}

KMMStringSet MyMoneyFile::referencedObjects() const
{
    KMMStringSet ids(d->journalModel.referencedObjects());
    ids.unite(d->accountsModel.referencedObjects());
    ids.unite(d->institutionsModel.referencedObjects());
    ids.unite(d->payeesModel.referencedObjects());
    ids.unite(d->tagsModel.referencedObjects());
    ids.unite(d->budgetsModel.referencedObjects());
    ids.unite(d->schedulesModel.referencedObjects());
    ids.unite(d->securitiesModel.referencedObjects());
    ids.unite(d->currenciesModel.referencedObjects());
    ids.unite(d->costCenterModel.referencedObjects());
    ids.unite(d->priceModel.referencedObjects());
    // we never report the empty id
    ids.erase(QString());
    return ids;
}

bool MyMoneyFile::checkNoUsed(const QString& accId, const QString& no) const
{
    // by definition, an empty string or a non-numeric string is not used
    static const QRegularExpression checkNumberExp(QLatin1String("(.*\\D)?(\\d+)(\\D.*)?"));
    if (no.isEmpty() || !checkNumberExp.match(no).hasMatch())
        return false;

    const auto model = &d->journalModel;
    const auto rows = model->rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = model->index(row, 0);
        if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accId) {
            const auto number = idx.data(eMyMoney::Model::JournalSplitNumberRole).toString();
            if (!number.isEmpty() && number == no) {
                return true;
            }
        }
    }
    return false;
}

QString MyMoneyFile::highestCheckNo(const QString& accId) const
{
    unsigned64 lno = 0;
    unsigned64 cno;
    QString no;

    const auto model = &d->journalModel;
    const auto rows = model->rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = model->index(row, 0);
        if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accId) {
            const auto number = idx.data(eMyMoney::Model::JournalSplitNumberRole).toString();
            if (!number.isEmpty()) {
                // non-numerical values stored in number will return 0 in the next line
                cno = number.toULongLong();
                if (cno > lno) {
                    lno = cno;
                    no = number;
                }
            }
        }
    }
    return no;
}

bool MyMoneyFile::hasNewerTransaction(const QString& accId, const QDate& date) const
{
    const auto model = &d->journalModel;
    const auto rows = model->rowCount();
    // the journal is kept in chronological order, so we can search from
    // the end towards the front. Upon the first transaction we find for
    // the account we can provide an answer
    for (int row = rows - 1; row >= 0; --row) {
        const auto idx = model->index(row, 0);
        if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accId) {
            return idx.data(eMyMoney::Model::TransactionPostDateRole).toDate() > date;
        }
    }
    return false;
}

void MyMoneyFile::clearCache()
{
    d->m_balanceCache.clear();
}

void MyMoneyFile::forceDataChanged()
{
    Q_EMIT dataChanged();
}

bool MyMoneyFile::isTransfer(const MyMoneyTransaction& t) const
{
    auto rc = true;
    if (t.splitCount() == 2) {
        const auto splits = t.splits();
        for (const auto& split : splits) {
            auto acc = account(split.accountId());
            if (acc.isIncomeExpense()) {
                rc = false;
                break;
            }
        }
    }
    return rc;
}

bool MyMoneyFile::referencesClosedAccount(const MyMoneyTransaction& t) const
{
    auto ret = false;
    const auto splits = t.splits();
    for (const auto& split : splits) {
        if (referencesClosedAccount(split)) {
            ret = true;
            break;
        }
    }
    return ret;
}

bool MyMoneyFile::referencesClosedAccount(const MyMoneySplit& s) const
{
    if (s.accountId().isEmpty())
        return false;

    try {
        return account(s.accountId()).isClosed();
    } catch (const MyMoneyException &) {
    }
    return false;
}

QUuid MyMoneyFile::storageId()
{
    QUuid uid(value("kmm-id"));
    if (uid.isNull()) {
        MyMoneyFileTransaction ft;
        try {
            uid = QUuid::createUuid();
            setValue("kmm-id", uid.toString());
            ft.commit();
        } catch (const MyMoneyException &) {
            qDebug("Unable to setup UID for new storage object");
        }
    }
    return uid;
}

QString MyMoneyFile::openingBalancesPrefix()
{
    return i18n("Opening Balances");
}

bool MyMoneyFile::hasMatchingOnlineBalance(const MyMoneyAccount& _acc) const
{
    // get current values
    auto acc = account(_acc.id());

    // if there's no last transaction import data we are done
    if (acc.value("lastImportedTransactionDate").isEmpty()
            || acc.value("lastStatementBalance").isEmpty())
        return false;

    // otherwise, we compare the balances
    MyMoneyMoney balance(acc.value("lastStatementBalance"));
    MyMoneyMoney accBalance = this->balance(acc.id(), QDate::fromString(acc.value("lastImportedTransactionDate"), Qt::ISODate));

    return balance == accBalance;
}

int MyMoneyFile::countTransactionsWithSpecificReconciliationState(const QString& accId, TransactionFilter::State state) const
{
    int rc = 0;
    const auto model = &d->journalModel;
    const auto rows = model->rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = model->index(row, 0);
        if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accId) {
            if (state == TransactionFilter::State::All) {
                rc++;
            } else {
                const auto reconciliationState = idx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>();
                switch (reconciliationState) {
                case eMyMoney::Split::State::NotReconciled:
                    rc += (state == TransactionFilter::State::NotReconciled) ? 1 : 0;
                    break;
                case eMyMoney::Split::State::Cleared:
                    rc += (state == TransactionFilter::State::Cleared) ? 1 : 0;
                    break;
                case eMyMoney::Split::State::Reconciled:
                    rc += (state == TransactionFilter::State::Reconciled) ? 1 : 0;
                    break;
                case eMyMoney::Split::State::Frozen:
                    rc += (state == TransactionFilter::State::Frozen) ? 1 : 0;
                    break;
                default:
                    break;
                }
            }
        }
    }
    return rc;
}

QMap<QString, QVector<int> > MyMoneyFile::countTransactionsWithSpecificReconciliationState() const
{
    QMap<QString, QVector<int> > result;

    // fill with empty result for all existing accounts
    QList<MyMoneyAccount> list;
    accountList(list);
    for (const auto& acc : qAsConst(list)) {
        result[acc.id()] = QVector<int>((int)eMyMoney::Split::State::MaxReconcileState, 0);
    }

    const auto rows = d->journalModel.rowCount();
    QModelIndex idx;
    for (int row = 0; row < rows; ++row) {
        idx = d->journalModel.index(row, 0);
        const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        const auto flag = idx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>();
        switch (flag) {
        case eMyMoney::Split::State::NotReconciled:
        case eMyMoney::Split::State::Cleared:
        case eMyMoney::Split::State::Reconciled:
        case eMyMoney::Split::State::Frozen:
            result[accountId][(int)flag]++;
            break;
        default:
            break;
        }
    }
    return result;
}

/**
 * Make sure that the splits value has the precision of the corresponding account
 */
void MyMoneyFile::fixSplitPrecision(MyMoneyTransaction& t) const
{
    auto transactionSecurity = security(t.commodity());
    auto transactionFraction = transactionSecurity.smallestAccountFraction();

    for (auto& split : t.splits()) {
        auto acc = account(split.accountId());
        auto fraction = acc.fraction();
        if(fraction == -1) {
            auto sec = security(acc.currencyId());
            fraction = acc.fraction(sec);
        }
        // Don't do any rounding on a split factor
        if (split.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) {
            split.setShares(static_cast<const MyMoneyMoney>(split.shares().convertDenominator(fraction).canonicalize()));
            split.setValue(static_cast<const MyMoneyMoney>(split.value().convertDenominator(transactionFraction).canonicalize()));
        }
    }
}

void MyMoneyFile::reloadSpecialDates()
{
    d->specialDatesModel.load();
    // calculate the time until midnite
    const auto now = QDateTime::currentDateTime();
    auto nextDay = now.addDays(1);
    nextDay.setTime(QTime(0, 0));
    QTimer::singleShot(now.msecsTo(nextDay), this, SLOT(reloadSpecialDates()));
}


void MyMoneyFile::fileSaved()
{
    d->markModelsAsClean();
}

QUndoStack* MyMoneyFile::undoStack() const
{
    return &d->undoStack;
}

bool MyMoneyFile::hasValidId(const MyMoneyAccount& acc) const
{
    static const KMMSet<eMyMoney::Account::Standard> stdAccNames{
        eMyMoney::Account::Standard::Liability,
        eMyMoney::Account::Standard::Asset,
        eMyMoney::Account::Standard::Expense,
        eMyMoney::Account::Standard::Income,
        eMyMoney::Account::Standard::Equity,
    };
    const auto id = acc.id();
    for (const auto idx : qAsConst(stdAccNames)) {
        if (id == MyMoneyAccount::stdAccName(idx))
            return true;
    }
    return d->accountsModel.isValidId(id);
}

bool MyMoneyFile::hasValidId(const MyMoneyPayee& payee) const
{
    return d->payeesModel.isValidId(payee.id());
}

bool MyMoneyFile::applyFileFixes(bool expertMode)
{
    return d->applyFileFixes(expertMode);
}

class MyMoneyFileTransactionPrivate
{
    Q_DISABLE_COPY(MyMoneyFileTransactionPrivate)

public:
    MyMoneyFileTransactionPrivate() :
        m_isNested(MyMoneyFile::instance()->hasTransaction()),
        m_needRollback(!m_isNested)
    {
    }

    void startTransaction(const QString& undoActionText, bool journalBlocking)
    {
        if (!m_isNested)
            MyMoneyFile::instance()->startTransaction(undoActionText, journalBlocking);
    }

public:
    bool m_isNested;
    bool m_needRollback;

};

MyMoneyFileTransaction::MyMoneyFileTransaction()
    : d_ptr(new MyMoneyFileTransactionPrivate)
{
    Q_D(MyMoneyFileTransaction);
    d->startTransaction(QString(), true);
}

MyMoneyFileTransaction::MyMoneyFileTransaction(const QString& undoActionText, bool journalBlocking)
    : d_ptr(new MyMoneyFileTransactionPrivate)
{
    Q_D(MyMoneyFileTransaction);
    d->startTransaction(undoActionText, journalBlocking);
}

MyMoneyFileTransaction::~MyMoneyFileTransaction()
{
    try {
        rollback();
    } catch (const MyMoneyException &e) {
        qDebug() << e.what();
    }
    Q_D(MyMoneyFileTransaction);
    delete d;
}

void MyMoneyFileTransaction::restart()
{
    rollback();

    Q_D(MyMoneyFileTransaction);
    d->m_needRollback = !d->m_isNested;
    if (!d->m_isNested)
        MyMoneyFile::instance()->startTransaction();
}

void MyMoneyFileTransaction::commit()
{
    Q_D(MyMoneyFileTransaction);
    if (!d->m_isNested)
        MyMoneyFile::instance()->commitTransaction();
    d->m_needRollback = false;
}

void MyMoneyFileTransaction::rollback()
{
    Q_D(MyMoneyFileTransaction);
    if (d->m_needRollback)
        MyMoneyFile::instance()->rollbackTransaction();
    d->m_needRollback = false;
}

#include "mymoneyfile.moc"
