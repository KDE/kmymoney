/*
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2012 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reportaccount.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.  This
// is a minor problem because we use these terms when rendering to HTML,
// and a more major problem because we need it to translate account types
// (e.g. eMyMoney::Account::Type::Checkings) into their text representation.  We also
// use that text representation in the core data structure of the report. (Ace)

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "reportdebug.h"
#include "mymoneyenums.h"

namespace reports
{

ReportAccount::ReportAccount()
    : m_securityCache(new QMap<QString, MyMoneySecurity>)
{
}

ReportAccount::ReportAccount(const ReportAccount& copy)
    : MyMoneyAccount(copy)
    , m_nameHierarchy(copy.m_nameHierarchy)
    , m_tradingCurrencyId(copy.m_tradingCurrencyId)
    , m_deepcurrency(copy.m_deepcurrency)
    , m_securityCache(new QMap<QString, MyMoneySecurity>(*copy.m_securityCache))
{
    // NOTE: I implemented the copy constructor solely for debugging reasons

    DEBUG_ENTER(Q_FUNC_INFO);
}

ReportAccount::ReportAccount(const QString& accountid)
    : MyMoneyAccount(MyMoneyFile::instance()->account(accountid))
    , m_securityCache(new QMap<QString, MyMoneySecurity>)
{
    DEBUG_ENTER(Q_FUNC_INFO);
    DEBUG_OUTPUT(QString("Account %1").arg(accountid));
    calculateAccountHierarchy();
}

ReportAccount::ReportAccount(const MyMoneyAccount& account)
    : MyMoneyAccount(account)
    , m_securityCache(new QMap<QString, MyMoneySecurity>)
{
    DEBUG_ENTER(Q_FUNC_INFO);
    DEBUG_OUTPUT(QString("Account %1").arg(account.id()));
    calculateAccountHierarchy();
}

ReportAccount::~ReportAccount() noexcept
{
    delete m_securityCache;
}

void ReportAccount::calculateAccountHierarchy()
{
    DEBUG_ENTER(Q_FUNC_INFO);

    MyMoneyFile* file = MyMoneyFile::instance();
    QString resultid = id();
    QString parentid = parentAccountId();

#ifdef DEBUG_HIDE_SENSITIVE
    m_nameHierarchy.prepend(file->account(resultid).id());
#else
    m_nameHierarchy.prepend(file->account(resultid).name());
#endif
    while (!parentid.isEmpty() && !file->isStandardAccount(parentid)) {
        // take on the identity of our parent
        resultid = parentid;

        // and try again
        parentid = file->account(resultid).parentAccountId();
#ifdef DEBUG_HIDE_SENSITIVE
        m_nameHierarchy.prepend(file->account(resultid).id());
#else
        m_nameHierarchy.prepend(file->account(resultid).name());
#endif
    }
    m_tradingCurrencyId = tradingCurrencyId();

    // First, get the deep currency
    m_deepcurrency = file->security(currencyId());
    if (!m_deepcurrency.isCurrency()) {
        m_deepcurrency = file->security(m_deepcurrency.tradingCurrency());
    }
}

MyMoneyMoney ReportAccount::deepCurrencyPrice(const QDate& date, bool exactDate) const
{
    DEBUG_ENTER(Q_FUNC_INFO);

    MyMoneyMoney result(1, 1);
    MyMoneyFile* file = MyMoneyFile::instance();

    if (!m_securityCache->contains(currencyId())) {
        m_securityCache->insert(currencyId(), file->security(currencyId()));
    }

    MyMoneySecurity undersecurity(m_securityCache->find(currencyId()).value());
    if (! undersecurity.isCurrency()) {
        const MyMoneyPrice &price = file->price(undersecurity.id(), undersecurity.tradingCurrency(), date, exactDate);
        if (price.isValid()) {
            result = price.rate(undersecurity.tradingCurrency());

            DEBUG_OUTPUT(QString("Converting under %1 to deep %2, price on %3 is %4")
                         .arg(undersecurity.name())
                         .arg(file->security(undersecurity.tradingCurrency()).name())
                         .arg(date.toString(Qt::ISODate))
                         .arg(result.toDouble()));
        } else {
            DEBUG_OUTPUT(QString("No price to convert under %1 to deep %2 on %3")
                         .arg(undersecurity.name())
                         .arg(file->security(undersecurity.tradingCurrency()).name())
                         .arg(date.toString(Qt::ISODate)));
            result = MyMoneyMoney();
        }
    }

    return result;
}

bool ReportAccount::isForeignCurrency() const
{
    return (m_tradingCurrencyId != MyMoneyFile::instance()->baseCurrency().id());
}

MyMoneyMoney ReportAccount::baseCurrencyPrice(const QDate& date, bool exactDate) const
{
    // Note that whether or not the user chooses to convert to base currency, all the values
    // for a given account/category are converted to the currency for THAT account/category
    // The "Convert to base currency" tells the report to convert from the account/category
    // currency to the file's base currency.
    //
    // An example where this matters is if Category 'C' and account 'U' are in USD, but
    // Account 'J' is in JPY.  Say there are two transactions, one is US$100 from U to C,
    // the other is JPY10,000 from J to C.  Given a JPY price of USD$0.01, this means
    // C will show a balance of $200 NO MATTER WHAT the user chooses for 'convert to base
    // currency.  This confused me for a while, which is why I wrote this comment.
    //    --acejones

    DEBUG_ENTER(Q_FUNC_INFO);

    MyMoneyMoney result(1, 1);
    MyMoneyFile* file = MyMoneyFile::instance();

    if (isForeignCurrency()) {
        result = foreignCurrencyPrice(file->baseCurrency().id(), date, exactDate);
    }

    return result;
}

MyMoneyMoney ReportAccount::foreignCurrencyPrice(const QString foreignCurrency, const QDate& date, bool exactDate) const
{
    DEBUG_ENTER(Q_FUNC_INFO);

    MyMoneyMoney result(1, 1);
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneySecurity security = file->security(foreignCurrency);

    //check whether it is a currency or a commodity. In the latter case case, get the trading currency
    QString tradingCurrency;
    if (security.isCurrency()) {
        tradingCurrency = foreignCurrency;
    } else {
        tradingCurrency = security.tradingCurrency();
    }

    //It makes no sense to get the price if both currencies are the same
    if (currency().id() != tradingCurrency) {
        const MyMoneyPrice &price = file->price(currency().id(), tradingCurrency, date, exactDate);

        if (price.isValid()) {
            result = price.rate(tradingCurrency);
            DEBUG_OUTPUT(QString("Converting deep %1 to currency %2, price on %3 is %4")
                         .arg(file->currency(currency().id()).name())
                         .arg(file->currency(foreignCurrency).name())
                         .arg(date.toString())
                         .arg(result.toDouble()));
        } else {
            DEBUG_OUTPUT(QString("No price to convert deep %1 to currency %2 on %3")
                         .arg(file->currency(currency().id()).name())
                         .arg(file->currency(foreignCurrency).name())
                         .arg(date.toString()));
        }
    }
    return result;
}

/**
  * Fetch the trading currency of this account's currency
  *
  * @return The account's currency trading currency
  */
