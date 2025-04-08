/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItem>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneytransaction.h"
#include "payeesmodel.h"
#include "securitiesmodel.h"
#include "tagsmodel.h"

struct SplitModel::Private
{
    Private(SplitModel* qq)
        : q(qq)
        , headerData(QHash<Column, QString>({
              {Category, i18nc("Split header", "Category")},
              {Memo, i18nc("Split header", "Memo")},
              {Tags, i18nc("Split header", "Tags")},
              {Payment, i18nc("Split header", "Payment")},
              {Deposit, i18nc("Split header", "Deposit")},
          }))
        , currentSplitCount(-1)
        , showCurrencies(false)
    {
    }

    void copyFrom(const SplitModel& right)
    {
        // suppress emission of dataChanged signal
        QSignalBlocker blocker(q);
        q->unload();
        headerData = right.d->headerData;
        const auto rows = right.rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = right.index(row, 0);
            const auto split = right.itemByIndex(idx);
            q->appendSplit(split);
        }
        transactionCommodity = right.d->transactionCommodity;
        currentSplitCount = right.d->currentSplitCount;
        showCurrencies = right.d->showCurrencies;

        blocker.unblock();
        // send out a combined dataChanged signal
        QModelIndex start(q->index(0, 0));
        QModelIndex end(q->index(rows - 1, q->columnCount() - 1));
        Q_EMIT q->dataChanged(start, end);

