/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// just make sure that the assertions always work in this model
#ifndef QT_FORCE_ASSERTS
#define QT_FORCE_ASSERTS
#endif

#include "accountsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QFont>
#include <QIcon>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLazyLocalizedString>
#include <QHeaderView>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "securitiesmodel.h"

#include "icons.h"

void AccountBalances::clear()
{
    m_totalBalance = MyMoneyMoney();
    m_clearedBalance = MyMoneyMoney();
}

AccountBalances& AccountBalances::operator+=(const MyMoneySplit& split)
{
    m_totalBalance += split.shares();
    switch (split.reconcileFlag()) {
    case eMyMoney::Split::State::Frozen:
    case eMyMoney::Split::State::Cleared:
    case eMyMoney::Split::State::Reconciled:
        m_clearedBalance += split.shares();
        break;
    default:
        break;
    }
    return *this;
}

AccountBalances& AccountBalances::operator-=(const MyMoneySplit& split)
{
    m_totalBalance -= split.shares();
    switch (split.reconcileFlag()) {
    case eMyMoney::Split::State::Frozen:
    case eMyMoney::Split::State::Cleared:
    case eMyMoney::Split::State::Reconciled:
        m_clearedBalance -= split.shares();
        break;
    default:
        break;
    }
    return *this;
}

AccountBalances& AccountBalances::operator*=(const MyMoneyMoney& factor)
{
    m_totalBalance *= factor;
    m_clearedBalance *= factor;
    return *this;
}

AccountBalances& AccountBalances::operator/=(const MyMoneyMoney& factor)
{
    m_totalBalance /= factor;
    m_clearedBalance /= factor;
    return *this;
}

struct AccountsModel::Private
{
    Q_DECLARE_PUBLIC(AccountsModel)

    Private(AccountsModel* qq, QObject* parent)
        : q_ptr(qq)
        , parentObject(parent)
        , updateOnBalanceChange(true)
    {
    }

