/*
    SPDX-FileCopyrightText: 2026 Junie <junie@jetbrains.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "homemodel.h"
#include "kmymoneysettings.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include <KColorScheme>
#include <KLocalizedString>
#include <QDate>

QString MoneyFormatter::format(const QString& amount, const QString& accountId) const
{
    MyMoneyMoney money(amount);
    auto file = MyMoneyFile::instance();
    if (!accountId.isEmpty()) {
        try {
            auto acc = file->account(accountId);
            auto currency = file->currency(acc.currencyId());
            return MyMoneyUtils::formatMoney(money, acc, currency);
        } catch (...) {
        }
    }
    return MyMoneyUtils::formatMoney(money, MyMoneyAccount(), file->baseCurrency());
}

HomeModel::HomeModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

HomeModel::~HomeModel()
{
    clear();
}

int HomeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_sections.size();
}

QVariant HomeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_sections.size())
        return QVariant();

    HomeSection* section = m_sections.at(index.row());
    switch (role) {
    case TitleRole:
        return section->title();
    case TypeRole:
        return section->type();
    case SectionObjectRole:
        return QVariant::fromValue<QObject*>(section);
    case VisibleRole:
        return section->isVisible();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> HomeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    roles[TypeRole] = "type";
    roles[SectionObjectRole] = "sectionObject";
    roles[VisibleRole] = "visible";
    return roles;
}

void HomeModel::addSection(HomeSection* section)
{
    beginInsertRows(QModelIndex(), m_sections.size(), m_sections.size());
    m_sections.append(section);
    section->setParent(this);
    endInsertRows();

    connect(section, &HomeSection::visibilityChanged, this, [this, section]() {
        int row = m_sections.indexOf(section);
        if (row != -1) {
            QModelIndex idx = index(row);
            Q_EMIT dataChanged(idx, idx, {VisibleRole});
        }
    });

    connect(section, &HomeSection::dataChanged, this, [this, section]() {
        int row = m_sections.indexOf(section);
        if (row != -1) {
            QModelIndex idx = index(row);
            Q_EMIT dataChanged(idx, idx);
        }
    });
}

void HomeModel::clear()
{
    beginResetModel();
    qDeleteAll(m_sections);
    m_sections.clear();
    endResetModel();
}

void HomeModel::refresh()
{
    for (auto section : std::as_const(m_sections)) {
        Q_EMIT section->dataChanged();
    }
}

AccountsSection::AccountsSection(const QString& title, AccountFilter filter, QObject* parent)
    : HomeSection(title, HomeModel::Accounts, parent)
    , m_filter(filter)
{
}

QVariantList AccountsSection::accounts() const
{
    QVariantList list;
    auto file = MyMoneyFile::instance();
    QList<MyMoneyAccount> allAccounts;
    file->accountList(allAccounts);

    const auto showAllAccounts = KMyMoneySettings::showAllAccounts();
    const bool hideZeroBalanceAccounts = KMyMoneySettings::hideZeroBalanceAccountsHome() && !showAllAccounts;

    for (const auto& acc : allAccounts) {
        bool include = false;

        if (acc.isClosed() && !showAllAccounts)
            continue;

        switch (m_filter) {
        case PreferredAccounts:
            if (acc.value("PreferredAccount", false)) {
                switch (acc.accountType()) {
                case eMyMoney::Account::Type::Asset:
                case eMyMoney::Account::Type::Liability:
                case eMyMoney::Account::Type::Investment:
                case eMyMoney::Account::Type::Checkings:
                case eMyMoney::Account::Type::Savings:
                case eMyMoney::Account::Type::Cash:
                case eMyMoney::Account::Type::CreditCard:
                    include = true;
                    break;
                default:
                    break;
                }
            }
            break;
        case PaymentAccounts:
            if (!acc.value("PreferredAccount", false)) {
                switch (acc.accountType()) {
                case eMyMoney::Account::Type::Checkings:
                case eMyMoney::Account::Type::Savings:
                case eMyMoney::Account::Type::Cash:
                case eMyMoney::Account::Type::CreditCard:
                    include = true;
                    break;
                default:
                    break;
                }
            }
            break;
        case AllAccounts:
            include = true;
            break;
        }

        if (include && hideZeroBalanceAccounts && file->balance(acc.id()).isZero()) {
            include = false;
        }

        if (include) {
            try {
                MyMoneySecurity currency = file->currency(acc.currencyId());

                QVariantMap map;
                map["id"] = acc.id();
                map["name"] = acc.name();

                if (!acc.institutionId().isEmpty()) {
                    try {
                        map["institution"] = file->institution(acc.institutionId()).name();
                    } catch (const MyMoneyException&) {
                    }
                }

                MyMoneyMoney balance = file->balance(acc.id());
                map["balance"] = MyMoneyUtils::formatMoney(balance, acc, currency);
                map["color"] = (balance.isNegative() ? KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText)
                                                     : KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NormalText))
                                   .color()
                                   .name();

                if (m_showTotalBalance) {
                    MyMoneyMoney totalBalance = file->totalBalance(acc.id());
                    map["totalBalance"] = MyMoneyUtils::formatMoney(totalBalance, acc, currency);
                    map["totalColor"] = (totalBalance.isNegative() ? KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText)
                                                                   : KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NormalText))
                                            .color()
                                            .name();
                }

                map["lastReconciliation"] = acc.lastReconciliationDate().isValid() ? MyMoneyUtils::formatDate(acc.lastReconciliationDate()) : i18n("Never");

                list.append(map);
            } catch (const MyMoneyException&) {
                continue;
            }
        }
    }

    return list;
}

SchedulesSection::SchedulesSection(const QString& title, QObject* parent)
    : HomeSection(title, HomeModel::Schedules, parent)
{
}

QVariantList SchedulesSection::schedules() const
{
    QVariantList list;
    auto file = MyMoneyFile::instance();
    auto schedules = file->scheduleList();

    for (const auto& sched : schedules) {
        if (sched.isFinished())
            continue;

        QVariantMap map;
        map["id"] = sched.id();
        map["name"] = sched.name();
        map["nextDueDate"] = MyMoneyUtils::formatDate(sched.nextDueDate());
        map["occurrence"] = sched.occurrenceToString();

        try {
            auto acc = file->account(sched.account().id());
            auto currency = file->currency(acc.currencyId());
            map["amount"] = MyMoneyUtils::formatMoney(sched.transaction().splitByAccount(acc.id()).value(), acc, currency);
        } catch (...) {
        }

        list.append(map);
    }
    return list;
}

AssetsLiabilitiesSection::AssetsLiabilitiesSection(const QString& title, QObject* parent)
    : HomeSection(title, HomeModel::AssetsLiabilities, parent)
{
}

QVariantList AssetsLiabilitiesSection::assets() const
{
    QVariantList list;
    // Simplified: in reality this should sum up all asset accounts
    return list;
}

QVariantList AssetsLiabilitiesSection::liabilities() const
{
    QVariantList list;
    return list;
}

QString AssetsLiabilitiesSection::netWorth() const
{
    auto file = MyMoneyFile::instance();
    MyMoneyMoney assets = file->balance(file->asset().id());
    MyMoneyMoney liabilities = file->balance(file->liability().id());
    MyMoneyMoney netWorth = assets + liabilities; // Liabilities are usually negative
    return MyMoneyUtils::formatMoney(netWorth, MyMoneyAccount(), file->baseCurrency());
}

QString AssetsLiabilitiesSection::netWorthColor() const
{
    auto file = MyMoneyFile::instance();
    MyMoneyMoney netWorth = file->balance(file->asset().id()) + file->balance(file->liability().id());
    return (netWorth.isNegative() ? KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText)
                                  : KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NormalText))
        .color()
        .name();
}