        updateItemCount();
    }

    int splitCount() const
    {
        int count = 0;
        const auto rows = q->rowCount();
        for (auto row = 0; row < rows; ++row) {
            const auto idx = q->index(row, 0);
            if (!idx.data(eMyMoney::Model::SplitAccountIdRole).toString().isEmpty()) {
                ++count;
            }
        }
        return count;
    }

    void updateItemCount()
    {
        const auto count = splitCount();
        if (count != currentSplitCount) {
            currentSplitCount = count;
            Q_EMIT q->itemCountChanged(currentSplitCount);
        }
    }


    QString counterAccount() const
    {
        // A transaction can have more than 2 splits ...
        if(splitCount() > 1) {
            return i18n("Split transaction");

            // ... exactly two splits ...
        } else if(splitCount() == 1) {
            // we have to check which one is filled and which one
            // could be an empty (new) split
            const auto rows = q->rowCount();
            for (auto row = 0; row < rows; ++row) {
                const auto idx = q->index(row, 0);
                const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
                if (!accountId.isEmpty()) {
                    return MyMoneyFile::instance()->accountsModel()->accountIdToHierarchicalName(accountId);
                }
            }

            // ... or a single split
#if 0
        } else if(!idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().isZero()) {
            return i18n("*** UNASSIGNED ***");
#endif
        }
        return QString();
    }

    int splitCurrencyPrecision(const MyMoneySplit& split) const
    {
        const auto file = MyMoneyFile::instance();
        const auto account = file->accountsModel()->itemById(split.accountId());
        if (!account.id().isEmpty()) {
            try {
                const auto currency = file->currency(account.currencyId());
                if (Q_UNLIKELY(account.accountType() == eMyMoney::Account::Type::Cash)) {
                    return MyMoneyMoney::denomToPrec(currency.smallestCashFraction());
                }
                return MyMoneyMoney::denomToPrec(currency.smallestAccountFraction());
            } catch (MyMoneyException&) {
            }
        }
        return 2; // the default precision is 2 digits
    }

    QString splitCurrencySymbol(const MyMoneySplit& split) const
    {
        QString currencySymbol;
        if (showCurrencies) {
            const auto file = MyMoneyFile::instance();
            const auto accountIdx = file->accountsModel()->indexById(split.accountId());
            if (accountIdx.isValid()) {
                const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
                const auto securityIdx = file->currenciesModel()->indexById(currencyId);
                currencySymbol = securityIdx.data(eMyMoney::Model::SecuritySymbolRole).toString();
            }
        }
        return currencySymbol;
    }

    QString transactionCurrencySymbol() const
    {
        QString currencySymbol;
        if (showCurrencies) {
            const auto securityIdx = MyMoneyFile::instance()->currenciesModel()->indexById(transactionCommodity);
            currencySymbol = securityIdx.data(eMyMoney::Model::SecuritySymbolRole).toString();
        }
        return currencySymbol;
    }

    int transactionCurrencyPrecision() const
    {
        try {
            const auto currency = MyMoneyFile::instance()->currency(transactionCommodity);
            return MyMoneyMoney::denomToPrec(currency.smallestAccountFraction());
        } catch (MyMoneyException&) {
        }
        return 2; // the default precision is 2 digits
    }

    QString tags(const QStringList& tagIdList) const
    {
        const auto file = MyMoneyFile::instance();
        QStringList splitTagList = tagIdList;
        if (!splitTagList.isEmpty()) {
            std::transform(splitTagList.begin(), splitTagList.end(), splitTagList.begin(), [file](const QString& tagId) {
                return file->tagsModel()->itemById(tagId).name();
            });
            return splitTagList.join(", ");
        }
        return {};
    }

    QString displayAutoCalc(const MyMoneySplit& split, int column) const
    {
        const auto account = MyMoneyFile::instance()->accountsModel()->itemById(split.accountId());
        // isDepsit is an assumption in which column the value is likely to appear
        // based on the assigned account/category type
        const auto isDeposit = (account.accountGroup() == eMyMoney::Account::Type::Asset) || (account.accountGroup() == eMyMoney::Account::Type::Income);

        switch (column) {
        case Column::Payment:
            if (!isDeposit) {
                return i18nc("@info:placeholder amount widget", "calculated");
            }
            break;
        case Column::Deposit:
            if (isDeposit) {
                return i18nc("@info:placeholder amount widget", "calculated");
            }
            break;
        default:
            break;
        }
        return {};
    }

    QString displayValueAmount(const MyMoneySplit& split, int column) const
    {
        const auto value = split.value();
        switch (column) {
        case Column::Payment:
            if (!split.id().isEmpty()) {
                if (value.isAutoCalc()) {
                    return displayAutoCalc(split, column);
                }
                if (value.isPositive()) {
                    return value.formatMoney(transactionCurrencySymbol(), transactionCurrencyPrecision());
                }
            }
            break;

        case Column::Deposit:
            if (!split.id().isEmpty()) {
                if (value.isAutoCalc()) {
                    return displayAutoCalc(split, column);
                }
                if (value.isNegative() || value.isZero()) {
                    return (-value).formatMoney(transactionCurrencySymbol(), transactionCurrencyPrecision());
                }
            }
            break;

        default:
            break;
        }
        return {};
    }

    QString displaySharesAmount(const MyMoneySplit& split, int column) const
    {
        const auto value = split.shares();
        switch (column) {
        case Column::Payment:
            if (!split.id().isEmpty()) {
                if (value.isAutoCalc()) {
                    return displayAutoCalc(split, column);
                }
                if (value.isPositive()) {
                    return value.formatMoney(splitCurrencySymbol(split), splitCurrencyPrecision(split));
                }
            }
            break;

        case Column::Deposit:
            if (!split.id().isEmpty()) {
                if (value.isAutoCalc()) {
                    return displayAutoCalc(split, column);
                }
                if (value.isNegative() || value.isZero()) {
                    return (-value).formatMoney(splitCurrencySymbol(split), splitCurrencyPrecision(split));
                }
            }
            break;

        default:
            break;
        }
        return {};
    }

    MyMoneyMoney adjustAutoCalc(const MyMoneyMoney& value)
    {
        // in case the magic autoCalc value comes in with the wrong sign
        // we make sure to use the constant (which has a defined sign)
        if (value.isAutoCalc()) {
            return MyMoneyMoney::autoCalc;
        }
        return value;
    }

    void setupSplitData(MyMoneySplit& s, const QModelIndex& idx) const
    {
        s.setNumber(idx.data(eMyMoney::Model::SplitNumberRole).toString());
        s.setMemo(idx.data(eMyMoney::Model::SplitMemoRole).toString());
        s.setAccountId(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
        s.setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
        s.setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());
        s.setCostCenterId(idx.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
        s.setPayeeId(idx.data(eMyMoney::Model::SplitPayeeIdRole).toString());
        s.setTagIdList(idx.data(eMyMoney::Model::SplitTagIdRole).toStringList());

        // update the price information. setting the price to zero
        // will cause MyMoneySplit::price to recalculate the price
        // based on value and shares of the split
        s.setPrice(MyMoneyMoney());
        s.setPrice(s.possiblyCalculatedPrice());
    }

    SplitModel* q;
    QHash<Column, QString> headerData;
    QString transactionCommodity;
    int currentSplitCount;
    bool showCurrencies;
};

