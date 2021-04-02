/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "models.h"
#include "costcentermodel.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneypayee.h"
#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "modelenums.h"

using namespace eLedgerModel;
using namespace eMyMoney;

class SplitModelPrivate
{
public:
    SplitModelPrivate()
        : m_invertValues(false)
    {}

    bool isCreateSplitEntry(const QString& id) const {
        return id.isEmpty();
    }

    MyMoneyTransaction    m_transaction;
    QVector<MyMoneySplit> m_splits;
    bool                  m_invertValues;
};

SplitModel::SplitModel(QObject* parent) :
    QAbstractTableModel(parent),
    d_ptr(new SplitModelPrivate)
{
}

SplitModel::~SplitModel()
{
}

QString SplitModel::newSplitId()
{
    return QLatin1String("New-ID");
}

bool SplitModel::isNewSplitId(const QString& id)
{
    return id.compare(newSplitId()) == 0;
}


int SplitModel::rowCount(const QModelIndex& parent) const
{
    Q_D(const SplitModel);
    Q_UNUSED(parent);
    return d->m_splits.count();
}

int SplitModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return (int)Column::LastColumn;
}


void SplitModel::deepCopy(const SplitModel& right, bool revertSplitSign)
{
    Q_D(SplitModel);
    beginInsertRows(QModelIndex(), 0, right.rowCount());
    d->m_splits = right.d_func()->m_splits;
    d->m_transaction = right.d_func()->m_transaction;
    if(revertSplitSign) {
        for(int idx = 0; idx < d->m_splits.count(); ++idx) {
            MyMoneySplit& split = d->m_splits[idx];
            split.setShares(-split.shares());
            split.setValue(-split.value());
        }
    }
    endInsertRows();
}

QVariant SplitModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case (int)Column::CostCenter:
            return i18n("Cost Center");
        case (int)Column::Detail:
            return i18n("Category");
        case (int)Column::Number:
            return i18n("No");
        case (int)Column::Date:
            return i18n("Date");
        case (int)Column::Security:
            return i18n("Security");
        case (int)Column::Reconciliation:
            return i18n("C");
        case (int)Column::Payment:
            return i18n("Payment");
        case (int)Column::Deposit:
            return i18n("Deposit");
        case (int)Column::Quantity:
            return i18n("Quantity");
        case (int)Column::Price:
            return i18n("Price");
        case (int)Column::Amount:
            return i18n("Amount");
        case (int)Column::Value:
            return i18n("Value");
        case (int)Column::Balance:
            return i18n("Balance");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant SplitModel::data(const QModelIndex& index, int role) const
{
    Q_D(const SplitModel);
    if(!index.isValid())
        return QVariant();
    if(index.row() < 0 || index.row() >= d->m_splits.count())
        return QVariant();

    QVariant rc;
    MyMoneyAccount acc;
    MyMoneyMoney value;
    const MyMoneySplit& split = d->m_splits[index.row()];
    QModelIndex subIndex;
    CostCenterModel* ccModel = Models::instance()->costCenterModel();

    switch(role) {
    case Qt::DisplayRole:
        // make sure to never return any displayable text for the dummy entry
        if(!d->isCreateSplitEntry(split.id())) {
            switch(index.column()) {
            case (int)Column::Detail:
                rc = MyMoneyFile::instance()->accountToCategory(split.accountId());
                break;
            case (int)Column::CostCenter:
                subIndex = Models::indexById(ccModel, CostCenterModel::CostCenterIdRole, split.costCenterId());
                rc = ccModel->data(subIndex);
                break;
            case (int)Column::Number:
                rc = split.number();
                break;
            case (int)Column::Reconciliation:
                rc = KMyMoneyUtils::reconcileStateToString(split.reconcileFlag(), false);
                break;
            case (int)Column::Payment:
                if(split.value().isNegative()) {
                    acc = MyMoneyFile::instance()->account(split.accountId());
                    rc = (-split).value(d->m_transaction.commodity(), acc.currencyId()).formatMoney(acc.fraction());
                }
                break;
            case (int)Column::Deposit:
                if(!split.value().isNegative()) {
                    acc = MyMoneyFile::instance()->account(split.accountId());
                    rc = split.value(d->m_transaction.commodity(), acc.currencyId()).formatMoney(acc.fraction());
                }
                break;
            default:
                break;
            }
        }
        break;

    case Qt::TextAlignmentRole:
        switch(index.column()) {
        case (int)Column::Payment:
        case (int)Column::Deposit:
        case (int)Column::Amount:
        case (int)Column::Balance:
        case (int)Column::Value:
            rc = QVariant(Qt::AlignRight| Qt::AlignTop);
            break;
        case (int)Column::Reconciliation:
            rc = QVariant(Qt::AlignHCenter | Qt::AlignTop);
            break;
        default:
            rc = QVariant(Qt::AlignLeft | Qt::AlignTop);
            break;
        }
        break;

    case (int)Role::AccountId:
        rc = split.accountId();
        break;

    case (int)Role::Account:
        rc = MyMoneyFile::instance()->accountToCategory(split.accountId());
        break;

    case (int)Role::TransactionId:
        rc = QString("%1").arg(d->m_transaction.id());
        break;

    case (int)Role::TransactionSplitId:
        rc = QString("%1-%2").arg(d->m_transaction.id(), split.id());
        break;

    case (int)Role::SplitId:
        rc = split.id();
        break;

    case (int)Role::Memo:
    case (int)Role::SingleLineMemo:
        rc = split.memo();
        if(role == (int)Role::SingleLineMemo) {
            QString txt = rc.toString();
            // remove empty lines
            txt.replace("\n\n", "\n");
            // replace '\n' with ", "
            txt.replace('\n', ", ");
            rc = txt;
        }
        break;

    case (int)Role::SplitShares:
        rc = QVariant::fromValue<MyMoneyMoney>(split.shares());
        break;

    case (int)Role::SplitValue:
        acc = MyMoneyFile::instance()->account(split.accountId());
        rc = QVariant::fromValue<MyMoneyMoney>(split.value(d->m_transaction.commodity(), acc.currencyId()));
        break;

    case (int)Role::PayeeName:
        try {
            rc = MyMoneyFile::instance()->payee(split.payeeId()).name();
        } catch (const MyMoneyException &e) {
        }
        break;

    case (int)Role::CostCenterId:
        rc = split.costCenterId();
        break;

    case (int)Role::TransactionCommodity:
        return d->m_transaction.commodity();
        break;

    case (int)Role::Number:
        rc = split.number();
        break;

    case (int)Role::PayeeId:
        rc = split.payeeId();
        break;

    default:
        if(role >= Qt::UserRole) {
            qWarning() << "Undefined role" << role << "(" << role-Qt::UserRole << ") in SplitModel::data";
        }
        break;
    }
    return rc;
}

