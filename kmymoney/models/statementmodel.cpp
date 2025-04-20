/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// just make sure that the assertions always work in this model
#ifndef QT_FORCE_ASSERTS
#define QT_FORCE_ASSERTS
#endif

#include "statementmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QFont>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLazyLocalizedString>
#include <QHeaderView>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"

struct StatementModel::Private {
    Q_DECLARE_PUBLIC(StatementModel)

    Private(StatementModel* qq, QObject* parent)
        : q_ptr(qq)
        , parentObject(parent)
    {
    }

    StatementModel* q_ptr;
    QObject* parentObject;
};

StatementModel::StatementModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<StatementEntry>(parent, QStringLiteral("STMT"), StatementModel::ID_SIZE, undoStack)
    , d(new Private(this, parent))
{
    setObjectName(QLatin1String("StatementModel"));

    setUseIdToItemMapper(true);
    setFullTableScan(true);

    // force creation of empty account structure
    unload();
}

StatementModel::~StatementModel()
{
}

void StatementModel::clearModelItems()
{
    MyMoneyModel::clearModelItems();
}

QString StatementModel::noInstitutionId() const
{
    return QLatin1String("NoInstId");
}

int StatementModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return Column::MaxColumns;
}

QVariant StatementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        // Using Qt::UserRole here to store QHeaderView::ResizeMode
        switch (role) {
        case Qt::UserRole:
            if (section == Column::AccountName) {
                return QHeaderView::Stretch;
            }
            return QHeaderView::ResizeToContents;

        case Qt::DisplayRole:
            switch (section) {
            case Column::AccountName:
                return i18nc("@title:column Shows the Name of account", "Name");
            case Column::StatementDate:
                return i18nc("@title:column Shows the date of statement", "Date");
            case Column::StatementBalance:
                return i18nc("@title:column Shows statement balance", "Balance");
            case Column::Read:
                return i18nc("@title:column Shows number of transactions read", "Processed");
            case Column::Added:
                return i18nc("@title:column Shows number of imported transactions", "Imported");
            case Column::Matched:
                return i18nc("@title:column Shows number of matched transactions", "Matched");
            case Column::Duplicate:
                return i18nc("@title:column Shows number of duplicate transactions", "Duplicates");
            case Column::PayeesCreated:
                return i18nc("@title:column Shows number of create payees", "Payees created");
            default:
                break;
            }
            return QVariant();

#if 0
        case Qt::ToolTipRole:
            switch (section) {
            case Column::AccountName:
                return i18nc("@info:tooltip for 'Name' column. Used in Account, Category and Institution context, so avoid being too specific", "Full name");
            case Column::Type:
                return i18nc("@info:tooltip for 'Type' column, used in both Account and Category context, so avoid being too specific", "Type");
            case Column::HasOnlineMapping:
                return i18nc("@info:tooltip for 'Online' column (YES/NO value)", "Whether account has online mapping set up");
            case Column::Tax:
                return i18nc("@info:tooltip for 'Tax' column (YES/NO value)", "Whether account is included in tax reports");
            case Column::Vat:
                return i18nc("@info:tooltip for 'VAT' column (% rate)", "VAT percentage rate, if configured.");
            case Column::CostCenter:
                return i18nc("@info:tooltip for 'CC' column (YES/NO value)", "Whether a cost center assignment is necessary when entering a transaction.");
            case Column::Balance:
                return i18nc("@info:tooltip for 'Balance' column", "Account balance (in account's currency)");
            case Column::PostedValue:
                return i18n("Posted Value");
            case Column::TotalPostedValue:
                return i18nc("@info:tooltip for 'Total Value' column", "Total Value (including subaccounts)");
            case Column::Number:
                return i18nc("@info:tooltip for 'Account Number' column", "Account or credit card number, as assigned by the institution.");
            case Column::Iban:
                return i18nc("@info:tooltip for 'IBAN' column", "Account number in IBAN (International Bank Account Number) format");
            case Column::BankCode:
                return i18nc(
                    "@info:tooltip for 'Bank Code' column. Include an example of the culture-specific term for 'Bank Code' here, if one is common "
                    "(https://en.wikipedia.org/wiki/Bank_code).",
                    "A bank code assigned to Institution by a central bank, a bank supervisory body or a Bankers Association. Known as 'Routing Number' (US "
                    "accounts) or a 'Sort Code' (UK accounts).");
            case Column::Bic:
                return i18nc("@info:tooltip for 'SWIFT/BIC' column", "A Business Identifier Code, also known as SWIFT.");
            default:
                break;
            }
            return QVariant();
#endif

        case Qt::TextAlignmentRole:
            switch (section) {
            case Column::StatementBalance:
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);

            case Column::Read:
            case Column::Added:
            case Column::Matched:
            case Column::Duplicate:
            case Column::PayeesCreated:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            default:
                break;
            }
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
    return MyMoneyModelBase::headerData(section, orientation, role);
}