SplitModel::SplitModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<MyMoneySplit>(parent, QStringLiteral("S"), 4, undoStack)
    , d(new Private(this))
{
    // new splits in the split model start with 2 instead of 1
    // since the first split id is assigned by the transaction
    // editor when the transaction is created. (see
    // NewTransactionEditor::saveTransaction() )
    ++m_nextId;
    connect(this, &SplitModel::modelReset, this, [&] { d->updateItemCount(); });
    connect(this, &SplitModel::dataChanged, this, &SplitModel::checkForForeignCurrency);
}

SplitModel::SplitModel(QObject* parent, QUndoStack* undoStack, const SplitModel& right)
    : MyMoneyModel<MyMoneySplit>(parent, QStringLiteral("S"), 4, undoStack)
    , d(new Private(this))
{
    d->copyFrom(right);
    connect(this, &SplitModel::dataChanged, this, &SplitModel::checkForForeignCurrency);
}

SplitModel& SplitModel::operator=(const SplitModel& right)
{
    d->copyFrom(right);
    return *this;
}

SplitModel::~SplitModel()
{
}

int SplitModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    Q_ASSERT(d->headerData.count() == MaxColumns);
    return MaxColumns;
}

QString SplitModel::newSplitId()
{
    return QStringLiteral("New-ID");
}

bool SplitModel::isNewSplitId(const QString& id)
{
    return id.compare(newSplitId()) == 0;
}

QVariant SplitModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
        case eMyMoney::Model::LongDisplayRole:
            return d->headerData.value(static_cast<Column>(section));

        case Qt::SizeHintRole:
            return QSize(20, 20);
        }
        return {};
    }
    if (orientation == Qt::Vertical && role == Qt::SizeHintRole) {
        return QSize(10, 10);
    }

    return MyMoneyModelBase::headerData(section, orientation, role);
}

Qt::ItemFlags SplitModel::flags(const QModelIndex& index) const
{
    if (index.isValid()) {
        return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    }
    return Qt::NoItemFlags;
}

