/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "journalmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QSize>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "costcentermodel.h"
#include "payeesmodel.h"
#include "securitiesmodel.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneymoney.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyutils.h"

struct JournalModel::Private
{
    typedef enum {
        Interest,
        Fees,
    } category_t;

    typedef enum {
        StockSplitForward,
        StockSplitBackward,
    } StockSplitDirection;

    Private(JournalModel* qq)
        : q(qq)
        , newTransactionModel(nullptr)
        , headerData(QHash<Column, QString>({
              {Number, i18nc("@title:column Cheque Number", "No.")},
              {Date, i18nc("@title:column Valuta date", "Date")},
              {Account, i18nc("@title:column", "Account")},
              {Payee, i18nc("@title:column", "Payee")},
              {Security, i18nc("@title:column", "Security")},
              {CostCenter, i18nc("@title:column Costcenter", "CC")},
              {Detail, i18nc("@title:column", "Detail")},
              {Reconciliation, i18nc("@title:column Reconciliation state", "C")},
              {Payment, i18nc("@title:column Payment made from account", "Payment")},
              {Deposit, i18nc("@title:column Deposit into account", "Deposit")},
              {Quantity, i18nc("@title:column", "Quantity")},
              {Price, i18nc("@title:column", "Price")},
              {Amount, i18nc("@title:column", "Amount")},
              {Value, i18nc("@title:column", "Value")},
              {Balance, i18nc("@title:column", "Balance")},
              {EntryDate, i18nc("@title:column Entry date", "Entry")},
          }))
        , extendedHeaderData(QHash<Column, QString>({
              {Number, i18nc("@title:column Cheque Number (ext)", "Number")},
              {Reconciliation, i18nc("@title:column Reconciliation state (ext)", "Reconciliation")},
              {CostCenter, i18nc("@title:column CostCenter (ext)", "Cost center")},
              {EntryDate, i18nc("@title:column Entry date (ext)", "Entry date")},
          }))
    {
    }

    QString reconciliationStateShort(eMyMoney::Split::State reconcileState) const
    {
        switch(reconcileState) {
        case eMyMoney::Split::State::NotReconciled:
        default:
            break;
        case eMyMoney::Split::State::Cleared:
            return i18nc("Reconciliation flag C", "C");
        case eMyMoney::Split::State::Reconciled:
            return i18nc("Reconciliation flag R", "R");
        case eMyMoney::Split::State::Frozen:
            return i18nc("Reconciliation flag F", "F");
        }
        return QString();
    }

    QString reconciliationStateLong(eMyMoney::Split::State reconcileState) const
    {
        switch(reconcileState) {
        case eMyMoney::Split::State::NotReconciled:
        default:
            return i18nc("Reconciliation flag empty", "Not reconciled");
        case eMyMoney::Split::State::Cleared:
            return i18nc("Reconciliation flag C", "Cleared");
        case eMyMoney::Split::State::Reconciled:
            return i18nc("Reconciliation flag R", "Reconciled");
        case eMyMoney::Split::State::Frozen:
            return i18nc("Reconciliation flag F", "Frozen");
        }
        return QString();
    }

    QString investmentActivity(const JournalEntry& journalEntry) const
    {
        switch (journalEntry.split().investmentTransactionType()) {
        case eMyMoney::Split::InvestmentTransactionType::AddShares:
            return i18nc("Add securities/shares/bonds", "Add shares");
        case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
            return i18nc("Remove securities/shares/bonds", "Remove shares");
        case eMyMoney::Split::InvestmentTransactionType::BuyShares:
            return i18nc("Buy securities/shares/bonds", "Buy shares");
        case eMyMoney::Split::InvestmentTransactionType::SellShares:
            return i18nc("Sell securities/shares/bonds", "Sell shares");
        case eMyMoney::Split::InvestmentTransactionType::Dividend:
            return i18n("Dividend");
        case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
            return i18n("Reinvest Dividend");
        case eMyMoney::Split::InvestmentTransactionType::Yield:
            return i18n("Yield");
        case eMyMoney::Split::InvestmentTransactionType::SplitShares:
            return i18nc("Split securities/shares/bonds", "Split shares");
        case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
            return i18n("Interest Income");
        default:
            return i18nc("Unknown investment activity", "Unknown");
        }
    }

    QString counterAccountId(const JournalEntry& journalEntry, const MyMoneyTransaction& transaction)
    {
        if (transaction.splitCount() == 2) {
            const auto& splitId = journalEntry.split().id();
            for (const auto& sp : transaction.splits()) {
                if(splitId != sp.id()) {
                    return sp.accountId();
                }
            }
        }
        return {};
    }

    QString counterAccount(const QModelIndex& index, const JournalEntry& journalEntry, const MyMoneyTransaction& transaction) const
    {
        // A transaction can have more than 2 splits ...
        const int rows = transaction.splitCount();
        if(rows > 2) {
            // find the first entry of the transaction
            QModelIndex idx = index;
            int row = index.row() - 1;
            for (; row >= 0; --row) {
                idx = q->index(row, 0);
                if (idx.data(eMyMoney::Model::JournalTransactionIdRole).toString() != transaction.id()) {
                    idx = q->index(row++, 0);
                    break;
                }
            }

            QString txt, sep;
            const auto endRow = row + rows;
            for (; row < endRow; ++row) {
                idx = q->index(row, index.column());
                if (idx != index) {
                    const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
                    idx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    txt += sep + idx.data(eMyMoney::Model::AccountNameRole).toString();
                    sep = QStringLiteral(", ");
                }
            }
            return txt;

            // ... exactly two splits ...
        } else if(rows == 2) {
            const auto& splitId = journalEntry.split().id();
            for (const auto& sp : transaction.splits()) {
                if(splitId != sp.id()) {
                    return MyMoneyFile::instance()->accountsModel()->accountIdToHierarchicalName(sp.accountId());
                }
            }

            // ... or a single split
        } else if(!journalEntry.split().shares().isZero()) {
            return i18n("*** UNASSIGNED ***");
        }
        return QString();
    }

    QString investmentBrokerageAccount(const JournalEntry& journalEntry) const
    {
        const auto file = MyMoneyFile::instance();
        if (file->isInvestmentTransaction(journalEntry.transaction())) {
            for (const auto& split : journalEntry.transaction().splits()) {
                const auto acc = file->account(split.accountId());
                if (acc.isAssetLiability() && !acc.isInvest() && (acc.accountType() != eMyMoney::Account::Type::Investment)) {
                    return acc.name();
                }
            }
        }
        return {};
    }

    QString interestOrFeeCategory(const JournalEntry& journalEntry, category_t type) const
    {
        const bool fees = (type == Fees);
        QString rc;
        const auto file = MyMoneyFile::instance();
        if (file->isInvestmentTransaction(journalEntry.transaction())) {
            for (const auto& split : journalEntry.transaction().splits()) {
                const auto acc = file->account(split.accountId());
                if (acc.isIncomeExpense()) {
                    if (split.shares().isNegative() ^ fees) {
                        if (rc.isEmpty()) {
                            rc = MyMoneyFile::instance()->accountsModel()->accountIdToHierarchicalName(split.accountId());
                        } else {
                            return fees ? i18n("Multiple fee categories") : i18n("Multiple interest categories");
                        }
                    }
                }
            }
        }
        return rc;
    }