    int loadSubAccounts(const QModelIndex parent, const QMap<QString, MyMoneyAccount>& list)
    {
        Q_Q(AccountsModel);
        const auto parentAccount = static_cast<TreeItem<MyMoneyAccount>*>(parent.internalPointer())->constDataRef();

        // create entries for the sub accounts
        const int subAccounts = parentAccount.accountCount();
        int itemCount = subAccounts;
        if (subAccounts > 0) {
            q->insertRows(0, subAccounts, parent);
            for (int row = 0; row < subAccounts; ++row) {
                const auto subAccountId = parentAccount.accountList().at(row);
                const auto subAccount = list.value(subAccountId);
                if (subAccount.id() == subAccountId) {
                    q->updateNextObjectId(subAccount.id());
                    const auto idx = q->index(row, 0, parent);
                    static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->dataRef() = subAccount;
                    if (q->m_idToItemMapper) {
                        q->m_idToItemMapper->insert(subAccountId, static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer()));
                    }
                    if (subAccount.value("PreferredAccount") == QLatin1String("Yes")
                            && !subAccount.isClosed() ) {
                        q->addFavorite(subAccountId);
                        ++itemCount;
                    }
                    itemCount += loadSubAccounts(idx, list);

                } else {
                    qDebug() << "Account" << parentAccount.id() << ": subaccount with ID" << subAccountId << "not found in list";
                }
            }
        }
        return itemCount;
    }

    QMap<QString, MyMoneyAccount> checkHierarchy(const QMap<QString, MyMoneyAccount>& _list)
    {
        auto list(_list);

        qDebug() << "Start verifying account hierarchy";
        // check forwards: entries in the accountList member of an
        // account must have the current account as parent. If not
        // or the accountId does not exist at all, the  accountId
        // will be removed from the accountList member.
        for (auto ita = list.begin(); ita != list.end(); ++ita) {
            const QStringList accountIds = (*ita).accountList();
            for (const auto& subAccount : accountIds) {
                if (!_list.contains(subAccount) || _list.value(subAccount).parentAccountId() != (*ita).id()) {
                    (*ita).removeAccountId(subAccount);
                    qDebug() << "check account hierarchy:" << "removed" << subAccount << "from" << (*ita).id();
                }
            }
        }

        // check backwards: make sure, the parentId points to an
        // existing account. If not, attach it to the top account.
        // Make sure, the parentId is listed in the parent account's
        // sub-account accountList member. If not, add it to the parent.
        // Don't check the parent of the top level accounts.
        for (auto ita = list.begin(); ita != list.end(); ++ita) {
            // standard account?
            if ((*ita).id().startsWith("AStd"))
                continue;
            // parent does not exist?
            if (!_list.contains((*ita).parentAccountId())) {
                const auto newParentid = MyMoneyAccount::stdAccName(static_cast<eMyMoney::Account::Standard>((*ita).accountGroup()));
                (*ita).setParentAccountId(newParentid);
                qDebug() << "check account hierarchy:" << "reparented" << (*ita).id() << "to" << newParentid;
            }
            // if parent does not know about us?
            auto itb = list.find((*ita).parentAccountId());
            if (!(*itb).accountList().contains((*ita).id())) {
                (*itb).addAccountId((*ita).id());
                qDebug() << "check account hierarchy:" << "added" << (*ita).id() << "to" << (*itb).id();
            }
        }
        qDebug() << "End verifying account hierarchy";
        return list;
    }

    bool isFavoriteIndex(const QModelIndex& idx) const
    {
        if (idx.isValid()) {
            TreeItem<MyMoneyAccount> *item;
            item = static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer());
            if (item->constDataRef().id() == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Favorite)) {
                return true;
            }
        }
        return false;
    }

    MyMoneyMoney adjustedBalance(const MyMoneyMoney& amount, const MyMoneyAccount& account)
    {
        switch(account.accountGroup()) {
        case eMyMoney::Account::Type::Liability:
        case eMyMoney::Account::Type::Income:
        case eMyMoney::Account::Type::Equity:
            return -amount;
        default:
            break;
        }
        return amount;
    }

    MyMoneyMoney calculateTotalValue(QModelIndex idx)
    {
        Q_Q(AccountsModel);
        MyMoneyMoney result = idx.data(eMyMoney::Model::AccountValueRole).value<MyMoneyMoney>();
        const auto rows = q->rowCount(idx);
        for (int row = 0; row < rows; ++row) {
            QModelIndex subIdx = q->index(row, 0, idx);
            result += calculateTotalValue(subIdx);
        }
        q->setData(idx, QVariant::fromValue(result), eMyMoney::Model::AccountTotalValueRole);
        return result;
    }

    MyMoneyMoney netWorth() const
    {
        Q_Q(const AccountsModel);
        return q->assetIndex().data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>()
               + q->liabilityIndex().data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>();
    }

    MyMoneyMoney profitLoss() const
    {
        Q_Q(const AccountsModel);
        return -q->incomeIndex().data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>()
               - q->expenseIndex().data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>();
    }

    QPair<MyMoneyMoney, bool> balanceToValue(const MyMoneyAccount& account, const MyMoneyMoney bal)
    {
        // now calculate the value, but only for balances differing from null
        const auto file = MyMoneyFile::instance();
        bool needConvert = false;
        MyMoneyMoney accountValue(bal);
        QList<MyMoneyPrice> prices;
        MyMoneyPrice price;
        bool approximate = false;
        if (!bal.isZero()) {
            const auto baseCurrency = file->baseCurrency();
            MyMoneySecurity security(baseCurrency);
            if (account.isInvest()) {
                security = file->security(account.currencyId());
                if (!security.id().isEmpty()) {
                    price = file->price(account.currencyId(), security.tradingCurrency());
                    if (!price.isValid()) {
                        price = MyMoneyPrice(security.tradingCurrency(), account.currencyId(), QDate::currentDate(), MyMoneyMoney::ONE, "internal");
                        approximate = true;
                    }
                    prices += price;
                    if (security.tradingCurrency() != baseCurrency.id()) {
                        MyMoneySecurity sec = file->currency(security.tradingCurrency());
                        if (!sec.id().isEmpty()) {
                            price = file->price(sec.id(), baseCurrency.id());
                            if (!price.isValid()) {
                                price = MyMoneyPrice(sec.id(), baseCurrency.id(), QDate::currentDate(), MyMoneyMoney::ONE, "internal");
                                approximate = true;
                            }
                            prices += price;
                        } else {
                            qDebug() << security.tradingCurrency() << "not found";
                            approximate = true;
                        }
                    }
                }
                needConvert = true;
            } else if (account.currencyId() != baseCurrency.id()) {
                security = file->currency(account.currencyId());
                if (!security.id().isEmpty()) {
                    price = file->price(account.currencyId(), baseCurrency.id());
                    if (!price.isValid()) {
                        price = MyMoneyPrice(account.currencyId(), baseCurrency.id(), QDate::currentDate(), MyMoneyMoney::ONE, "internal");
                        approximate = true;
                    }
                    prices += price;
                } else {
                    qDebug() << security.id() << "not found";
                    approximate = true;
                }
                needConvert = true;
            }
        }

        if (needConvert) {
            QList<MyMoneyPrice>::const_iterator it_p;
            QString securityID = account.currencyId();
            for (it_p = prices.constBegin(); it_p != prices.constEnd(); ++it_p) {
                const auto prec = MyMoneyMoney::denomToPrec(file->security(securityID).pricePrecision());
                accountValue = (accountValue * (MyMoneyMoney::ONE / (*it_p).rate(securityID))).convertPrecision(prec);
                if ((*it_p).from() == securityID)
                    securityID = (*it_p).to();
                else
                    securityID = (*it_p).from();
            }
            accountValue = accountValue.convert(file->baseCurrency().smallestAccountFraction());
        }
        return qMakePair(accountValue, approximate);
    }

    MyMoneyAccount itemByName(const QString& name, const QModelIndex& start) const
    {
        Q_Q(const AccountsModel);
        QStringList parts = name.split(MyMoneyAccount::accountSeparator());
        if (parts.isEmpty())
            return {};

        QModelIndex parent = start;
        QModelIndexList indexes;
        do {
            indexes = q->MyMoneyModelBase::indexListByName(parts.at(0), parent);
            if (indexes.isEmpty()) {
                return {};
            }
            parent = indexes.first();
            parts.takeFirst();
        } while(!parts.isEmpty());
        return q->MyMoneyModel::itemByIndex(parent);
    }

    bool hasOnlineBalance(const MyMoneyAccount& account)
    {
        return !(account.value("lastImportedTransactionDate").isEmpty()
                 || account.value("lastStatementBalance").isEmpty());
    }

    QString formatIban(const MyMoneyAccount& account)
    {
        auto txt(account.value("iban"));
        txt.remove(QLatin1String(" "));
        int pos(4);
        while (pos < txt.length()) {
            txt.insert(pos, QLatin1String(" "));
            pos += 5;
        }
        return txt;
    }

    struct DefaultAccounts {
        eMyMoney::Account::Standard groupType;
        eMyMoney::Account::Type     accountType;
        const char*                 description;
    };
    const QVector<DefaultAccounts> defaults = {
        {eMyMoney::Account::Standard::Favorite, eMyMoney::Account::Type::Asset, kli18n("Favorite").untranslatedText()},
        {eMyMoney::Account::Standard::Asset, eMyMoney::Account::Type::Asset, kli18n("Asset accounts").untranslatedText()},
        {eMyMoney::Account::Standard::Liability, eMyMoney::Account::Type::Liability, kli18n("Liability accounts").untranslatedText()},
        {eMyMoney::Account::Standard::Income, eMyMoney::Account::Type::Income, kli18n("Income categories").untranslatedText()},
        {eMyMoney::Account::Standard::Expense, eMyMoney::Account::Type::Expense, kli18n("Expense categories").untranslatedText()},
        {eMyMoney::Account::Standard::Equity, eMyMoney::Account::Type::Equity, kli18n("Equity accounts").untranslatedText()},
    };

    AccountsModel*                  q_ptr;
    QObject*                        parentObject;
    QHash<QString, MyMoneyMoney>    balance;
    QHash<QString, MyMoneyMoney>    value;
    QHash<QString, MyMoneyMoney>    totalValue;
    bool                            updateOnBalanceChange;
    QColor                          positiveScheme;
    QColor                          negativeScheme;
};