QVariant SplitModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return QVariant();

    const MyMoneySplit& split = static_cast<TreeItem<MyMoneySplit>*>(idx.internalPointer())->constDataRef();
    const auto file = MyMoneyFile::instance();
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(idx.column()) {
        case Column::Category:
            return file->accountsModel()->accountIdToHierarchicalName(split.accountId());

        case Column::Memo:
        {
            QString rc(split.memo());
            // remove empty lines
            rc.replace(QStringLiteral("\n\n"), QStringLiteral("\n"));
            // replace '\n' with ", "
            rc.replace(QStringLiteral("\n"), QStringLiteral(", "));
            return rc;
        }

        case Column::Payment:
        case Column::Deposit:
            return d->displayValueAmount(split, idx.column());

        case Tags:
            return d->tags(split.tagIdList());

        default:
            break;
        }
        break;

    case Qt::ToolTipRole:
        switch (idx.column()) {
        case Column::Payment:
        case Column::Deposit:
            if (d->showCurrencies) {
                return d->displaySharesAmount(split, idx.column());
            }
        }
        break;

    case Qt::TextAlignmentRole:
        switch (idx.column()) {
        case Payment:
        case Deposit:
            return QVariant(Qt::AlignRight | Qt::AlignTop);

        default:
            break;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignTop);

    case eMyMoney::Model::IdRole:
        return split.id();

    case eMyMoney::Model::SplitSingleLineMemoRole:
    case eMyMoney::Model::SplitMemoRole: {
        QString rc(split.memo());
        if (role == eMyMoney::Model::SplitSingleLineMemoRole) {
            // remove empty lines
            rc.replace(QStringLiteral("\n\n"), QStringLiteral("\n"));
            // replace '\n' with ", "
            rc.replace(QStringLiteral("\n"), QStringLiteral(", "));
        }
        return rc;
    }

    case eMyMoney::Model::SplitAccountIdRole:
        return split.accountId();

    case eMyMoney::Model::AccountTypeRole:
        return QVariant::fromValue<eMyMoney::Account::Type>(file->accountsModel()->itemById(split.accountId()).accountType());

    case eMyMoney::Model::AccountIsClosedRole:
        return file->accountsModel()->itemById(split.accountId()).isClosed();

    case eMyMoney::Model::AccountFullNameRole:
        return file->accountsModel()->accountIdToHierarchicalName(split.accountId());

    case eMyMoney::Model::SplitSharesRole:
        return QVariant::fromValue<MyMoneyMoney>(split.shares());

    case eMyMoney::Model::SplitValueRole:
        return QVariant::fromValue<MyMoneyMoney>(split.value());

    case eMyMoney::Model::SplitCostCenterIdRole:
        return split.costCenterId();

    case eMyMoney::Model::SplitNumberRole:
        return split.number();

    case eMyMoney::Model::SplitPayeeIdRole:
        return split.payeeId();

    case eMyMoney::Model::SplitPayeeRole:
        return file->payeesModel()->itemById(split.payeeId()).name();

    case eMyMoney::Model::SplitTagIdRole:
        return QVariant::fromValue<QStringList>(split.tagIdList());

    case eMyMoney::Model::TransactionCounterAccountRole:
        break;

    case eMyMoney::Model::SplitIsNewRole:
        return split.id().isEmpty() || split.id().endsWith(QLatin1Char('-'));

    case eMyMoney::Model::SplitActionRole:
        return split.action();

    case eMyMoney::Model::SplitIsAutoCalcRole:
        return split.shares().isAutoCalc();

    default:
        break;
    }
    return {};
}

bool SplitModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if(!idx.isValid()) {
        return false;
    }
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent())) {
        return false;
    }

    const auto startIdx = idx.model()->index(idx.row(), 0);
    const auto endIdx = idx.model()->index(idx.row(), idx.model()->columnCount()-1);
    MyMoneySplit& split = static_cast<TreeItem<MyMoneySplit>*>(idx.internalPointer())->dataRef();

    // in case we modify the data of a new split, we need to setup an id
    // this will be updated once we add the split to the transaction
    // we do this only when the category is set since this is a required
    // field
    if ((role == eMyMoney::Model::SplitAccountIdRole) && split.id().isEmpty()) {
        split = MyMoneySplit(newSplitId(), split);
    }

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        break;

    case eMyMoney::Model::SplitNumberRole:
        split.setNumber(value.toString());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitMemoRole:
        split.setMemo(value.toString());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitAccountIdRole:
        split.setAccountId(value.toString());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitCostCenterIdRole:
        split.setCostCenterId(value.toString());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitSharesRole:
        split.setShares(d->adjustAutoCalc(value.value<MyMoneyMoney>()));
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitValueRole:
        split.setValue(d->adjustAutoCalc(value.value<MyMoneyMoney>()));
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitPayeeIdRole:
        split.setPayeeId(value.toString());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitTagIdRole:
        split.setTagIdList(value.toStringList());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitBankIdRole:
        split.setBankID(value.toString());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    case eMyMoney::Model::SplitActionRole:
        split.setAction(value.toString());
        Q_EMIT dataChanged(startIdx, endIdx);
        return true;

    default:
        break;
    }
    return QAbstractItemModel::setData(idx, value, role);
}

