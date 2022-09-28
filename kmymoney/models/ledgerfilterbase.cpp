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

#include "accountsmodel.h"
#include "journalmodel.h"
#include "ledgerviewsettings.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "specialdatesmodel.h"

using namespace eMyMoney;

LedgerFilterBase::LedgerFilterBase(LedgerFilterBasePrivate* dd, QObject* parent)
    : LedgerSortProxyModel(dd, parent)
{
    Q_D(LedgerFilterBase);
    d->concatModel = new QConcatenateTablesProxyModel(parent);

    setSortRole(-1);
    setFilterRole(-1);
    setFilterKeyColumn(-1);
}

LedgerFilterBase::~LedgerFilterBase()
{
}

void LedgerFilterBase::setAccountType(Account::Type type)
{
    Q_D(LedgerFilterBase);
    d->accountType = type;
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

bool LedgerFilterBase::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const LedgerFilterBase);

    // if no filter is set, we don't display anything
    if (d->filterIds.isEmpty())
        return false;

    // special dates are always true
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    if (d->isSpecialDatesModel(idx)) {
        return true;
    }

    if (LedgerSortProxyModel::filterAcceptsRow(source_row, source_parent)) {
        const auto id = idx.data(filterRole()).toString();
        bool rc = d->filterIds.contains(id);

        // in case a journal entry has no id, it is the new transaction placeholder
        if (!rc) {
            rc = idx.data(eMyMoney::Model::IdRole).toString().isEmpty();
        }
        return rc;
    }
    return false;
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