    MyMoneyMoney interestOrFeeValue(const JournalEntry& journalEntry, category_t type) const
    {
        const bool fees = (type == Fees);
        MyMoneyMoney value;
        const auto file = MyMoneyFile::instance();
        if (file->isInvestmentTransaction(journalEntry.transaction())) {
            for (const auto& split : journalEntry.transaction().splits()) {
                const auto acc = file->account(split.accountId());
                if (acc.isIncomeExpense()) {
                    if (split.shares().isNegative() ^ fees) {
                        value += split.value();
                    }
                }
            }
        }
        return value;
    }

    bool haveInterestOrFeeSplit(const JournalEntry& journalEntry, category_t type) const
    {
        const auto file = MyMoneyFile::instance();
        if (file->isInvestmentTransaction(journalEntry.transaction())) {
            for (const auto& split : journalEntry.transaction().splits()) {
                const auto acc = file->account(split.accountId());
                if (acc.isIncomeExpense()) {
                    if (split.shares().isNegative() && (type == Interest)) {
                        return true;
                    }
                    else if (split.shares().isPositive() && (type == Fees)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    MyMoneySecurity security(const JournalEntry& journalEntry) const
    {
        const auto file = MyMoneyFile::instance();
        const auto acc = file->accountsModel()->itemById(journalEntry.split().accountId());
        return file->securitiesModel()->itemById(acc.currencyId());
    }

    void removeIdKeyMapping(const QString& id)
    {
        transactionIdKeyMap.remove(id);
    }

    void addIdKeyMapping(const QString& id, const QString& key)
    {
        transactionIdKeyMap[id] = key;
    }

    QString mapIdToKey(const QString& id) const
    {
        return transactionIdKeyMap.value(id);
    }

    void loadAccountCache()
    {
        accountCache.clear();
        const auto accountList = MyMoneyFile::instance()->accountsModel()->itemList();
        Q_FOREACH(const auto& acc, accountList) {
            accountCache[acc.id()] = acc;
        }
    }

    void startBalanceCacheOperation()
    {
        balanceChangedSet.clear();
        fullBalanceRecalc.clear();
    }

    void removeTransactionFromBalance(int startRow, int rows)
    {
        for (int row = 0; row < rows; ++row)  {
            const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(q->index(startRow, 0).internalPointer())->constDataRef();
            balanceChangedSet.insert(journalEntry.split().accountId());
            if (Q_UNLIKELY(journalEntry.transaction().isStockSplit())) {
                fullBalanceRecalc.insert(journalEntry.split().accountId());
            } else {
                balanceCache[journalEntry.split().accountId()] -= journalEntry.split();
            }
            ++startRow;
        }
    }

    void addTransactionToBalance(int startRow, int rows)
    {
        for (int row = 0; row < rows; ++row)  {
            const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(q->index(startRow, 0).internalPointer())->constDataRef();
            balanceChangedSet.insert(journalEntry.split().accountId());
            if (Q_UNLIKELY(journalEntry.transaction().isStockSplit())) {
                fullBalanceRecalc.insert(journalEntry.split().accountId());
            } else {
                balanceCache[journalEntry.split().accountId()] += journalEntry.split();
            }
            ++startRow;
        }
    }

    /**
     * Check if the balance of any account will change when exchanging
     * @a prev with @a next. Return @c true if this is the case,
     * @c false otherwise.
     *
     * @param curr const ref to current transaction in model
     * @param next const ref to replacing transaction in model
     *
     * @retval false no balance affected
     * @retval true balance of at least one account is affected
     */
    bool checkBalanceChange(const MyMoneyTransaction& curr, const MyMoneyTransaction& next) const
    {
        auto nextSplits = next.splits();
        const auto currSplits = curr.splits();
        for (const auto& currSplit : currSplits) {
            const int splitCount = nextSplits.count();
            if (splitCount == 0) {
                // the next version of the transaction has
                // more splits than the current one so there
                // must be a change of a balance
                return true;
            }
            for (int i = 0; i < splitCount; ++i) {
                const MyMoneySplit& nextSplit = nextSplits.at(i);
                if ((nextSplit.accountId() == currSplit.accountId()) && (nextSplit.value() == currSplit.value()) && nextSplit.shares() == currSplit.shares()) {
                    // we found exactly the same split
                    // in curr and next so we remove it
                    // from the list and continue with
                    // the next one to check
                    nextSplits.removeAt(i);
                    break;
                }
            }
        }

        // in case the next version has more splits
        // there must be a change of a balance
        return !nextSplits.isEmpty();
    }

    void finishBalanceCacheOperation()
    {
        if (!fullBalanceRecalc.isEmpty()) {
            const auto journalRows = q->rowCount();
            for (const auto& accountId : qAsConst(fullBalanceRecalc)) {
                balanceCache[accountId].clear();
            }

            for (int row = 0; row < journalRows; ++row) {
                const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(q->index(row, 0).internalPointer())->constDataRef();
                if (fullBalanceRecalc.contains(journalEntry.split().accountId())) {
                    if (journalEntry.transaction().isStockSplit()) {
                        const auto accountId = journalEntry.split().accountId();
                        balanceCache[accountId] = stockSplit(accountId, balanceCache[accountId], journalEntry.split().shares(), StockSplitForward);
                    } else {
                        balanceCache[journalEntry.split().accountId()] += journalEntry.split();
                    }
                }
            }
        }

        // inform others about the changes
        QHash<QString, AccountBalances> balances;
        for (const auto& accountId : qAsConst(balanceChangedSet)) {
            balances.insert(accountId, balanceCache.value(accountId));
            Q_EMIT q->balanceChanged(accountId);
        }
        if (!balances.isEmpty()) {
            Q_EMIT q->balancesChanged(balances);
        }
    }

    QString formatValue(const MyMoneyTransaction& t, const MyMoneySplit& s, const MyMoneyMoney& factor = MyMoneyMoney::ONE)
    {
        auto acc = MyMoneyFile::instance()->accountsModel()->itemById(s.accountId());
        auto value = s.value(t.commodity(), acc.currencyId());
        return (value * factor).formatMoney(acc.fraction());
    }

    QString formatShares(const MyMoneySplit& s)
    {
        auto acc = MyMoneyFile::instance()->accountsModel()->itemById(s.accountId());
        return (s.shares().abs()).formatMoney(acc.fraction());
    }

    MyMoneyMoney stockSplit(const QString& accountId, MyMoneyMoney balance, MyMoneyMoney factor, StockSplitDirection direction)
    {
        AccountBalances balances;
        balances.m_totalBalance = balance;
        balances.m_clearedBalance = balance;
        balances = stockSplit(accountId, balances, factor, direction);
        return balances.m_totalBalance;
    }

    AccountBalances stockSplit(const QString& accountId, AccountBalances balance, MyMoneyMoney factor, StockSplitDirection direction)
    {
        if (direction == StockSplitForward) {
            balance *= factor;
        } else {
            balance /= factor;
        }
        const auto account = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
        const auto security = MyMoneyFile::instance()->securitiesModel()->itemById(account.tradingCurrencyId());

        AlkValue::RoundingMethod roundingMethod = AlkValue::RoundRound;
        if (security.roundingMethod() != AlkValue::RoundNever)
            roundingMethod = security.roundingMethod();

        int securityFraction = security.smallestAccountFraction();

        balance.m_totalBalance = balance.m_totalBalance.convertDenominator(securityFraction, roundingMethod);
        balance.m_clearedBalance = balance.m_clearedBalance.convertDenominator(securityFraction, roundingMethod);
        return balance;
    }

    void resetRowHeightInformation(int firstRow, int lastRow)
    {
        for (int row = firstRow; row <= lastRow; ++row) {
            const auto idx = q->index(row, 0);
            q->setData(idx, 0, eMyMoney::Model::JournalSplitMaxLinesCountRole);
        }
    }

    MyMoneySplit matchedSplit(const MyMoneySplit& split)
    {
        try {
            const auto splitId = split.value(QLatin1String("kmm-match-split"));
            return split.matchedTransaction().splitById(splitId);
        } catch (...) {
        }
        return {};
    }

    JournalModel* q;
    JournalModelNewTransaction* newTransactionModel;
    QMap<QString, QString> transactionIdKeyMap;
    QHash<Column, QString> headerData;
    QHash<Column, QString> extendedHeaderData;
    QHash<QString, AccountBalances> balanceCache;
    QHash<QString, MyMoneyAccount> accountCache;
    QSet<QString> fullBalanceRecalc;
    QSet<QString> balanceChangedSet;
};

JournalModelNewTransaction::JournalModelNewTransaction(QObject* parent)
    : JournalModel(parent)
{
    setObjectName(QLatin1String("JournalModelNewTransaction"));
    QMap<QString, QSharedPointer<MyMoneyTransaction>> list;
    MyMoneyTransaction t;
    MyMoneySplit sp;
    sp.setAccountId(fakeId());
    t.addSplit(sp);
    list[QString()] = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(t));
    JournalModel::load(list);
}

JournalModelNewTransaction::~JournalModelNewTransaction()
{
}

QVariant JournalModelNewTransaction::data(const QModelIndex& idx, int role) const
{
    Q_UNUSED(idx)
    Q_UNUSED(role)
    if (role == eMyMoney::Model::DelegateRole) {
        return static_cast<int>(eMyMoney::Delegates::Types::JournalDelegate);
    }
    // never show any data for the empty transaction
    return {};
}





JournalModel::JournalModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<JournalEntry>(parent, QStringLiteral("T"), JournalModel::ID_SIZE, undoStack)
    , d(new Private(this))
{
    setObjectName(QLatin1String("JournalModel"));
    useIdToItemMapper(true);
}

JournalModel::JournalModel(const QString& idLeadin, QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<JournalEntry>(parent, idLeadin, JournalModel::ID_SIZE, undoStack)
    , d(new Private(this))
{
    setObjectName(QLatin1String("JournalModel"));
    useIdToItemMapper(true);
}

JournalModel::~JournalModel()
{
}

JournalModelNewTransaction * JournalModel::newTransaction()
{
    if (d->newTransactionModel == nullptr) {
        d->newTransactionModel = new JournalModelNewTransaction(this);
    }
    return d->newTransactionModel;
}

int JournalModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    Q_ASSERT(d->headerData.count() == MaxColumns);

    return MaxColumns;
}

QVariant JournalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
            return d->headerData.value(static_cast<Column>(section));

        case Qt::SizeHintRole:
            return QSize(20, 20);

        case eMyMoney::Model::LongDisplayRole:
            return d->extendedHeaderData.value(static_cast<Column>(section), d->headerData.value(static_cast<Column>(section)));
        }
    }
    if (orientation == Qt::Vertical && role == Qt::SizeHintRole) {
        return QSize(10, 10);
    }
    return MyMoneyModelBase::headerData(section, orientation, role);
}

Qt::ItemFlags JournalModel::flags(const QModelIndex& index) const
{
    if (index.isValid()) {
        return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    }
    return Qt::NoItemFlags;
}

QVariant JournalModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return QVariant();