AccountsModel::AccountsModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<MyMoneyAccount>(parent, QStringLiteral("A"), AccountsModel::ID_SIZE, undoStack)
    , d(new Private(this, parent))
{
    // validate that our assumptions about the order of the
    // groups in the model are still correct
    Q_ASSERT(d->defaults[0].groupType == eMyMoney::Account::Standard::Favorite);
    Q_ASSERT(d->defaults[1].groupType == eMyMoney::Account::Standard::Asset);
    Q_ASSERT(d->defaults[2].groupType == eMyMoney::Account::Standard::Liability);
    Q_ASSERT(d->defaults[3].groupType == eMyMoney::Account::Standard::Income);
    Q_ASSERT(d->defaults[4].groupType == eMyMoney::Account::Standard::Expense);
    Q_ASSERT(d->defaults[5].groupType == eMyMoney::Account::Standard::Equity);

    setObjectName(QLatin1String("AccountsModel"));

    useIdToItemMapper(true);

    // force creation of empty account structure
    unload();
}

AccountsModel::~AccountsModel()
{
}

int AccountsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return Column::MaxColumns;
}

QVariant AccountsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal) {
        // Using Qt::UserRole here to store QHeaderView::ResizeMode
        if (role == Qt::UserRole)
            switch (section) {
            case Column::AccountName:
                return QHeaderView::Stretch;
            default:
                return QHeaderView::ResizeToContents;
            }
        if (role == Qt::DisplayRole) {
            switch (section) {
            case Column::AccountName:
                return i18nc("@title:column Shows the Name of account", "Name");
            case Column::Type:
                return i18nc("@title:column Shows the Type of account (e.g. Asset, Checking, Credit Card)", "Type");
            case Column::HasOnlineMapping:
                return i18nc("@title:column Indicates whether account has an online mapping configured", "Online");
            case Column::Tax:
                return i18nc("@title:column Indicates whether account is included in tax reports", "Tax");
            case Column::Vat:
                return i18nc("@title:column Shows VAT percentage rate, if configured", "VAT");
            case Column::CostCenter:
                return i18nc("@title:column Indicates whether a Cost Center assignment is necessary when entering a transaction", "CC");
            case Column::Balance:
                return i18nc("@title:column Shows account's balance (in account's currency)", "Balance");
            case Column::PostedValue:
                return i18n("Posted Value");
            case Column::TotalPostedValue:
                return i18nc("@title:column Shows Total Value, which includes a sum of all subaccounts (in base currency)", "Total Value");
            case Column::Number:
                return i18nc("@title:column Shows Account Number", "Number");
            case Column::Iban:
                return i18nc("@title:column Shows IBAN", "IBAN");
            case Column::BankCode:
                return i18nc(
                    "@title:column Shows Bank Code. Use a generic form, do not translate using a culture-specific term, such as 'Routing Number' (en_US) or "
                    "'Sort Code' (en_GB)",
                    "Bank Code");
            case Column::Bic:
                return i18nc("@title:column Shows SWIFT/BIC", "SWIFT/BIC");
            default:
                return QVariant();
            }
        }

        if (role == Qt::ToolTipRole) {
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
                return QVariant();
            }
        }
    }
    return MyMoneyModelBase::headerData(section, orientation, role);
}

