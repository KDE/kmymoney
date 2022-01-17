/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerfilterbase.h"
#include "ledgerfilterbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "journalmodel.h"
#include "accountsmodel.h"
#include "specialdatesmodel.h"

using namespace eMyMoney;

LedgerFilterBase::LedgerFilterBase(LedgerFilterBasePrivate* dd, QObject* parent)
    : QSortFilterProxyModel(parent)
    , d_ptr(dd)
{
    Q_D(LedgerFilterBase);
    d->concatModel = new KConcatenateRowsProxyModel(parent);

    setFilterRole(eMyMoney::Model::Roles::SplitAccountIdRole);
    setFilterKeyColumn(0);
    setSortRole(eMyMoney::Model::Roles::TransactionPostDateRole);
}

LedgerFilterBase::~LedgerFilterBase()
{
}

void LedgerFilterBase::setAccountType(Account::Type type)
{
    Q_D(LedgerFilterBase);
    d->accountType = type;
}

QVariant LedgerFilterBase::data(const QModelIndex& idx, int role) const
{
    if (role == eMyMoney::Model::SplitSharesSuffixRole) {
        const int columnRole = data(idx, eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().isNegative() ? JournalModel::Column::Payment : JournalModel::Column::Deposit;
        return QString("(%1)").arg(headerData(columnRole, Qt::Horizontal, Qt::DisplayRole).toString());
    }

    return QSortFilterProxyModel::data(idx, role);
}

QVariant LedgerFilterBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Q_D(const LedgerFilterBase);
        switch(section) {
        case JournalModel::Column::Payment:
            switch(d->accountType) {
            case Account::Type::CreditCard:
                return i18nc("Payment made with credit card", "Charge");

            case Account::Type::Asset:
            case Account::Type::AssetLoan:
                return i18nc("Decrease of asset/liability value", "Decrease");

            case Account::Type::Liability:
            case Account::Type::Loan:
                return i18nc("Increase of asset/liability value", "Increase");

            case Account::Type::Income:
            case Account::Type::Expense:
                return i18n("Income");

            default:
                break;
            }
            break;

        case JournalModel::Column::Deposit:
            switch(d->accountType) {
            case Account::Type::CreditCard:
                return i18nc("Payment towards credit card", "Payment");

            case Account::Type::Asset:
            case Account::Type::AssetLoan:
                return i18nc("Increase of asset/liability value", "Increase");

            case Account::Type::Liability:
            case Account::Type::Loan:
                return i18nc("Decrease of asset/liability value", "Decrease");

            case Account::Type::Income:
            case Account::Type::Expense:
                return i18n("Expense");

            default:
                break;
            }
            break;
        }
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

bool LedgerFilterBase::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const LedgerFilterBase);

    // make sure that the dummy transaction is shown last in any case
    if(left.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        return false;

    } else if(right.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        return true;
    }

    const auto model = MyMoneyFile::baseModel();
    // make sure that the online balance is the last entry of a day
    // and the date headers are the first
    if (left.data(eMyMoney::Model::TransactionPostDateRole).toDate() == right.data(eMyMoney::Model::TransactionPostDateRole).toDate()) {
        const auto leftModel = model->baseModel(left);
        const auto rightModel = model->baseModel(right);
        if (leftModel != rightModel) {
            // schedules will always be presented last on the same day
            // before that the online balance is shown
            // before that the reconciliation records are displayed
            // special date records are shown on top
            if (d->isSchedulesJournalModel(leftModel)) {
                return false;
            } else if (d->isSchedulesJournalModel(rightModel)) {
                return true;
            } else if (d->isAccountsModel(leftModel)) {
                return false;
            } else if (d->isAccountsModel(rightModel)) {
                return true;
            } else if (d->isSpecialDatesModel(leftModel)) {
                return true;
            } else if (d->isSpecialDatesModel(rightModel)) {
                return false;
            } else if (d->isReconciliationModel(leftModel)) {
                return false;
            } else if (d->isReconciliationModel(rightModel)) {
                return true;
            }
            // if we get here, both are transaction entries
        }
        // same model and same post date, the ids decide
        return left.data(eMyMoney::Model::IdRole).toString() < right.data(eMyMoney::Model::IdRole).toString();
    }

    // otherwise use normal sorting
    return QSortFilterProxyModel::lessThan(left, right);
}

bool LedgerFilterBase::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const LedgerFilterBase);

    // if no filter is set, we don't display anything
    if (d->filterIds.isEmpty())
        return false;

    // in case it's a special date entry, we accept it
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    const auto baseModel = MyMoneyFile::baseModel()->baseModel(idx);
    if (d->isSpecialDatesModel(baseModel)) {
        return (sortRole() == eMyMoney::Model::TransactionPostDateRole);
    }

    // now do the filtering
    const auto id = idx.data(filterRole()).toString();
    bool rc = d->filterIds.contains(id);

    // in case a journal entry has no id, it is the new transaction placeholder
    if(!rc) {
        rc = idx.data(eMyMoney::Model::IdRole).toString().isEmpty();
    }
    return rc;
}

void LedgerFilterBase::setFilterFixedString(const QString& id)
{
    setFilterFixedStrings(QStringList() << id);
}

void LedgerFilterBase::setFilterFixedStrings(const QStringList& filters)
{
    Q_D(LedgerFilterBase);
    d->filterIds = filters;
    invalidateFilter();
}

void LedgerFilterBase::appendFilterFixedString(const QString& filter)
{
    Q_D(LedgerFilterBase);
    if (!d->filterIds.contains(filter)) {
        d->filterIds.append(filter);
        invalidateFilter();
    }
}

QStringList LedgerFilterBase::filterFixedStrings() const
{
    Q_D(const LedgerFilterBase);
    return d->filterIds;
}


void LedgerFilterBase::setShowEntryForNewTransaction(bool show)
{
    if (show) {
        addSourceModel(MyMoneyFile::instance()->journalModel()->newTransaction());
    } else {
        removeSourceModel(MyMoneyFile::instance()->journalModel()->newTransaction());
    }
}


void LedgerFilterBase::addSourceModel(QAbstractItemModel* model)
{
    Q_D(LedgerFilterBase);
    if (model && !d->sourceModels.contains(model)) {
        d->concatModel->addSourceModel(model);
        d->sourceModels.insert(model);
        invalidateFilter();
    }
}

void LedgerFilterBase::removeSourceModel(QAbstractItemModel* model)
{
    Q_D(LedgerFilterBase);
    if (model && d->sourceModels.contains(model)) {
        d->concatModel->removeSourceModel(model);
        d->sourceModels.remove(model);
        invalidateFilter();
    }
}