    const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
    if (journalEntry.transactionPtr() == nullptr) {
        return QVariant();
    }

    const MyMoneyTransaction& transaction = journalEntry.transaction();
    const MyMoneySplit& split = journalEntry.split();

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(idx.column()) {
        case Number:
            return split.number();

        case Date:
            return MyMoneyUtils::formatDate(transaction.postDate());

        case Account:
            return MyMoneyFile::instance()->accountsModel()->itemById(split.accountId()).name();

        case Payee:
            return MyMoneyFile::instance()->payeesModel()->itemById(split.payeeId()).name();

        case Security:
            return d->security(journalEntry).name();

        case CostCenter:
#if 0
            /// @todo finish implementation
            // in case the own split does not have a costcenter, but the counter split does
            // we use it nevertheless
            if(m_costCenterId.isEmpty())
                m_costCenterId = split.costCenterId();
#endif
            break;

        case Detail:
            return d->counterAccount(idx, journalEntry, transaction);

        case Reconciliation:
            return d->reconciliationStateShort(split.reconcileFlag());
            break;

        case Payment:
            if (split.value().isNegative()) {
                return d->formatValue(transaction, split, MyMoneyMoney::MINUS_ONE);
            }
            break;

        case Deposit:
            if (!split.value().isNegative()) {
                return d->formatValue(transaction, split, MyMoneyMoney::ONE);
            }
            break;

        case Quantity:
            switch (split.investmentTransactionType()) {
            case eMyMoney::Split::InvestmentTransactionType::Dividend:
            case eMyMoney::Split::InvestmentTransactionType::Yield:
            case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
                break;
            case eMyMoney::Split::InvestmentTransactionType::SplitShares:
                return QString("1 / %1").arg(split.shares().abs().formatMoney(QString(), -1));
                break;
            default:
                return split.shares().abs().formatMoney(QString(), MyMoneyMoney::denomToPrec(d->security(journalEntry).smallestAccountFraction()));
            }
            break;

        case Price:
            switch (split.investmentTransactionType()) {
            case eMyMoney::Split::InvestmentTransactionType::BuyShares:
            case eMyMoney::Split::InvestmentTransactionType::SellShares:
            case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
                if (!split.shares().isZero()) {
                    return split.price().formatMoney(MyMoneyFile::instance()->currency(transaction.commodity()).tradingSymbol(), d->security(journalEntry).pricePrecision());

                }
                break;
            default:
                break;
            }
            break;

        case Amount:
        case Value:
        {
            MyMoneySplit assetAccountSplit;
            QList<MyMoneySplit> feeSplits, interestSplits;
            MyMoneySecurity security, currency;
            MyMoneyMoney amount;
            eMyMoney::Split::InvestmentTransactionType transactionType;
            MyMoneyUtils::dissectTransaction(transaction, split, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
            switch(transactionType) {
            case eMyMoney::Split::InvestmentTransactionType::BuyShares:
            case eMyMoney::Split::InvestmentTransactionType::SellShares:
            case eMyMoney::Split::InvestmentTransactionType::Dividend:
            case eMyMoney::Split::InvestmentTransactionType::Yield:
            case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
                return MyMoneyUtils::formatMoney(assetAccountSplit.value().abs(), currency);

            case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
                for(const auto& sp : qAsConst(interestSplits)) {
                    amount += sp.value();
                }
                return  MyMoneyUtils::formatMoney(-amount, currency);

            default:
                break;
            }
        }
        break;

        case Balance:
            return QLatin1String("n/a");

        case EntryDate:
            return MyMoneyUtils::formatDate(transaction.entryDate());
        }
        break;

    case Qt::TextAlignmentRole:
        switch( idx.column()) {
        case Quantity:
        case Price:
        case Amount:
        case Payment:
        case Deposit:
        case Balance:
        case Value:
            return QVariant(Qt::AlignRight | Qt::AlignTop);

        case Reconciliation:
            return QVariant(Qt::AlignHCenter | Qt::AlignTop);

        default:
            break;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignTop);

    case eMyMoney::Model::IdRole:
        return journalEntry.id();

    case eMyMoney::Model::SplitAccountIdRole:
        return split.accountId();

    case eMyMoney::Model::SplitReconcileFlagRole:
        return QVariant::fromValue<eMyMoney::Split::State>(split.reconcileFlag());

    case eMyMoney::Model::SplitReconcileDateRole:
        switch (split.reconcileFlag()) {
        case eMyMoney::Split::State::Reconciled:
        case eMyMoney::Split::State::Frozen:
            if (split.reconcileDate().isValid()) {
                return split.reconcileDate();
            }
            // in case a split is marked reconciled but
            // the reconciliation date is invalid
            // we return the postdate instead as the
            // closest approximation.
            return transaction.postDate();

        default:
            break;
        }
        return {};

    case eMyMoney::Model::SplitActionRole:
        return split.action();

    case eMyMoney::Model::JournalSplitIdRole:
        return split.id();

    case eMyMoney::Model::JournalSplitAccountIdRole:
        return split.accountId();

    case eMyMoney::Model::JournalTransactionIdRole:
        return journalEntry.transaction().id();

    case eMyMoney::Model::TransactionSplitCountRole:
        return journalEntry.transaction().splitCount();

    case eMyMoney::Model::TransactionValuableSplitCountRole:
        return journalEntry.transaction().splitCountWithValue();

    case eMyMoney::Model::TransactionSplitSumRole:
        return QVariant::fromValue(journalEntry.transaction().splitSum());

    case eMyMoney::Model::TransactionIsTransferRole:
        if (journalEntry.transaction().splitCount() == 2) {
            for (const auto& sp : journalEntry.transaction().splits()) {
                const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(sp.accountId());
                if (acc.isIncomeExpense()) {
                    return false;
                }
            }
        }
        return true;

    case eMyMoney::Model::TransactionIsInvestmentRole:
        return MyMoneyFile::instance()->isInvestmentTransaction(journalEntry.transaction());

    case eMyMoney::Model::TransactionIsImportedRole:
        return transaction.isImported();

    case eMyMoney::Model::TransactionInvestementType:
        return QVariant::fromValue<eMyMoney::Split::InvestmentTransactionType>(split.investmentTransactionType());

    case eMyMoney::Model::TransactionBrokerageAccountRole:
        return d->investmentBrokerageAccount(journalEntry);

    case eMyMoney::Model::TransactionInterestCategoryRole:
        return d->interestOrFeeCategory(journalEntry, Private::Interest);

    case eMyMoney::Model::TransactionFeesCategoryRole:
        return d->interestOrFeeCategory(journalEntry, Private::Fees);

    case eMyMoney::Model::TransactionPostDateRole:
        return transaction.postDate();

    case eMyMoney::Model::TransactionEntryDateRole:
        return transaction.entryDate();

    case eMyMoney::Model::TransactionIsStockSplitRole:
        return transaction.isStockSplit();

    case eMyMoney::Model::TransactionErroneousRole:
        return !transaction.splitSum().isZero();

    case eMyMoney::Model::TransactionInvestmentAccountIdRole:
        if (MyMoneyFile::instance()->isInvestmentTransaction(journalEntry.transaction())) {
            QString accountId;
            for (const auto& sp : journalEntry.transaction().splits()) {
                const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(sp.accountId());
                if (acc.accountType() == eMyMoney::Account::Type::Investment) {
                    return acc.id();
                }
                if (acc.isInvest()) {
                    accountId = acc.parentAccountId();
                }
            }
            return accountId;
        }
        break;

    case eMyMoney::Model::TransactionInterestValueRole:
        return QVariant::fromValue<MyMoneyMoney>(d->interestOrFeeValue(journalEntry, Private::Interest));

    case eMyMoney::Model::TransactionFeesValueRole:
        return QVariant::fromValue<MyMoneyMoney>(d->interestOrFeeValue(journalEntry, Private::Fees));

    case eMyMoney::Model::TransactionInterestSplitPresentRole:
        return d->haveInterestOrFeeSplit(journalEntry, Private::Interest);

    case eMyMoney::Model::TransactionFeeSplitPresentRole:
        return d->haveInterestOrFeeSplit(journalEntry, Private::Fees);

    case eMyMoney::Model::TransactionCommodityRole:
        return transaction.commodity();

    case eMyMoney::Model::SplitSharesSuffixRole:
        return QString("(%1)").arg(
            headerData(split.shares().isNegative() ? JournalModel::Column::Payment : JournalModel::Column::Deposit, Qt::Horizontal, Qt::DisplayRole)
                .toString());

    case eMyMoney::Model::SplitSharesRole:
    {
        QVariant rc;
        rc.setValue(split.shares());
        return rc;
    }

    case eMyMoney::Model::SplitFormattedValueRole:
        return d->formatValue(transaction, split, MyMoneyMoney::ONE);

    case eMyMoney::Model::SplitFormattedSharesRole:
        return d->formatShares(split);

    case eMyMoney::Model::SplitValueRole:
    {
        QVariant rc;
        rc.setValue(split.value());
        return rc;
    }

    case eMyMoney::Model::SplitPriceRole:
    {
        QVariant rc;
        rc.setValue(split.price());
        return rc;
    }

    case eMyMoney::Model::SplitPayeeIdRole:
        if (split.payeeId().isEmpty()) {
            // not sure if we want to replace it with the payeeId
            // of another split. Anyway, here would be the spot to do it
#if 0
            Q_FOREACH (const auto sp, transaction.splits()) {
                if(split.id() != sp.id()) {
                    if (!split.payeeId().isEmpty())

                    }
            }
#endif
            return QVariant();
        }
        return split.payeeId();

    case eMyMoney::Model::SplitTagIdRole:
        return QVariant::fromValue<QStringList>(split.tagIdList());

    case eMyMoney::Model::SplitSingleLineMemoRole:
    case eMyMoney::Model::SplitMemoRole:
    {
        QString rc(split.memo());
        if(role == eMyMoney::Model::SplitSingleLineMemoRole) {
            // remove empty lines
            rc.replace("\n\n", "\n");
            // replace '\n' with ", "
            rc.replace('\n', ", ");
        }
        return rc;
    }

    case eMyMoney::Model::MatchedSplitPayeeRole:
        if (split.isMatched()) {
            const auto matchedSplit = d->matchedSplit(split);
            return MyMoneyFile::instance()->payeesModel()->itemById(matchedSplit.payeeId()).name();
        }
        return {};

    case eMyMoney::Model::MatchedSplitMemoRole:
        if (split.isMatched()) {
            const auto matchedSplit = d->matchedSplit(split);
            auto rc(matchedSplit.memo());
            // remove empty lines
            rc.replace("\n\n", "\n");
            // replace '\n' with ", "
            rc.replace('\n', ", ");
            return rc;
        }
        return {};

    case eMyMoney::Model::SplitPayeeRole:
        return MyMoneyFile::instance()->payeesModel()->itemById(split.payeeId()).name();

    case eMyMoney::Model::TransactionCounterAccountRole:
        return d->counterAccount(idx, journalEntry, transaction);

    case eMyMoney::Model::TransactionCounterAccountIdRole:
        return d->counterAccountId(journalEntry, transaction);

    case eMyMoney::Model::TransactionScheduleRole:
        return false;

    case eMyMoney::Model::JournalSplitPaymentRole:
        if (split.value().isNegative()) {
            return d->formatValue(transaction, split, MyMoneyMoney::MINUS_ONE);
        }
        break;

    case eMyMoney::Model::JournalSplitDepositRole:
        if (!split.value().isNegative()) {
            return d->formatValue(transaction, split, MyMoneyMoney::ONE);
        }
        break;

    case eMyMoney::Model::JournalSplitIsMatchedRole:
        return split.isMatched();

    case eMyMoney::Model::SplitSharesFormattedRole:
        return d->formatShares(split);

    case eMyMoney::Model::SplitNumberRole:
    case eMyMoney::Model::JournalSplitNumberRole:
        return split.number();

    case eMyMoney::Model::SplitActivityRole:
        return d->investmentActivity(journalEntry);

    case eMyMoney::Model::JournalBalanceRole:
        return QVariant::fromValue(journalEntry.balance());

    case eMyMoney::Model::ScheduleIsOverdueRole:
        return false;

    case eMyMoney::Model::TransactionScheduleIdRole:
        // we don't store schedules in this model
        return {};

    case eMyMoney::Model::JournalSplitMaxLinesCountRole:
        return journalEntry.linesInLedger();

    case eMyMoney::Model::JournalSplitQuantitySortRole:
        switch (split.investmentTransactionType()) {
        case eMyMoney::Split::InvestmentTransactionType::Dividend:
        case eMyMoney::Split::InvestmentTransactionType::Yield:
        case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
            return QVariant::fromValue(MyMoneyMoney());
        case eMyMoney::Split::InvestmentTransactionType::SplitShares:
        default:
            return QVariant::fromValue(split.shares().abs());
        }
        break;

    case eMyMoney::Model::JournalSplitPriceSortRole:
        switch (split.investmentTransactionType()) {
        case eMyMoney::Split::InvestmentTransactionType::BuyShares:
        case eMyMoney::Split::InvestmentTransactionType::SellShares:
        case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
            if (!split.shares().isZero()) {
                return QVariant::fromValue(split.price());
            }
            return QVariant::fromValue(MyMoneyMoney());
        default:
            return QVariant::fromValue(MyMoneyMoney());
        }
        break;

    case eMyMoney::Model::JournalSplitValueSortRole: {
        MyMoneySplit assetAccountSplit;
        QList<MyMoneySplit> feeSplits, interestSplits;
        MyMoneySecurity security, currency;
        MyMoneyMoney amount;
        eMyMoney::Split::InvestmentTransactionType transactionType;
        MyMoneyUtils::dissectTransaction(transaction, split, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
        switch (transactionType) {
        case eMyMoney::Split::InvestmentTransactionType::BuyShares:
        case eMyMoney::Split::InvestmentTransactionType::SellShares:
        case eMyMoney::Split::InvestmentTransactionType::Dividend:
        case eMyMoney::Split::InvestmentTransactionType::Yield:
        case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
            return QVariant::fromValue(assetAccountSplit.value().abs());

        case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
            for (const auto& sp : qAsConst(interestSplits)) {
                amount += sp.value();
            }
            return QVariant::fromValue(-amount);

        default:
            break;
        }
        return QVariant::fromValue(MyMoneyMoney());
    }

    case eMyMoney::Model::JournalSplitSecurityNameRole:
        return d->security(journalEntry).name();

    case eMyMoney::Model::DelegateRole:
        return static_cast<int>(eMyMoney::Delegates::Types::JournalDelegate);

    case eMyMoney::Model::OnlineBalanceEntryRole:
    case eMyMoney::Model::SecurityAccountNameEntryRole:
        return false;

    default:
        if (role >= Qt::UserRole)
            qDebug() << "JournalModel::data(), role" << role << "offset" << role-Qt::UserRole << "not implemented";
        break;
    }
    return QVariant();
}

bool JournalModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if(!idx.isValid()) {
        return false;
    }
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent())) {
        return false;
    }

    JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->dataRef();
    if (journalEntry.transactionPtr() == nullptr) {
        return false;
    }

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(idx.column()) {
        case Balance:
            journalEntry.setBalance(value.value<MyMoneyMoney>());
            return true;

        default:
            return false;
        }
        break;

    case eMyMoney::Model::JournalSplitMaxLinesCountRole:
        journalEntry.setLinesInLedger(value.toInt());
        return true;

    default:
        return false;

    }
    // qDebug() << "setData(" << idx.row() << idx.column() << ")" << value << role;
    return QAbstractItemModel::setData(idx, value, role);
}