QVariant AccountsModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return QVariant();

    const MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->constDataRef();

    if (d->isFavoriteIndex(idx.parent())) {
        if (role == eMyMoney::Model::AccountIsFavoriteIndexRole) {
            return true;
        }
        const auto accountIdx = indexById(account.id());
        const auto subIdx = index(accountIdx.row(), idx.column(), accountIdx.parent());
        return data(subIdx, role);
    }

    MyMoneyAccount tradingCurrency;
    const auto isInvestmentAccount = account.accountType() == eMyMoney::Account::Type::Investment;

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(idx.column()) {
        case Column::AccountName:
            // make sure to never return any displayable text for the dummy entry
            if (!account.id().isEmpty()) {
                return account.name();
            }
            break;

        case Column::Type:
            return account.accountTypeToString(account.accountType());

        case Column::HasOnlineMapping:
            if (account.hasOnlineMapping()) {
                return QChar(0x2713);
            }
            break;

        case AccountsModel::Column::CostCenter:
            if (account.isCostCenterRequired()) {
                return QChar(0x2713);
            }
            break;

        case AccountsModel::Column::Tax:
            if (account.isInTaxReports()) {
                return QChar(0x2713);
            }
            break;

        case Column::Vat:
            if (!account.value("VatAccount").isEmpty()) {
                const auto vatAccount = itemById(account.value("VatAccount"));
                return vatAccount.name();

                // VAT Rate
            } else if (!account.value("VatRate").isEmpty()) {
                const auto vatRate = MyMoneyMoney(account.value("VatRate")) * MyMoneyMoney(100, 1);
                return QString::fromLatin1("%1 %").arg(vatRate.formatMoney(QString(), 1));
            }
            break;

        case Column::Balance:
        {
            try {
                auto security = MyMoneyFile::instance()->security(account.currencyId());
                const auto prec = MyMoneyMoney::denomToPrec(account.fraction());
                return d->adjustedBalance(account.balance(), account).formatMoney(security.tradingSymbol(), prec);
            } catch (const MyMoneyException&) {
            }
        }

        case Column::PostedValue:
        {
            const auto baseCurrency = MyMoneyFile::instance()->baseCurrency();
            return d->adjustedBalance(account.postedValue(), account).formatMoney(baseCurrency.tradingSymbol(), MyMoneyMoney::denomToPrec(baseCurrency.smallestAccountFraction()));
        }

        case Column::TotalPostedValue:
        {
            const auto baseCurrency = MyMoneyFile::instance()->baseCurrency();
            return d->adjustedBalance(account.totalPostedValue(), account).formatMoney(baseCurrency.tradingSymbol(), MyMoneyMoney::denomToPrec(baseCurrency.smallestAccountFraction()));
        }

        case Column::Bic:
            return account.value("bic");

        case Column::Number:
            return account.number();

        case Column::Iban:
            return d->formatIban(account);

        default:
            break;
        }
        break;

    case Qt::TextAlignmentRole:
        switch (idx.column()) {
        case AccountsModel::Column::Vat:
        case AccountsModel::Column::Balance:
        case AccountsModel::Column::PostedValue:
        case AccountsModel::Column::TotalPostedValue:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case AccountsModel::Column::HasOnlineMapping:
        case AccountsModel::Column::CostCenter:
        case AccountsModel::Column::Tax:
            return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        default:
            break;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case Qt::ForegroundRole:
        switch(idx.column()) {
        case Column::Balance:
            return d->adjustedBalance(account.balance(), account).isNegative() ? d->negativeScheme : d->positiveScheme;

        case Column::PostedValue:
            return d->adjustedBalance(account.postedValue(), account).isNegative() ? d->negativeScheme : d->positiveScheme;

        case Column::TotalPostedValue:
            return d->adjustedBalance(account.totalPostedValue(), account).isNegative() ? d->negativeScheme : d->positiveScheme;
        }
        break;

    case Qt::FontRole:
    {
        QFont font;
        // display top level account groups in bold
        if (!idx.parent().isValid()) {
            font.setBold(true);
        }
        // display the names of closed accounts with strikeout font
        // all others without
        if (account.isClosed()) {
            font.setStrikeOut(account.isClosed());
        }
        return font;
    }
    break;

    case Qt::DecorationRole:
        switch (idx.column()) {
        case AccountsModel::Column::AccountName:
            if (d->isFavoriteIndex(idx)) {
                return Icons::get(Icons::Icon::BankAccount);
            } else {
                return account.accountIcon();
            }
            break;

        default:
            break;
        }
        break;

    case eMyMoney::Model::IdRole:
        return account.id();

    case eMyMoney::Model::AccountTypeRole:
        return QVariant::fromValue(account.accountType());

    case eMyMoney::Model::AccountIsClosedRole:
        return account.isClosed();

    case eMyMoney::Model::AccountCanBeClosedRole:
        if (!account.balance().isZero())
            return false;
        if (account.hasOnlineMapping())
            return false;
        {
            // an investment account can be closed if all
            // subaccounts can be closed. For all others,
            // the subaccounts already have to be closed
            const auto subAccountRows = rowCount();
            const auto closeRole = isInvestmentAccount ? eMyMoney::Model::AccountCanBeClosedRole : eMyMoney::Model::AccountIsClosedRole;
            for (int row = 0; row < subAccountRows; ++row) {
                const auto subIdx = index(row, 0, idx);
                if (!subIdx.data(closeRole).toBool()) {
                    return false;
                }
            }
        }
        return true;

    case eMyMoney::Model::AccountIsAssetLiabilityRole:
        return account.isAssetLiability();

    case eMyMoney::Model::AccountIsIncomeExpenseRole:
        return account.isIncomeExpense();

    case eMyMoney::Model::AccountFullHierarchyNameRole:
        return indexToHierarchicalName(idx, true);

    case eMyMoney::Model::AccountNameRole:
        return account.name();

    case eMyMoney::Model::AccountFullNameRole:
        return indexToHierarchicalName(idx, false);

    case eMyMoney::Model::AccountDisplayOrderRole:
        // is it a normal account, make it high
        if (idx.parent().isValid())
            return 99;
        // otherwise sort in the group order of the model
        return idx.row();

    case eMyMoney::Model::AccountFractionRole:
        return account.fraction();

    case eMyMoney::Model::AccountParentIdRole:
        return account.parentAccountId();

    case eMyMoney::Model::AccountTotalValueRole:
        return QVariant::fromValue(account.totalPostedValue());

    case eMyMoney::Model::AccountValueRole:
        return QVariant::fromValue(account.postedValue());

    case eMyMoney::Model::AccountBalanceRole:
        return QVariant::fromValue(account.balance());

    case eMyMoney::Model::AccountCurrencyIdRole:
        return account.currencyId();

    case eMyMoney::Model::AccountInstitutionIdRole:
        return account.institutionId();

    case eMyMoney::Model::AccountGroupRole:
        return static_cast<int>(account.accountGroup());

    case eMyMoney::Model::AccountOnlineBalanceDateRole:
        if (d->hasOnlineBalance(account)) {
            return QDate::fromString(account.value("lastImportedTransactionDate"), Qt::ISODate);
        }
        break;

    case eMyMoney::Model::AccountOnlineBalanceValueRole:
        if (d->hasOnlineBalance(account)) {
            auto balance = MyMoneyMoney(account.value("lastStatementBalance"));
            // revert the sign for liability accouts
            if (account.accountGroup() == eMyMoney::Account::Type::Liability) {
                balance = -balance;
            }
            return QVariant::fromValue(balance);
        }
        break;

    case eMyMoney::Model::AccountIsFavoriteIndexRole:
        return false;

    case eMyMoney::Model::AccountIsInvestRole:
        return account.isInvest();
    }
    return QVariant();
}

