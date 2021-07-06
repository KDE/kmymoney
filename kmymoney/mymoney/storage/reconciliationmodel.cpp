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

            insertRows(0, history.count());

            int row = 0;
            QMap<QDate, MyMoneyMoney>::const_iterator it;
            for (it = history.constBegin(); it != history.constEnd(); ++it) {
                ReconciliationEntry entry(nextId(), accountId, it.key(), *it);
                static_cast<TreeItem<ReconciliationEntry>*>(index(row, 0).internalPointer())->dataRef() = entry;
                ++row;
            }
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

    case eMyMoney::Model::TransactionPostDateRole:
        return reconciliationEntry.date();

    case eMyMoney::Model::SplitAccountIdRole:
    case eMyMoney::Model::JournalSplitAccountIdRole:
        return reconciliationEntry.accountId();

    case eMyMoney::Model::ReconciliationAmountRole:
        return QVariant::fromValue(reconciliationEntry.amount());

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