void JournalModel::load(const QMap<QString, QSharedPointer<MyMoneyTransaction>>& list)
{
    QElapsedTimer t;

    t.start();
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    // create the number of required items
    int itemCount = 0;
    for (const auto& item : qAsConst(list)) {
        itemCount += item->splitCount();
    }
    insertRows(0, itemCount);

    m_nextId = 0;

    int row = 0;
    QMap<QString, QSharedPointer<MyMoneyTransaction>>::const_iterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        const QString& id = (*it)->id();
        updateNextObjectId(id);
        d->addIdKeyMapping(id, it.key());
        // auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(*it));
        for (const auto& split : (*it)->splits()) {
            const JournalEntry journalEntry(QString("%1-%2").arg(it.key(), split.id()), *it, split);
            const auto newIdx = index(row, 0);
            static_cast<TreeItem<JournalEntry>*>(newIdx.internalPointer())->dataRef() = journalEntry;
            if (m_idToItemMapper) {
                m_idToItemMapper->insert(journalEntry.id(), static_cast<TreeItem<JournalEntry>*>(newIdx.internalPointer()));
            }
            ++row;
        }
    }
    endResetModel();

    Q_EMIT modelLoaded();

    // and don't count loading as a modification
    setDirty(false);

    qDebug() << "Model for" << m_idLeadin << "loaded with" << rowCount() << "items in" << t.elapsed() << "ms";
}