bool AccountsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    // check if something is performed on a favorite entry
    // and skip right away
    if (d->isFavoriteIndex(index.parent())) {
        return true;
    }

    MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(index.internalPointer())->dataRef();
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
#if 0
        switch(index.column()) {
        case Column::Name:
            account.setName(value.toString());
            rc = true;
            break;
        default:
            break;
        }
#endif
        break;
    case eMyMoney::Model::AccountBalanceRole:
        account.setBalance(value.value<MyMoneyMoney>());
        return true;

    case eMyMoney::Model::AccountValueRole:
        account.setPostedValue(value.value<MyMoneyMoney>());
        return true;

    case eMyMoney::Model::AccountTotalValueRole:
        account.setTotalPostedValue(value.value<MyMoneyMoney>());
        return true;

    default:
        if (role >= Qt::UserRole) {
            qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
        }
        break;
    }
    return QAbstractItemModel::setData(index, value, role);
}

void AccountsModel::setColorScheme(AccountsModel::ColorScheme scheme, const QColor& color)
{
    switch(scheme) {
    case Positive:
        d->positiveScheme = color;
        break;
    case Negative:
        d->negativeScheme = color;
        break;
    }
}

void AccountsModel::clearModelItems()
{
    MyMoneyModel<MyMoneyAccount>::clearModelItems();

    // create the account groups with favorite entry as the first thing
    int row = 0;
    insertRows(0, static_cast<int>(eMyMoney::Account::Standard::MaxGroups));
    Q_FOREACH(auto baseAccount, d->defaults) {
        MyMoneyAccount account;
        account.setName(i18n(baseAccount.description));
        account.setAccountType(baseAccount.accountType);
        auto newAccount = MyMoneyAccount(MyMoneyAccount::stdAccName(baseAccount.groupType), account);
        static_cast<TreeItem<MyMoneyAccount>*>(index(row, 0).internalPointer())->dataRef() = newAccount;
        ++row;
    }
}