bool SplitModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_D(SplitModel);
    bool rc = false;
    if(index.isValid()) {
        MyMoneySplit& split = d->m_splits[index.row()];
        if(split.id().isEmpty()) {
            split = MyMoneySplit(newSplitId(), split);
        }
        QString val;
        rc = true;
        switch(role) {
        case (int)Role::PayeeId:
            split.setPayeeId(value.toString());
            break;

        case (int)Role::AccountId:
            split.setAccountId(value.toString());
            break;

        case (int)Role::Memo:
            split.setMemo(value.toString());
            break;

        case (int)Role::CostCenterId:
            val = value.toString();
            split.setCostCenterId(value.toString());
            break;

        case (int)Role::Number:
            split.setNumber(value.toString());
            break;

        case (int)Role::SplitShares:
            split.setShares(value.value<MyMoneyMoney>());
            break;

        case (int)Role::SplitValue:
            split.setValue(value.value<MyMoneyMoney>());
            break;

        case (int)Role::EmitDataChanged:
        {
            // the whole row changed
            QModelIndex topLeft = this->index(index.row(), 0);
            QModelIndex bottomRight = this->index(index.row(), this->columnCount()-1);
            emit dataChanged(topLeft, bottomRight);
        }
        break;

        default:
            rc = false;
            break;
        }
    }

    return rc;
}


void SplitModel::addSplit(const QString& transactionSplitId)
{
    Q_D(SplitModel);
    QRegExp transactionSplitIdExp("^(\\w+)-(\\w+)$");
    if(transactionSplitIdExp.exactMatch(transactionSplitId)) {
        const QString transactionId = transactionSplitIdExp.cap(1);
        const QString splitId = transactionSplitIdExp.cap(2);
        if(transactionId != d->m_transaction.id()) {
            try {
                d->m_transaction = MyMoneyFile::instance()->transaction(transactionId);
            } catch (const MyMoneyException &e) {
                d->m_transaction = MyMoneyTransaction();
            }
        }
        try {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            d->m_splits.append(d->m_transaction.splitById(splitId));
            endInsertRows();
        } catch (const MyMoneyException &e) {
            d->m_transaction = MyMoneyTransaction();
        }
    }
}

void SplitModel::addEmptySplitEntry()
{
    Q_D(SplitModel);
    QModelIndexList list = match(index(0, 0), (int)Role::SplitId, QString(), -1, Qt::MatchExactly);
    if(list.count() == 0) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        // d->m_splits.append(MyMoneySplit(d->newSplitEntryId(), MyMoneySplit()));
        d->m_splits.append(MyMoneySplit());
        endInsertRows();
    }
}

void SplitModel::removeEmptySplitEntry()
{
    Q_D(SplitModel);
    // QModelIndexList list = match(index(0, 0), SplitIdRole, d->newSplitEntryId(), -1, Qt::MatchExactly);
    QModelIndexList list = match(index(0, 0), (int)Role::SplitId, QString(), -1, Qt::MatchExactly);
    if(list.count()) {
        QModelIndex index = list.at(0);
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        d->m_splits.remove(index.row(), 1);
        endRemoveRows();
    }
}

bool SplitModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_D(SplitModel);
    bool rc = false;
    if(count > 0) {
        beginRemoveRows(parent, row, row + count - 1);
        d->m_splits.remove(row, count);
        endRemoveRows();
        rc = true;
    }
    return rc;
}

Qt::ItemFlags SplitModel::flags(const QModelIndex& index) const
{
    Q_D(const SplitModel);
    Qt::ItemFlags flags;

    if(!index.isValid())
        return flags;
    if(index.row() < 0 || index.row() >= d->m_splits.count())
        return flags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}


#if 0
void SplitModel::removeSplit(const LedgerTransaction& t)
{
    Q_D(SplitModel);
    QModelIndexList list = match(index(0, 0), TransactionSplitIdRole, t.transactionSplitId(), -1, Qt::MatchExactly);
    if(list.count()) {
        QModelIndex index = list.at(0);
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        delete d->m_ledgerItems[index.row()];
        d->m_ledgerItems.remove(index.row(), 1);
        endRemoveRows();

        // just make sure we're in sync
        Q_ASSERT(d->m_ledgerItems.count() == rowCount());
    }
}
#endif