void JournalModel::unload()
{
    d->balanceCache.clear();
    d->accountCache.clear();
    d->transactionIdKeyMap.clear();
    MyMoneyModel::unload();
}

MyMoneyTransaction JournalModel::transactionById(const QString& id) const
{
    const QModelIndex idx = firstIndexById(id);
    if (idx.isValid()) {
        return static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef().transaction();
    }
    return {};
}

MyMoneyTransaction JournalModel::transactionByIndex(const QModelIndex& idx) const
{
    if (idx.isValid()) {
        return static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef().transaction();
    }
    return {};
}

QModelIndex JournalModel::firstIndexById(const QString& id) const
{
    const QString key = d->mapIdToKey(id);
    // in case we do not have a key, the transactionId does not exist
    // so no need to search for it
    if (key.isEmpty()) {
        return QModelIndex();
    }

    return MyMoneyModelBase::lowerBound(key);
}

QModelIndexList JournalModel::indexesByTransactionId(const QString& id) const
{
    QModelIndexList indexes;
    QModelIndex idx = firstIndexById(id);
    while (idx.isValid() && (idx.data(eMyMoney::Model::JournalTransactionIdRole).toString() == id)) {
        indexes.append(idx);
        idx = index(idx.row()+1, 0);
    }
    return indexes;
}

