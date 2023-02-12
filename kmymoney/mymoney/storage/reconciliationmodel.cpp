/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reconciliationmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QFont>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "journalmodel.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"

struct ReconciliationModel::Private {
    Private(ReconciliationModel* qq)
        : q(qq)
        , showDateHeaders(false)
        , updateRequested(false)
    {
    }

    QString formatValue(const QString& accountId, const MyMoneyMoney& value)
    {
        auto acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
        auto factor = MyMoneyMoney::ONE;
        switch (acc.accountGroup()) {
        case eMyMoney::Account::Type::Liability:
        case eMyMoney::Account::Type::Income:
            factor = MyMoneyMoney::MINUS_ONE;
            break;
        default:
            break;
        }
        return (value * factor).formatMoney(acc.fraction());
    }

    ReconciliationModel* q;
    bool showDateHeaders;
    bool updateRequested;
};

ReconciliationModel::ReconciliationModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<ReconciliationEntry>(parent, QStringLiteral("RD"), ReconciliationModel::ID_SIZE, undoStack)
    , d(new Private(this))
{
    setObjectName(QLatin1String("ReconciliationModel"));
}

ReconciliationModel::~ReconciliationModel()
{
}

int ReconciliationModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return JournalModel::Column::MaxColumns;
}

Qt::ItemFlags ReconciliationModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::NoItemFlags;
}

void ReconciliationModel::load(const QMap<QString, ReconciliationEntry>& list)
{
    Q_UNUSED(list);
    qDebug() << Q_FUNC_INFO << "must never be called";
}

void ReconciliationModel::updateData()
{
    // register the update of the model for the
    // next event loop run so that the model is
    // updated only once even if load() is called
    // multiple times in a row. This is enough,
    // since the model is always loaded completely
    // (no partial updates)
    if (!d->updateRequested) {
        d->updateRequested = true;
        QMetaObject::invokeMethod(this, "doLoad", Qt::QueuedConnection);
    }
}

void ReconciliationModel::doLoad()
{
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    QList<MyMoneyAccount> accountList;
    MyMoneyFile::instance()->accountList(accountList);

    m_nextId = 0;
    for (auto& account : accountList) {
        const auto history = account.reconciliationHistory();
        if (!history.isEmpty()) {
            const auto& accountId = account.id();
            const auto rows = history.count();
            insertRows(0, rows);

            int row = 0;
            QMap<QDate, MyMoneyMoney>::const_iterator it;
            for (it = history.constBegin(); it != history.constEnd(); ++it) {
                ReconciliationEntry entry(nextId(),
                                          accountId,
                                          it.key(),
                                          *it,
                                          ((row + 1) < rows) ? eMyMoney::Model::StdFilter : eMyMoney::Model::DontFilterLast);
                static_cast<TreeItem<ReconciliationEntry>*>(index(row, 0).internalPointer())->dataRef() = entry;
                ++row;
            }
        }
        // in active reconciliation, the lastReconciledBalance is empty,
        // statementBalance and statementDate are not empty
        // see also
        bool inReconciliation =
            account.value("lastReconciledBalance").isEmpty() && !account.value("statementBalance").isEmpty() && !account.value("statementDate").isEmpty();
        if (inReconciliation) {
            ReconciliationEntry entry(nextId(),
                                      account.id(),
                                      QDate::fromString(account.value("statementDate"), Qt::ISODate),
                                      MyMoneyMoney(account.value("statementBalance")),
                                      eMyMoney::Model::DontFilter);
            insertRows(0, 1);
            static_cast<TreeItem<ReconciliationEntry>*>(index(0, 0).internalPointer())->dataRef() = entry;
        }
    }

    endResetModel();
    setDirty(false);
    d->updateRequested = false;
}

QVariant ReconciliationModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    // we report to have the same number of columns as the
    // journal model but we only react on the first column
    if (idx.column() < 0 || idx.column() >= JournalModel::Column::MaxColumns)
        return {};

    const ReconciliationEntry& reconciliationEntry = static_cast<TreeItem<ReconciliationEntry>*>(idx.internalPointer())->constDataRef();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (idx.column()) {
        case JournalModel::Column::EntryDate:
        case JournalModel::Column::Date:
            return reconciliationEntry.date();

        case JournalModel::Column::Balance:
            return d->formatValue(reconciliationEntry.accountId(), reconciliationEntry.amount());
        }
        break;

    case Qt::FontRole: {
        QFont font;
        font.setBold(true);
        return font;
    }

    case Qt::TextAlignmentRole:
        switch (idx.column()) {
        case JournalModel::Column::Quantity:
        case JournalModel::Column::Price:
        case JournalModel::Column::Amount:
        case JournalModel::Column::Payment:
        case JournalModel::Column::Deposit:
        case JournalModel::Column::Balance:
        case JournalModel::Column::Value:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);

        case JournalModel::Column::Reconciliation:
            return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);

        default:
            break;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::IdRole:
        return reconciliationEntry.id();

    case eMyMoney::Model::TransactionEntryDateRole:
    case eMyMoney::Model::TransactionPostDateRole:
    case eMyMoney::Model::SplitReconcileDateRole:
        return reconciliationEntry.date();

    case eMyMoney::Model::SplitAccountIdRole:
    case eMyMoney::Model::JournalSplitAccountIdRole:
        return reconciliationEntry.accountId();

    case eMyMoney::Model::ReconciliationAmountRole:
        return QVariant::fromValue(reconciliationEntry.amount());

    case eMyMoney::Model::ReconciliationBalanceRole:
        return d->formatValue(reconciliationEntry.accountId(), reconciliationEntry.amount());

    case eMyMoney::Model::SplitReconcileFlagRole:
        // the reconciliation entries should not be shown during reconciliation
        return QVariant::fromValue<eMyMoney::Split::State>(eMyMoney::Split::State::Reconciled);

    case eMyMoney::Model::DelegateRole:
        return static_cast<int>(eMyMoney::Delegates::Types::ReconciliationDelegate);

    case eMyMoney::Model::ReconciliationFilterHintRole:
        return QVariant::fromValue<eMyMoney::Model::ReconciliationFilterHint>(reconciliationEntry.filterHint());

    default:
        break;
    }
    return {};
}

void ReconciliationModel::setOptions(bool showDateHeaders)
{
    if (d->showDateHeaders != showDateHeaders) {
        d->showDateHeaders = showDateHeaders;
        updateData();
    }
}