QModelIndex AccountsModel::favoriteIndex() const
{
    return index(0, 0);
}

QModelIndex AccountsModel::assetIndex() const
{
    return index(1, 0);
}

QModelIndex AccountsModel::liabilityIndex() const
{
    return index(2, 0);
}

QModelIndex AccountsModel::incomeIndex() const
{
    return index(3, 0);
}

QModelIndex AccountsModel::expenseIndex() const
{
    return index(4, 0);
}

QModelIndex AccountsModel::equityIndex() const
{
    return index(5, 0);
}

void AccountsModel::load(const QMap<QString, MyMoneyAccount>& _list)
{
    QElapsedTimer t;

    t.start();
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    auto list = d->checkHierarchy(_list);
    int itemCount = 0;
    for(const auto& baseAccount : d->defaults) {
        ++itemCount;
        // we have nothing to do for favorites
        if (baseAccount.groupType == eMyMoney::Account::Standard::Favorite)
            continue;
        const auto account = list.value(MyMoneyAccount::stdAccName(baseAccount.groupType));
        if (account.id() == MyMoneyAccount::stdAccName(baseAccount.groupType)) {
            const auto idx = indexById(account.id());
            static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->dataRef() = account;
            itemCount += d->loadSubAccounts(idx, list);
        } else {
            qDebug() << "Baseaccount for" << MyMoneyAccount::stdAccName(baseAccount.groupType) << "not found in list";
        }
    }

    // and don't count loading as a modification
    setDirty(false);

    endResetModel();

    Q_EMIT modelLoaded();

    qDebug() << "Model for accounts loaded with" << itemCount << "items in" << t.elapsed() << "ms";
}

QList<MyMoneyAccount> AccountsModel::itemList() const
{
    QList<MyMoneyAccount> list;
    // never search in the first row which is favorites
    QModelIndexList indexes = match(assetIndex(), eMyMoney::Model::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);

    for (int row = 0; row < indexes.count(); ++row) {
        const MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(indexes.value(row).internalPointer())->constDataRef();
        if (!account.id().startsWith("AStd"))
            list.append(account);
    }
    return list;
}

QModelIndex AccountsModel::indexById(const QString& id) const
{
    if (m_idToItemMapper) {
        const auto item = m_idToItemMapper->value(id, nullptr);
        if (item) {
            return createIndex(item->row(), 0, item);
        }
    }
    // never search in the first row which is favorites
    const QModelIndexList indexes = match(assetIndex(), eMyMoney::Model::IdRole, id, 1, Qt::MatchFixedString | Qt::MatchRecursive);
    if (indexes.isEmpty())
        return QModelIndex();
    return indexes.first();
}

QModelIndexList AccountsModel::indexListByName(const QString& name, const QModelIndex& parent) const
{
    // never search in the first row which is favorites
    Q_UNUSED(parent)
    return match(assetIndex(), Qt::DisplayRole, name, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive | Qt::MatchRecursive);
}

void AccountsModel::addFavorite(const QString& id)
{
    // no need to do anything if it is already listed
    // bypass our own indexById as it does not return favorites
    const auto favoriteIdx = MyMoneyModel<MyMoneyAccount>::indexById(MyMoneyAccount::stdAccName(d->defaults[0].groupType));

    // check if the favorite is already present, if not add it
    const QModelIndexList indexes = match(index(0, 0, favoriteIdx), eMyMoney::Model::IdRole, id, 1, Qt::MatchFixedString | Qt::MatchRecursive);
    if (indexes.isEmpty()) {
        const auto count = rowCount(favoriteIdx);
        // we append a single row at the end
        const bool dirty = isDirty();
        insertRows(count, 1, favoriteIdx);
        const auto idx = index(count, 0, favoriteIdx);
        MyMoneyAccount subAccount(id, MyMoneyAccount());
        static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->dataRef() = subAccount;
        // don't modify the dirty flag here. This is done elsewhere.
        setDirty(dirty);
    }
}