QString JournalModel::keyForDate(const QDate& date) const
{
    return MyMoneyTransaction::uniqueSortKey(date, QString());
}

void JournalModel::addTransaction(MyMoneyTransaction& item)
{
    item = MyMoneyTransaction(nextId(), item);
    auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(item));
    JournalEntry entry(QString(), transaction, MyMoneySplit());

    m_undoStack->push(new UndoCommand(this, JournalEntry(), entry));
}

void JournalModel::doAddItem(const JournalEntry& item, const QModelIndex& parentIdx)
{
    Q_UNUSED(parentIdx);
    auto transaction = item.sharedtransactionPtr();
    QString key = (*transaction).uniqueSortKey();

    // add mapping
    d->addIdKeyMapping((*transaction).id(), key);

    const auto idx = MyMoneyModelBase::lowerBound(key);
    auto startRow = idx.row();
    if (!idx.isValid()) {
        startRow = rowCount();
    }

    const int rows = (*transaction).splitCount();

    // insert the items into the model
    insertRows(startRow, rows);
    const QModelIndex startIdx = index(startRow, 0);
    const QModelIndex endIdx = index(startRow+rows-1, columnCount()-1);

    d->startBalanceCacheOperation();

    for (const auto& split : (*transaction).splits()) {
        const JournalEntry journalEntry(QString("%1-%2").arg(key, split.id()), transaction, split);
        const auto newIdx = index(startRow, 0);
        static_cast<TreeItem<JournalEntry>*>(newIdx.internalPointer())->dataRef() = journalEntry;
        if (m_idToItemMapper) {
            m_idToItemMapper->insert(journalEntry.id(), static_cast<TreeItem<JournalEntry>*>(newIdx.internalPointer()));
        }
        ++startRow;
    }

    d->resetRowHeightInformation(startIdx.row(), endIdx.row());

    // add the splits to the balance cache
    d->addTransactionToBalance(startIdx.row(), rows);

    doUpdateReferencedObjects();
    Q_EMIT dataChanged(startIdx, endIdx);

    d->finishBalanceCacheOperation();
    setDirty();
}

void JournalModel::removeTransaction(const MyMoneyTransaction& item)
{
    const auto idx = firstIndexById(item.id());
    if (idx.isValid()) {
        const auto currentItem = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
        m_undoStack->push(new UndoCommand(this, currentItem, JournalEntry()));
    }
}

void JournalModel::doRemoveItem(const JournalEntry& before)
{

    const auto& transaction = before.transaction();
    const auto idx = firstIndexById(transaction.id());
    const auto rows = transaction.splitCount();
    d->startBalanceCacheOperation();
    d->removeTransactionFromBalance(idx.row(), rows);

    // removeRows() also handles the m_idToItemMapper
    removeRows(idx.row(), rows);
    d->removeIdKeyMapping(transaction.id());

    d->finishBalanceCacheOperation();
    doUpdateReferencedObjects();
    setDirty();
}

void JournalModel::modifyTransaction(const MyMoneyTransaction& newTransaction)
{
    const auto idx = firstIndexById(newTransaction.id());
    if (idx.isValid()) {
        auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(newTransaction));
        JournalEntry entry(QString(), transaction, MyMoneySplit());

        const auto currentItem = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
        m_undoStack->push(new UndoCommand(this, currentItem, entry));
    }
}