MyMoneySecurity ReportAccount::currency() const
{
    return m_deepcurrency;
}

ReportAccount& ReportAccount::operator=(const ReportAccount& right)
{
    m_nameHierarchy = right.m_nameHierarchy;
    m_tradingCurrencyId = right.m_tradingCurrencyId;
    m_deepcurrency = right.m_deepcurrency;
    *m_securityCache = *right.m_securityCache;
    return *this;
}

bool ReportAccount::operator<(const ReportAccount& second) const
{
//   DEBUG_ENTER(Q_FUNC_INFO);

    bool result = false;
    bool haveresult = false;
    QStringList::const_iterator it_first = m_nameHierarchy.begin();
    QStringList::const_iterator it_second = second.m_nameHierarchy.begin();
    while (it_first != m_nameHierarchy.end()) {
        // The first string is longer than the second, but otherwise identical
        if (it_second == second.m_nameHierarchy.end()) {
            result = false;
            haveresult = true;
            break;
        }

        if ((*it_first) < (*it_second)) {
            result = true;
            haveresult = true;
            break;
        } else if ((*it_first) > (*it_second)) {
            result = false;
            haveresult = true;
            break;
        }

        ++it_first;
        ++it_second;
    }

    // The second string is longer than the first, but otherwise identical
    if (!haveresult && (it_second != second.m_nameHierarchy.end()))
        result = true;

//   DEBUG_OUTPUT(QString("%1 < %2 is %3").arg(debugName(),second.debugName()).arg(result));
    return result;
}

/**
  * The name of only this account.  No matter how deep the hierarchy, this
  * method only returns the last name in the list, which is the engine name
  * of this account.
  *
  * @return QString The account's name
  */
QString ReportAccount::name() const
{
    return m_nameHierarchy.back();
}

// MyMoneyAccount:fullHierarchyDebug()
QString ReportAccount::debugName() const
{
    return m_nameHierarchy.join("|");
}

// MyMoneyAccount:fullHierarchy()
QString ReportAccount::fullName() const
{
    return m_nameHierarchy.join(": ");
}

// MyMoneyAccount:isTopCategory()
bool ReportAccount::isTopLevel() const
{
    return (m_nameHierarchy.size() == 1);
}

// MyMoneyAccount:hierarchyDepth()
unsigned ReportAccount::hierarchyDepth() const
{
    return (m_nameHierarchy.size());
}

ReportAccount ReportAccount::parent() const
{
    return ReportAccount(parentAccountId());
}

ReportAccount ReportAccount::topParent() const
{
    DEBUG_ENTER(Q_FUNC_INFO);

    MyMoneyFile* file = MyMoneyFile::instance();
    QString resultid = id();
    QString parentid = parentAccountId();

    while (!parentid.isEmpty() && !file->isStandardAccount(parentid)) {
        // take on the identity of our parent
        resultid = parentid;

        // and try again
        parentid = file->account(resultid).parentAccountId();
    }

    return ReportAccount(resultid);
}

QString ReportAccount::topParentName() const
{
    return m_nameHierarchy.first();
}

QString ReportAccount::institutionId() const
{
    DEBUG_ENTER(Q_FUNC_INFO);

    MyMoneyFile* file = MyMoneyFile::instance();
    QString resultid = MyMoneyAccount::institutionId();
    QString parentid = parentAccountId();

    while (resultid.isEmpty() && !parentid.isEmpty() && !file->isStandardAccount(parentid)) {
        const auto account = file->account(parentid);
        resultid = account.institutionId();
        // and try again
        parentid = account.parentAccountId();
    }
    return resultid;
}

}  // end namespace reports