void AccountsModel::touchAccountById(const QString& id)
{
    const auto idx = indexById(id);
    if (idx.isValid()) {
        static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->dataRef().touch();
        setDirty();
        Q_EMIT dataChanged(idx, idx);
    }
}

void AccountsModel::removeFavorite(const QString& id)
{
    // no need to do anything if it is not listed
    // bypass our own indexById as it does not return favorites
    const auto favoriteIdx = MyMoneyModel<MyMoneyAccount>::indexById(MyMoneyAccount::stdAccName(d->defaults[0].groupType));

    // check if the favorite is already present, if not add it
    const QModelIndexList indexes = match(index(0, 0, favoriteIdx), eMyMoney::Model::IdRole, id, 1, Qt::MatchFixedString | Qt::MatchRecursive);
    if (!indexes.isEmpty()) {
        const QModelIndex& idx = indexes.first();
        // we remove a single row
        const bool dirty = isDirty();
        removeRows(idx.row(), 1, favoriteIdx);
        // don't modify the dirty flag here. This is done elsewhere.
        setDirty(dirty);
    }
}

int AccountsModel::processItems(Worker *worker)
{
    // make sure to work only on real entries and not on favorites
    return MyMoneyModel<MyMoneyAccount>::processItems(worker, match(assetIndex(), eMyMoney::Model::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive));
}

QString AccountsModel::indexToHierarchicalName(const QModelIndex& _idx, bool includeStandardAccounts) const
{
    QString rc;
    auto idx(_idx);

    if (idx.isValid()) {
        do {
            const MyMoneyAccount& acc = static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->constDataRef();
            if (!rc.isEmpty())
                rc = MyMoneyAccount::accountSeparator() + rc;
            rc = acc.name() + rc;
            idx = idx.parent();
        } while (idx.isValid() && (includeStandardAccounts || idx.parent().isValid()));
    }
    return rc;
}

QString AccountsModel::accountIdToHierarchicalName(const QString& accountId, bool includeStandardAccounts) const
{
    return indexToHierarchicalName(indexById(accountId), includeStandardAccounts);
}


QString AccountsModel::accountNameToId(const QString& category, eMyMoney::Account::Type type) const
{
    QString id;

    // search the category in the expense accounts and if it is not found, try
    // to locate it in the income accounts in case the type is not provided
    if (type == eMyMoney::Account::Type::Unknown
            || type == eMyMoney::Account::Type::Expense) {
        id = d->itemByName(category, expenseIndex()).id();
    }

    if ((id.isEmpty() && type == eMyMoney::Account::Type::Unknown)
            || type == eMyMoney::Account::Type::Income) {
        id = d->itemByName(category, incomeIndex()).id();
    }

    return id;
}

void AccountsModel::setupAccountFractions()
{
    QModelIndexList indexes = match(assetIndex(), eMyMoney::Model::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);
    MyMoneySecurity currency;
    for (int row = 0; row < indexes.count(); ++row) {
        MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(indexes.value(row).internalPointer())->dataRef();
        if (account.currencyId() != currency.id()) {
            currency = MyMoneyFile::instance()->security(account.currencyId());
        }
        account.fraction(currency);
    }
}

QPair<MyMoneyMoney, bool> AccountsModel::balanceToValue(const QString& accountId, MyMoneyMoney balance) const
{
    const auto accountIdx = indexById(accountId);
    auto result = qMakePair(MyMoneyMoney(), true);

    if (accountIdx.isValid()) {
        MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(accountIdx.internalPointer())->dataRef();
        result = d->balanceToValue(account, balance);
    }
    return result;
}

void AccountsModel::updateAccountBalances(const QHash<QString, AccountBalances>& balances)
{
    const MyMoneyFile* file = MyMoneyFile::instance();
    const MyMoneySecurity baseCurrency = file->baseCurrency();

    // suppress single updates while processing whole batch
    d->updateOnBalanceChange = false;
    bool approximate = false;
    for(auto it = balances.constBegin(); it != balances.constEnd(); ++it) {
        MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(indexById(it.key()).internalPointer())->dataRef();
        account.setBalance(it.value().m_totalBalance);

        const auto result = d->balanceToValue(account, it.value().m_totalBalance);

        account.setPostedValue(result.first);
        approximate |= result.second;
    }

    // now that we have all values, we can calculate the total values in the parent accounts
    MyMoneyMoney netWorth = d->netWorth();
    MyMoneyMoney profit = d->profitLoss();

    for (int row = assetIndex().row(); row < rowCount(); ++row) {
        d->calculateTotalValue(index(row, 0));
    }
    // turn on update on balance change
    d->updateOnBalanceChange = true;

    MyMoneyMoney newNetWorth = d->netWorth();
    if (netWorth != newNetWorth)
        Q_EMIT netWorthChanged(newNetWorth, approximate);

    MyMoneyMoney newProfit = d->profitLoss();
    if (profit != newProfit)
        Q_EMIT profitLossChanged(newProfit, approximate);
}

