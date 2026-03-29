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
    const auto file = MyMoneyFile::instance();
    const auto journalModel = file->journalModel();

    file->accountList(accountList);

    m_nextId = 0;
    const int maxJournalRows = file->journalModel()->rowCount();

    for (auto& account : accountList) {
        const auto history = account.reconciliationHistory();
        if (!history.isEmpty()) {
            const auto& accountId = account.id();
            const auto rows = history.count();
            insertRows(0, rows);

            int reconciliationRow = 0;
            int nextReconciliationStartRow = 0;
            MyMoneyMoney nextReconciliationStartAmount;

            QMap<QDate, MyMoneyMoney>::const_iterator it;
            for (it = history.cbegin(); it != history.cend(); ++it) {
                MyMoneyMoney reconciledBalance = nextReconciliationStartAmount;
                const auto reconciliationReferenceDate = it.key();
                auto journalRow = nextReconciliationStartRow;
                nextReconciliationStartRow = -1;

                for (; journalRow < maxJournalRows; ++journalRow) {
                    const auto idx = journalModel->index(journalRow, 0);
                    const auto postDate = idx.data(eMyMoney::Model::TransactionPostDateRole).toDate();

                    // the scan went beyond the reconciliation records date
                    // means the loop can be quit this time
                    if (postDate > reconciliationReferenceDate) {
                        if (nextReconciliationStartRow == -1) {
                            nextReconciliationStartRow = journalRow;
                            nextReconciliationStartAmount = reconciledBalance;
                        }
                        break;
                    }

                    if (idx.data(eMyMoney::Model::SplitAccountIdRole).toString() == accountId) {
                        const auto reconciliationDate = idx.data(eMyMoney::Model::SplitReconcileDateRole).toDate();
                        if (reconciliationDate.isValid()) {
                            if (reconciliationDate > reconciliationReferenceDate && (nextReconciliationStartRow == -1)) {
                                // the reconciliation for this split happened
                                // during a later reconciliation process. Keep
                                // that row so that we can start here again if
                                // it is the first of such transactions for this
                                // reconciliation period.
                                nextReconciliationStartRow = journalRow;
                                nextReconciliationStartAmount = reconciledBalance;
                            } else if (reconciliationDate <= reconciliationReferenceDate) {
                                // this is a transaction that was reconciled in the current period
                                const auto state = idx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>();
                                if ((state == eMyMoney::Split::State::Reconciled) || (state == eMyMoney::Split::State::Frozen)) {
                                    reconciledBalance += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                                }
                            }
                        }
                    }
                }

                ReconciliationEntry entry(nextId(),
                                          accountId,
                                          reconciliationReferenceDate,
                                          *it,
                                          ((reconciliationRow + 1) < rows) ? eMyMoney::Model::StdFilter : eMyMoney::Model::DontFilterLast);
                entry.setBackgroundColorRole((reconciledBalance == *it) ? KColorScheme::PositiveBackground : KColorScheme::NegativeBackground);
                auto nextIt = it;
                entry.setLastReconciliation(++nextIt == history.cend());

                static_cast<TreeItem<ReconciliationEntry>*>(index(reconciliationRow, 0).internalPointer())->dataRef() = entry;
                ++reconciliationRow;
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
                                      eMyMoney::Model::DontFilter,
                                      true);
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
    if (role == eMyMoney::Model::BaseModelRole) {
        return eMyMoney::Model::ReconciliationEntryRole;
    }

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

    case eMyMoney::Model::ReconciliationBackgroundRole:
        return reconciliationEntry.backgroundColorRole();

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

    case eMyMoney::Model::ReconciliationCurrentRole:
        return reconciliationEntry.isReconciliationInProgress();

    case eMyMoney::Model::LastReconciliationRole:
        return reconciliationEntry.isLastReconciliation();

    default:
        break;
    }
    return {};
}

bool ReconciliationModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid())
        return false;

    // we report to have the same number of columns as the
    // journal model but we only react on the first column
    if (idx.column() < 0 || idx.column() >= JournalModel::Column::MaxColumns)
        return false;

    ReconciliationEntry& reconciliationEntry = static_cast<TreeItem<ReconciliationEntry>*>(idx.internalPointer())->dataRef();

    switch (role) {
    case eMyMoney::Model::ReconciliationBackgroundRole:
        reconciliationEntry.setBackgroundColorRole(static_cast<KColorScheme::BackgroundRole>(value.toInt()));
        return true;

    default:
        break;
    }
    return false;
}

void ReconciliationModel::setOptions(bool showDateHeaders)
{
    if (d->showDateHeaders != showDateHeaders) {
        d->showDateHeaders = showDateHeaders;
        updateData();
    }
}

QModelIndex ReconciliationModel::currentReconciliationIndex(const QString& accountId) const
{
    const auto rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = index(row, 0);
        if (idx.data(eMyMoney::Model::ReconciliationCurrentRole).toBool()) {
            if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accountId) {
                return idx;
            }
        }
    }
    return {};
}