void SplitModel::appendSplit(const MyMoneySplit& split)
{
    doAddItem(split);
}

void SplitModel::appendEmptySplit()
{
    const QModelIndexList list = match(index(0, 0), eMyMoney::Model::IdRole, QString(), -1, Qt::MatchExactly);
    if(list.isEmpty()) {
        doAddItem(MyMoneySplit());
    }
}

void SplitModel::removeEmptySplit()
{
    const QModelIndexList list = match(index(0, 0), eMyMoney::Model::IdRole, QString(), -1, Qt::MatchExactly);
    if(!list.isEmpty()) {
        removeRow(list.first().row(), list.first().parent());
    }
}

QModelIndex SplitModel::emptySplit() const
{
    const QModelIndexList list = match(index(0, 0), eMyMoney::Model::IdRole, QString(), -1, Qt::MatchExactly);
    if (!list.isEmpty()) {
        return list.first();
    }
    return {};
}

void SplitModel::doAddItem(const MyMoneySplit& item, const QModelIndex& parentIdx)
{
    MyMoneyModel::doAddItem(item, parentIdx);
    d->updateItemCount();
}

void SplitModel::doRemoveItem(const MyMoneySplit& before)
{
    MyMoneyModel::doRemoveItem(before);
    d->updateItemCount();
}

MyMoneyMoney SplitModel::valueSum() const
{
    MyMoneyMoney sum;
    const auto rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = index(row, 0);
        sum += idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
    }
    return sum;
}

void SplitModel::addSplitsToTransaction(MyMoneyTransaction& t) const
{
    // now update and add what we have in the model
    const auto rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = index(row, 0);
        MyMoneySplit s;
        const QString splitId = idx.data(eMyMoney::Model::IdRole).toString();
        // Extract the split from the transaction if
        // it already exists. Otherwise it remains
        // an empty split and will be added later.
        try {
            s = t.splitById(splitId);
        } catch (const MyMoneyException&) {
        }
        d->setupSplitData(s, idx);
        if (s.id().isEmpty()) {
            t.addSplit(s);
        } else {
            t.modifySplit(s);
        }
    }
}

QList<MyMoneySplit> SplitModel::splitList() const
{
    QList<MyMoneySplit> splits;
    const auto rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = index(row, 0);
        MyMoneySplit s;
        d->setupSplitData(s, idx);
        splits.append(s);
    }
    return splits;
}

void SplitModel::setTransactionCommodity(const QString& commodity)
{
    d->transactionCommodity = commodity;
    checkForForeignCurrency();
}

void SplitModel::checkForForeignCurrency() const
{
    d->showCurrencies = false;
    const auto file = MyMoneyFile::instance();
    const auto rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = index(row, 0);
        const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        if (!accountId.isEmpty()) {
            const auto accountIdx = file->accountsModel()->indexById(accountId);
            if (accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString() != d->transactionCommodity) {
                d->showCurrencies = true;
                break;
            }
        }
    }
}

bool SplitModel::hasMultiCurrencySplits() const
{
    return d->showCurrencies;
}

void SplitModel::resetAllSplitIds()
{
    const auto startIdx = index(0, 0);
    const auto endIdx = index(rowCount() - 1, columnCount() - 1);
    for (int row = 0; row <= endIdx.row(); ++row) {
        const auto idx = index(row, 0);
        MyMoneySplit& split = static_cast<TreeItem<MyMoneySplit>*>(idx.internalPointer())->dataRef();
        split = MyMoneySplit(newSplitId(), split);
    }
    Q_EMIT dataChanged(startIdx, endIdx);
}