QVariant StatementModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return QVariant();

    const StatementEntry& statement = static_cast<TreeItem<StatementEntry>*>(idx.internalPointer())->constDataRef();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx.parent().isValid()) {
            // it's an account entry
            switch (idx.column()) {
            case Column::AccountName:
                return statement.name();

            case Column::StatementDate:
                if (statement.statementDate().isValid()) {
                    return MyMoneyUtils::formatDate(statement.statementDate());
                }
                break;

            case Column::StatementBalance:
                if (statement.balance().isAutoCalc()) {
                    return QLatin1String("---");
                }
                return MyMoneyUtils::formatMoney(statement.balance(), MyMoneySecurity());

            case Column::Read:
                return statement.transactionsCount();

            case Column::Added:
                return statement.transactionsAdded();

            case Column::Matched:
                return statement.transactionsMatched();

            case Column::Duplicate:
                return statement.transactionDuplicates();

            case Column::PayeesCreated:
                return statement.payeesCreated();
            }

        } else if (idx.column() == Column::AccountName) {
            // it's an institution entry and we only fill the account name column
            return statement.name();
        }

        break;

    case Qt::TextAlignmentRole:
        return headerData(idx.column(), Qt::Horizontal, role);

    case Qt::ForegroundRole:
        break;

    case Qt::FontRole: {
        QFont font;
        // display top level account groups in bold
        if (!idx.parent().isValid()) {
            font.setBold(true);

        } else if ((idx.column() == Column::Added) && (statement.transactionsAdded() > 0)) {
            font.setBold(true);
        }
        return font;
    } break;

    case Qt::DecorationRole:
        switch (idx.column()) {
        case Column::AccountName:
            // display top level groups as institution
            if (!idx.parent().isValid()) {
                return Icons::get(Icons::Icon::BankAccount);
#if 0
            } else {
                return account.accountIcon();
#endif
            }
            break;

        default:
            break;
        }
        break;

    case eMyMoney::Model::IdRole:
        return statement.id();

    case eMyMoney::Model::AccountNameRole:
        return statement.name();

    case eMyMoney::Model::StatementBalanceRole:
        return QVariant::fromValue(statement.balance());

    case eMyMoney::Model::StatementEndDateRole:
        return statement.statementDate();

    case eMyMoney::Model::StatementTransactionCountRole:
        return statement.transactionsCount();

    case eMyMoney::Model::StatementTransactionsAddedRole:
        return statement.transactionsAdded();

    case eMyMoney::Model::StatementTransactionsMatchedRole:
        return statement.transactionsMatched();

    case eMyMoney::Model::StatementTransactionDuplicatesRole:
        return statement.transactionDuplicates();

    case eMyMoney::Model::StatementPayeesCreatedRole:
        return statement.payeesCreated();

    default:
        break;
    }
    return {};
}

bool StatementModel::setData(const QModelIndex& idx, const QVariant& v, int role)
{
    if (!idx.isValid()) {
        return false;
    }

    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return false;

    StatementEntry& statement = static_cast<TreeItem<StatementEntry>*>(idx.internalPointer())->dataRef();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        break;

    case eMyMoney::Model::StatementBalanceRole:
        statement.setBalance(v.value<MyMoneyMoney>());
        break;
    case eMyMoney::Model::StatementEndDateRole:
        statement.setStatementDate(v.toDate());
        break;
    case eMyMoney::Model::StatementTransactionCountRole:
        statement.setTransactionsCount(v.toInt());
        break;
    case eMyMoney::Model::StatementTransactionsAddedRole:
        statement.setTransactionsAdded(v.toInt());
        break;
    case eMyMoney::Model::StatementTransactionsMatchedRole:
        statement.setTransactionsMatched(v.toInt());
        break;
    case eMyMoney::Model::StatementTransactionDuplicatesRole:
        statement.setTransactionDuplicates(v.toInt());
        break;
    case eMyMoney::Model::StatementPayeesCreatedRole:
        statement.setPayeesCreated(v.toInt());
        break;
    }
    return false;
}

Qt::ItemFlags StatementModel::flags(const QModelIndex& idx) const
{
    // the following sets the flag for a regular account entry
    Qt::ItemFlags flags = idx.data(eMyMoney::Model::StatementTransactionsAddedRole).toInt() > 0 ? Qt::ItemIsEnabled : Qt::NoItemFlags;

    // for an institution entry, we need to scan all accounts
    if (!idx.parent().isValid()) {
        const auto rows = rowCount(idx);
        for (int row = 0; row < rows; ++row) {
            const auto accountIdx = index(row, 0, idx);
            if (accountIdx.data(eMyMoney::Model::StatementTransactionsAddedRole).toInt() > 0) {
                flags = Qt::ItemIsEnabled;
                break;
            }
        }
    }
    return flags;
}

QModelIndex StatementModel::addItem(const QString& id, const QString& name, const QModelIndex& parentIdx)
{
    auto statement = StatementEntry(id, StatementEntry());
    statement.setName(name);
    doAddItem(statement, parentIdx);
    return indexById(statement.id());
}

uint StatementModel::statementCount() const
{
    uint institutionCount = 0;
    const auto institutions = rowCount();
    for (int row = 0; row < institutions; ++row) {
        const auto idx = index(row, 0);
        institutionCount += rowCount(idx);
    }
    return institutionCount;
}