void JournalModel::doModifyItem(const JournalEntry& before, const JournalEntry& after)
{
    Q_UNUSED(before)
    auto srcIdx = firstIndexById(after.transaction().id());

    if (!srcIdx.isValid())
        return;

    // we keep a copy of the original transaction
    // (we don't believe the caller except for the id)
    const auto newTransaction = after.transaction();
    const auto newSplitCount = static_cast<int>(newTransaction.splitCount());
    const auto newKey = newTransaction.uniqueSortKey();

    const auto oldTransaction = static_cast<TreeItem<JournalEntry>*>(srcIdx.internalPointer())->constDataRef().transaction();
    const auto oldSplitCount = static_cast<int>(oldTransaction.splitCount());
    const auto oldKey = oldTransaction.uniqueSortKey();

    const auto balanceChanges = d->checkBalanceChange(oldTransaction, newTransaction);

    d->startBalanceCacheOperation();
    if (balanceChanges) {
        d->removeTransactionFromBalance(srcIdx.row(), oldSplitCount);
    }

    // we have to deal with several cases here. The first differentiation
    // is the unique key. It remains the same as long as the postDate()
    // of the two transactions is identical. In case the postDate changed, we
    // need to move the transaction to a new spot in the model. This move
    // is done last so that we have the complete new data already in the model.
    // Besides the post date we have to differentiate between three other cases:
    //
    //   a) number of splits increases
    //   b) number of splits decreases
    //   c) number of splits remains the same
    //
    // In a first step we take care of cases a) and b) by inserting
    // or removing rows.
    // In the next step we simply assign new JournalEntry items to
    // the model for all splits.
    // And as last act we move the new items around

    // Step 1
    if (newSplitCount > oldSplitCount) {
        insertRows(srcIdx.row() + oldSplitCount, newSplitCount - oldSplitCount);

    } else if (newSplitCount < oldSplitCount) {
        removeRows(srcIdx.row() + newSplitCount, oldSplitCount - newSplitCount);
    }

    // Step 2
    auto transaction = after.sharedtransactionPtr();

    // use the oldKey for now to keep sorting in a correct state
    int row = srcIdx.row();
    for (const auto& split : newTransaction.splits()) {
        JournalEntry journalEntry(QString("%1-%2").arg(oldKey, split.id()), transaction, split);
        // force recalc of row height
        journalEntry.setLinesInLedger(0);
        const auto newIdx = index(row, 0);
        static_cast<TreeItem<JournalEntry>*>(newIdx.internalPointer())->dataRef() = journalEntry;
        if (m_idToItemMapper) {
            m_idToItemMapper->insert(journalEntry.id(), static_cast<TreeItem<JournalEntry>*>(newIdx.internalPointer()));
        }
        ++row;
    }

    // let the world know that things have changed
    QModelIndex endIdx = index(row-1, columnCount()-1);
    Q_EMIT dataChanged(srcIdx, endIdx);

    // Step 3
    if (newTransaction.postDate() != oldTransaction.postDate()) {
        const auto destIdx = MyMoneyModelBase::lowerBound(newKey);
        auto destRow = destIdx.row();
        if (!destIdx.isValid()) {
            destRow = rowCount();
        }
        // we can skip moving if there is no transaction between
        // the current location and the new one. The first part
        // (destRow != srcIdx.row()) checks for the same location
        // when new post date is earlier than old post date and
        // (destRow != (srcIdx.row() + newSplitCount)) checks
        // for the same location when new post date is later than
        // old post date.
        //
        // If we don't move we still have to update the id of
        // the journal entry and the m_idToItemMapper to use
        // the new date, though.
        if ((destRow != srcIdx.row()) && (destRow != (srcIdx.row() + newSplitCount))) {
            // Moving rows using moveRows() in a source model which is used by
            // a QConcatenateTablesProxyModel does not get propagated through
            // it which destructs the information.
            //
            // A workaround is to remove and insert the item(s) which is supported
            // by the QConcatenateTablesProxyModel.
            beginRemoveRows(QModelIndex(), srcIdx.row(), srcIdx.row() + newSplitCount - 1);

            d->removeIdKeyMapping(oldTransaction.id());

            // take the items out of their old location
            // and update the key (kept in their m_id)
            QVector<TreeItem<JournalEntry>*> entries;
            const int srcRow = srcIdx.row();
            for (int rows = newSplitCount; rows > 0; --rows) {
                auto journalEntry = m_rootItem->takeChild(srcRow);
                if (m_idToItemMapper) {
                    m_idToItemMapper->remove(journalEntry->dataRef().m_id);
                }
                journalEntry->dataRef().m_id = QString("%1-%2").arg(newKey, journalEntry->constDataRef().m_split.id());
                // force recalc of row height
                journalEntry->dataRef().m_linesInLedger = 0;
                entries.append(journalEntry);
            }

            endRemoveRows();

            // check if the destination row must be adjusted
            // since we removed the splits already
            if (srcIdx.row() < destRow)
                destRow -= newSplitCount;

            beginInsertRows(QModelIndex(), destRow, destRow + newSplitCount - 1);

            // insert the items at the new location
            m_rootItem->insertChildren(destRow, entries);

            // and possibly fill in the new m_idToItemMapper values
            if (m_idToItemMapper) {
                const int lastRow = destRow + entries.count() - 1;
                for (row = destRow; row <= lastRow; ++row) {
                    const auto idx = index(row, 0);
                    m_idToItemMapper->insert(idx.data(eMyMoney::Model::IdRole).toString(), static_cast<TreeItem<JournalEntry>*>(idx.internalPointer()));
                    // moveing the transaction in the ledger may have an impact on
                    // the total balance if the transaction is e.g. moved around a
                    // stock split. we simply force a recalc of balances in this
                    // account.
                    d->fullBalanceRecalc.insert(static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->dataRef().split().accountId());
                }
            }

            d->addIdKeyMapping(oldTransaction.id(), newKey);

            endInsertRows();

            // update the index of the transaction
            srcIdx = index(destRow, 0);

        } else {
            d->removeIdKeyMapping(oldTransaction.id());

            int srcRow = srcIdx.row();
            for (int rows = newSplitCount; rows > 0; --rows) {
                auto newIdx = index(srcRow, 0);
                auto journalEntry = static_cast<TreeItem<JournalEntry>*>(newIdx.internalPointer());
                if (m_idToItemMapper) {
                    m_idToItemMapper->remove(journalEntry->dataRef().m_id);
                }
                journalEntry->dataRef().m_id = QString("%1-%2").arg(newKey, journalEntry->constDataRef().m_split.id());
                // force recalc of row height
                journalEntry->dataRef().m_linesInLedger = 0;
                if (m_idToItemMapper) {
                    m_idToItemMapper->insert(journalEntry->dataRef().m_id, journalEntry);
                }
                ++srcRow;
            }

            d->addIdKeyMapping(oldTransaction.id(), newKey);
        }
    }

    if (balanceChanges) {
        d->addTransactionToBalance(srcIdx.row(), newTransaction.splitCount());
    }

    d->finishBalanceCacheOperation();
    doUpdateReferencedObjects();

    if (oldKey != newKey) {
        Q_EMIT idChanged(newKey, oldKey);
    }

    setDirty();
}

bool JournalModel::matchTransaction(const QModelIndex& idx, MyMoneyTransactionFilter& filter) const
{
    if (idx.row() < 0 || idx.row() > rowCount() - 1)
        return false;

    const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
    return filter.match(journalEntry.transaction());
}

void JournalModel::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
    list.clear();

    const int rows = rowCount();
    for (int row = 0; row < rows;) {
        const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
        const auto cnt = filter.matchingSplitsCount(journalEntry.transaction());
        for (uint i = 0; i < cnt; ++i) {
            list.append(journalEntry.transaction());
        }
        row += journalEntry.transaction().splitCount();
    }
}

void JournalModel::transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
    list.clear();

    const int rows = rowCount();
    QVector<MyMoneySplit> splits;
    for (int row = 0; row < rows; ) {
        const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
        splits = filter.matchingSplits(journalEntry.transaction());
        if (!splits.isEmpty()) {
            for (const auto& split : qAsConst(splits)) {
                list.append(qMakePair(journalEntry.transaction(), split));
            }
        }
        row += journalEntry.transaction().splitCount();
    }
}

unsigned int JournalModel::transactionCount(const QString& accountid) const
{
    unsigned int result = 0;

    if (accountid.isEmpty()) {
        result = d->transactionIdKeyMap.count();

    } else {
        const int rows = rowCount();
        for (int row = 0; row < rows; ++row) {
            const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
            if (journalEntry.split().accountId() == accountid) {
                ++result;
            }
        }
    }
    return result;
}