void AccountsModel::addItem(MyMoneyAccount& account)
{
    auto parentIdx = indexById(account.parentAccountId());
    if (parentIdx.isValid()) {
        account = MyMoneyAccount(nextId(), account);
        m_undoStack->push(new UndoCommand(this, MyMoneyAccount(), account));
    }
}

QModelIndexList AccountsModel::accountsWithoutInstitutions() const
{
    return match(index(0, 0, assetIndex()), eMyMoney::Model::AccountInstitutionIdRole, QString(), -1, Qt::MatchExactly | Qt::MatchRecursive)
           + match(index(0, 0, liabilityIndex()), eMyMoney::Model::AccountInstitutionIdRole, QString(), -1, Qt::MatchExactly| Qt::MatchRecursive);
}

void AccountsModel::removeItem(const QModelIndex& idx)
{
    if (idx.isValid()) {
        // update the parent internal structure
        const auto parentIdx = idx.parent();
        if (parentIdx.isValid()) {
            auto account = itemByIndex(idx);
            m_undoStack->push(new UndoCommand(this, account, MyMoneyAccount()));
        }
    }
}


void AccountsModel::reparentAccount(const QString& accountId, const QString& newParentId)
{
    QModelIndex accountIdx = indexById(accountId);

    // keep the account data
    MyMoneyAccount account = static_cast<TreeItem<MyMoneyAccount>*>(accountIdx.internalPointer())->dataRef();
    // setup new parent
    MyMoneyAccount newAccount(account);
    newAccount.setParentAccountId(newParentId);

    m_undoStack->push(new UndoCommand(this, account, newAccount));
}

void AccountsModel::doAddItem(const MyMoneyAccount& item, const QModelIndex& parentIdx)
{
    Q_UNUSED(parentIdx);
    const auto realParentIdx = indexById(item.parentAccountId());
    static_cast<TreeItem<MyMoneyAccount>*>(realParentIdx.internalPointer())->dataRef().addAccountId(item.id());
    MyMoneyModel::doAddItem(item, realParentIdx);
    if (item.value("PreferredAccount") == QLatin1String("Yes")) {
        addFavorite(item.id());
    }
}

void AccountsModel::doModifyItem(const MyMoneyAccount& before, const MyMoneyAccount& after)
{
    Q_UNUSED(before);
    const auto idx = indexById(after.id());
    if (idx.isValid()) {
        MyMoneyModel::doModifyItem(before, after);
        if (after.value("PreferredAccount") == QLatin1String("Yes")) {
            addFavorite(after.id());
        } else {
            removeFavorite(after.id());
        }
        if (before.reconciliationHistory() != after.reconciliationHistory()) {
            Q_EMIT reconciliationInfoChanged();
        }
        // MyMoneyModel::doModifyItem already sents this out, so maybe we can skip it here
        // Q_EMIT dataChanged(idx, index(idx.row(), columnCount(idx.parent())-1));
    }
}

void AccountsModel::doRemoveItem(const MyMoneyAccount& before)
{
    const auto idx = indexById(before.id());
    if (idx.isValid()) {
        const auto itemId = idx.data(eMyMoney::Model::IdRole).toString();
        static_cast<TreeItem<MyMoneyAccount>*>(idx.parent().internalPointer())->dataRef().removeAccountId(itemId);
        MyMoneyModel::doRemoveItem(before);
        removeFavorite(itemId);
    }
}


void AccountsModel::doReparentItem(const MyMoneyAccount& before, const MyMoneyAccount& after)
{
    QModelIndex itemIdx = indexById(before.id());
    QModelIndex oldParentIdx = itemIdx.parent();
    QModelIndex newParentIdx = indexById(after.parentAccountId());

    static_cast<TreeItem<MyMoneyAccount>*>(oldParentIdx.internalPointer())->dataRef().removeAccountId(before.id());
    static_cast<TreeItem<MyMoneyAccount>*>(itemIdx.internalPointer())->dataRef().setParentAccountId(after.parentAccountId());

    reparentRow(oldParentIdx, itemIdx.row(), newParentIdx);

    static_cast<TreeItem<MyMoneyAccount>*>(newParentIdx.internalPointer())->dataRef().addAccountId(after.id());

    setDirty(true);
}


MyMoneyModel<MyMoneyAccount>::Operation AccountsModel::undoOperation(const MyMoneyAccount& before, const MyMoneyAccount& after) const
{
    Operation op = MyMoneyModel::undoOperation(before, after);
    if (op == Modify) {
        if (before.parentAccountId() != after.parentAccountId())
            op = Reparent;
    }
    return op;
}