QString JournalModel::updateJournalId(const QString& journalId) const
{
    const QRegularExpression regExp(QStringLiteral("[\\d-]+(T\\d+)-(S\\d+)"));
    const auto expMatch = regExp.match(journalId);
    if (expMatch.hasMatch()) {
        const auto transactionId = expMatch.captured(1);
        const auto splitId = expMatch.captured(2);
        const auto indeces = indexesByTransactionId(transactionId);
        for (const auto& idx : indeces) {
            if (idx.data(eMyMoney::Model::JournalSplitIdRole).toString() == splitId) {
                qDebug() << "converted" << journalId << "to" << idx.data(eMyMoney::Model::IdRole).toString();
                return idx.data(eMyMoney::Model::IdRole).toString();
            }
        }
    }
    return {};
}

void JournalModel::updateBalances()
{
    d->loadAccountCache();

    // calculate the balances
    d->balanceCache.clear();
    const int rows = rowCount();
    qDebug() << "Start calculating balances:" << rows << "splits";
    for (int row = 0; row < rows; ++row) {
        const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
        if (journalEntry.transaction().isStockSplit()) {
            const auto accountId = journalEntry.split().accountId();
            d->balanceCache[accountId] = d->stockSplit(accountId, d->balanceCache[accountId], journalEntry.split().shares(), Private::StockSplitForward);
        } else {
            d->balanceCache[journalEntry.split().accountId()] += journalEntry.split();
        }
    }
    qDebug() << "End calculating balances";

    // and store the results in the accountsModel
    Q_EMIT balancesChanged(d->balanceCache);
}

MyMoneyMoney JournalModel::clearedBalance(const QString& accountId, const QDate& date) const
{
    MyMoneyMoney balance;
    const int rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
        const auto split(journalEntry.split());
        if (Q_UNLIKELY(!date.isValid()) || Q_LIKELY(journalEntry.transaction().postDate() <= date)) {
            if ((split.accountId() == accountId) && (split.reconcileFlag() != eMyMoney::Split::State::NotReconciled)) {
                if (journalEntry.transaction().isStockSplit()) {
                    const auto accId = journalEntry.split().accountId();
                    balance = d->stockSplit(accId, balance, journalEntry.split().shares(), Private::StockSplitForward);
                } else {
                    balance += journalEntry.split().shares();
                }
            }
        }
    }
    return balance;
}

MyMoneyMoney JournalModel::balance(const QString& accountId, const QDate& date) const
{
    if (date.isValid()) {
        MyMoneyMoney balance;
        QModelIndex lastIdx = upperBound(MyMoneyTransaction::uniqueSortKey(date, QStringLiteral("x")), 0, rowCount()-1);
        // in case the index is invalid, we search for a data past
        // the end of the journal, so we can simply use the cached
        // balance if it is available. Otherwise, we have to go
        // through the journal
        if (lastIdx.isValid() || !d->balanceCache.contains(accountId)) {
            // in case the entry is in the first half,
            // we start from the beginning and go forward
            auto lastRow = lastIdx.row();

            // if the lastRow is -1 we have an invalid index which means,
            // that the balance cache is not filled and we have to scan
            // all journal entries.
            if (lastRow == -1) {
                lastRow = rowCount();
            }

            // invalid index or first half?
            if (lastIdx.row() < rowCount()/2)  {
                for (int row = 0; row < lastRow; ++row) {
                    const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
                    if (journalEntry.split().accountId() == accountId) {
                        if (journalEntry.transaction().isStockSplit()) {
                            const auto accId = journalEntry.split().accountId();
                            balance = d->stockSplit(accId, balance, journalEntry.split().shares(), Private::StockSplitForward);
                        } else {
                            balance += journalEntry.split().shares();
                        }
                    }
                }
            } else {
                // in case the entry is in the second half,
                // we start at the end and go backwards
                // This requires the balance cache to always
                // be up-to-date
                balance = d->balanceCache.value(accountId).m_totalBalance;
                for (int row = rowCount()-1; row >= lastIdx.row(); --row) {
                    const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
                    if (journalEntry.split().accountId() == accountId) {
                        if (journalEntry.transaction().isStockSplit()) {
                            const auto accId = journalEntry.split().accountId();
                            balance = d->stockSplit(accId, balance, journalEntry.split().shares(), Private::StockSplitBackward);
                        } else {
                            balance -= journalEntry.split().shares();
                        }
                    }
                }
            }
            return balance;
        }
    }
    return d->balanceCache.value(accountId).m_totalBalance;
}

// determine for which type we were created:
//
// a) add item: m_after.id() is filled, m_before.id() is empty
// b) modify item: m_after.id() is filled, m_before.id() is filled
// c) add item: m_after.id() is empty, m_before.id() is filled
JournalModel::Operation JournalModel::undoOperation(const JournalEntry& before, const JournalEntry& after) const
{
    const auto afterIdEmpty = (after.transactionPtr() == nullptr) || (after.transaction().id().isEmpty());
    const auto beforeIdEmpty = (before.transactionPtr() == nullptr) || (before.transaction().id().isEmpty());
    if (!afterIdEmpty && beforeIdEmpty)
        return Add;
    if (!afterIdEmpty && !beforeIdEmpty)
        return Modify;
    if (afterIdEmpty && !beforeIdEmpty)
        return Remove;
    return Invalid;
}

QString JournalModel::fakeId() const
{
    return QStringLiteral("FakeID");
}

QModelIndex JournalModel::adjustToFirstSplitIdx(const QModelIndex& index) const
{
    if (!index.isValid() || index.model() != this) {
        return {};
    }

    const auto id = index.data(eMyMoney::Model::IdRole).toString();

    // find the first split of this transaction in the journal
    QModelIndex idx;
    int startRow;
    for (startRow = index.row()-1; startRow >= 0; --startRow) {
        idx = this->index(startRow, 0);
        const auto cid = idx.data(eMyMoney::Model::IdRole).toString();
        if (cid != id)
            break;
    }
    startRow++;

    return this->index(startRow, index.column());
}

JournalModel::DateRange JournalModel::dateRange() const
{
    DateRange result;
    const auto rows = rowCount();
    if (rows > 0) {
        const auto firstIdx = index(0, 0);
        const auto lastIdx = index(rows - 1, 0);
        result.firstTransaction = firstIdx.data(eMyMoney::Model::TransactionPostDateRole).toDate();
        result.lastTransaction = lastIdx.data(eMyMoney::Model::TransactionPostDateRole).toDate();
    }
    return result;
}

void JournalModel::resetRowHeightInformation()
{
    const auto lastRow = rowCount() - 1;
    d->resetRowHeightInformation(0, lastRow);

    const QModelIndex first = index(0, 0);
    const QModelIndex last = index(lastRow, columnCount() - 1);

    if (last.isValid()) {
        Q_EMIT dataChanged(first, last);
    }
}

MyMoneyMoney JournalModel::stockSplitBalance(const QString& accountId, MyMoneyMoney balance, MyMoneyMoney factor) const
{
    return d->stockSplit(accountId, balance, factor, Private::StockSplitForward);
}

bool JournalModel::hasReferenceTo(const QString& id) const
{
    bool rc = false;

    const int rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
        if ((rc |= journalEntry.hasReferenceTo(id)) == true) {
            break;
        }
    }
    return rc;
}
