/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneystoragenames.h"

#include <QMap>

#include "mymoneyenums.h"

QString tagName(Tag tagID)
{
    static const QHash<Tag, QString> tagNames {
        {Tag::Institutions, QStringLiteral("INSTITUTIONS")},
        {Tag::Payees,       QStringLiteral("PAYEES")},
        {Tag::CostCenters,  QStringLiteral("COSTCENTERS")},
        {Tag::Tags,         QStringLiteral("TAGS")},
        {Tag::Accounts,     QStringLiteral("ACCOUNTS")},
        {Tag::Transactions, QStringLiteral("TRANSACTIONS")},
        {Tag::Schedules,    QStringLiteral("SCHEDULES")},
        {Tag::Securities,   QStringLiteral("SECURITIES")},
        {Tag::Currencies,   QStringLiteral("CURRENCIES")},
        {Tag::Prices,       QStringLiteral("PRICES")},
        {Tag::Reports,      QStringLiteral("REPORTS")},
        {Tag::Budgets,      QStringLiteral("BUDGETS")},
        {Tag::OnlineJobs,   QStringLiteral("ONLINEJOBS")},
        {Tag::KMMFile,      QStringLiteral("KMYMONEY-FILE")},
        {Tag::FileInfo,     QStringLiteral("FILEINFO")},
        {Tag::User,         QStringLiteral("USER")}
    };
    return tagNames.value(tagID);
}

uint qHash(const Tag key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}

QString nodeName(Node nodeID)
{
    static const QHash<Node, QString> nodeNames {
        {Node::Institution,   QStringLiteral("INSTITUTION")},
        {Node::Payee,         QStringLiteral("PAYEE")},
        {Node::CostCenter,    QStringLiteral("COSTCENTER")},
        {Node::Tag,           QStringLiteral("TAG")},
        {Node::Account,       QStringLiteral("ACCOUNT")},
        {Node::Transaction,   QStringLiteral("TRANSACTION")},
        {Node::Split,         QStringLiteral("SPLIT")},
        {Node::ScheduleTX,    QStringLiteral("SCHEDULED_TX")},
        {Node::Security,      QStringLiteral("SECURITY")},
        {Node::Currency,      QStringLiteral("CURRENCY")},
        {Node::Price,         QStringLiteral("PRICE")},
        {Node::PricePair,     QStringLiteral("PRICEPAIR")},
        {Node::Report,        QStringLiteral("REPORT")},
        {Node::Budget,        QStringLiteral("BUDGET")},
        {Node::OnlineJob,     QStringLiteral("ONLINEJOB")},
        {Node::KeyValuePairs, QStringLiteral("KEYVALUEPAIRS")},
        {Node::Equity,        QStringLiteral("EQUITY")},
    };
    return nodeNames.value(nodeID);
}

uint qHash(const Node key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}

namespace Element {
uint qHash(const General key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
uint qHash(const Transaction key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
uint qHash(const Account key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
uint qHash(const Payee key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
uint qHash(const KVP key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
uint qHash(const Institution key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
uint qHash(const Schedule key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
uint qHash(const OnlineJob key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
}

namespace Attribute {
uint qHash(const General key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const Transaction key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const Account key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const Payee key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const Tag key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const Security key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const KVP key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const Institution key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const Schedule key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
uint qHash(const OnlineJob key, uint seed) {
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}
}

QString elementName(Element::General elementID)
{
    static const QMap<Element::General, QString> elementNames {
        {Element::General::Address,           QStringLiteral("ADDRESS")},
        {Element::General::CreationDate,      QStringLiteral("CREATION_DATE")},
        {Element::General::LastModifiedDate,  QStringLiteral("LAST_MODIFIED_DATE")},
        {Element::General::Version,           QStringLiteral("VERSION")},
        {Element::General::FixVersion,        QStringLiteral("FIXVERSION")},
        {Element::General::Pair,              QStringLiteral("PAIR")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::General attributeID)
{
    static const QMap<Attribute::General, QString> attributeNames {
        {Attribute::General::ID,        QStringLiteral("id")},
        {Attribute::General::Date,      QStringLiteral("date")},
        {Attribute::General::Count,     QStringLiteral("count")},
        {Attribute::General::From,      QStringLiteral("from")},
        {Attribute::General::To,        QStringLiteral("to")},
        {Attribute::General::Source,    QStringLiteral("source")},
        {Attribute::General::Key,       QStringLiteral("key")},
        {Attribute::General::Value,     QStringLiteral("value")},
        {Attribute::General::Price,     QStringLiteral("price")},
        {Attribute::General::Name,      QStringLiteral("name")},
        {Attribute::General::Email,     QStringLiteral("email")},
        {Attribute::General::Country,   QStringLiteral("county")},
        {Attribute::General::City,      QStringLiteral("city")},
        {Attribute::General::ZipCode,   QStringLiteral("zipcode")},
        {Attribute::General::Street,    QStringLiteral("street")},
        {Attribute::General::Telephone, QStringLiteral("telephone")}
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::Transaction elementID)
{
    static const QMap<Element::Transaction, QString> elementNames {
        {Element::Transaction::Split,  QStringLiteral("SPLIT")},
        {Element::Transaction::Splits, QStringLiteral("SPLITS")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::Transaction attributeID)
{
    static const QMap<Attribute::Transaction, QString> attributeNames {
        {Attribute::Transaction::Name,       QStringLiteral("name")},
        {Attribute::Transaction::Type,       QStringLiteral("type")},
        {Attribute::Transaction::PostDate,   QStringLiteral("postdate")},
        {Attribute::Transaction::Memo,       QStringLiteral("memo")},
        {Attribute::Transaction::EntryDate,  QStringLiteral("entrydate")},
        {Attribute::Transaction::Commodity,  QStringLiteral("commodity")},
        {Attribute::Transaction::BankID,     QStringLiteral("bankid")},
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::Split elementID)
{
    static const QMap<Element::Split, QString> elementNames {
        {Element::Split::Split,          QStringLiteral("SPLIT")},
        {Element::Split::Tag,            QStringLiteral("TAG")},
        {Element::Split::Match,          QStringLiteral("MATCH")},
        {Element::Split::Container,      QStringLiteral("CONTAINER")},
        {Element::Split::KeyValuePairs,  QStringLiteral("KEYVALUEPAIRS")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::Split attributeID)
{
    static const QMap<Attribute::Split, QString> attributeNames {
        {Attribute::Split::ID,             QStringLiteral("id")},
        {Attribute::Split::BankID,         QStringLiteral("bankid")},
        {Attribute::Split::Account,        QStringLiteral("account")},
        {Attribute::Split::Payee,          QStringLiteral("payee")},
        {Attribute::Split::Tag,            QStringLiteral("tag")},
        {Attribute::Split::Number,         QStringLiteral("number")},
        {Attribute::Split::Action,         QStringLiteral("action")},
        {Attribute::Split::Value,          QStringLiteral("value")},
        {Attribute::Split::Shares,         QStringLiteral("shares")},
        {Attribute::Split::Price,          QStringLiteral("price")},
        {Attribute::Split::Memo,           QStringLiteral("memo")},
        {Attribute::Split::CostCenter,     QStringLiteral("costcenter")},
        {Attribute::Split::ReconcileDate,  QStringLiteral("reconciledate")},
        {Attribute::Split::ReconcileFlag,  QStringLiteral("reconcileflag")},
        {Attribute::Split::KMMatchedTx,    QStringLiteral("kmm-matched-tx")}
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::Account elementID)
{
    static const QMap<Element::Account, QString> elementNames {
        {Element::Account::SubAccount,     QStringLiteral("SUBACCOUNT")},
        {Element::Account::SubAccounts,    QStringLiteral("SUBACCOUNTS")},
        {Element::Account::OnlineBanking,  QStringLiteral("ONLINEBANKING")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::Account attributeID)
{
    static const QHash<Attribute::Account, QString> attributeNames {
        {Attribute::Account::ID,             QStringLiteral("id")},
        {Attribute::Account::Name,           QStringLiteral("name")},
        {Attribute::Account::Type,           QStringLiteral("type")},
        {Attribute::Account::ParentAccount,  QStringLiteral("parentaccount")},
        {Attribute::Account::LastReconciled, QStringLiteral("lastreconciled")},
        {Attribute::Account::LastModified,   QStringLiteral("lastmodified")},
        {Attribute::Account::Institution,    QStringLiteral("institution")},
        {Attribute::Account::Opened,         QStringLiteral("opened")},
        {Attribute::Account::Number,         QStringLiteral("number")},
        {Attribute::Account::Type,           QStringLiteral("type")},
        {Attribute::Account::Description,    QStringLiteral("description")},
        {Attribute::Account::Currency,       QStringLiteral("currency")},
        {Attribute::Account::OpeningBalance, QStringLiteral("openingbalance")},
        {Attribute::Account::IBAN,           QStringLiteral("iban")},
        {Attribute::Account::BIC,            QStringLiteral("bic")},
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::Payee elementID)
{
    static const QMap<Element::Payee, QString> elementNames {
        {Element::Payee::Address,         QStringLiteral("ADDRESS")},
        {Element::Payee::Identifier,      QStringLiteral("payeeIdentifier")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::Payee attributeID)
{
    static const QMap<Attribute::Payee, QString> attributeNames {
        {Attribute::Payee::ID,               QStringLiteral("id")},
        {Attribute::Payee::Name,             QStringLiteral("name")},
        {Attribute::Payee::Type,             QStringLiteral("type")},
        {Attribute::Payee::Reference,        QStringLiteral("reference")},
        {Attribute::Payee::Notes,            QStringLiteral("notes")},
        {Attribute::Payee::MatchingEnabled,  QStringLiteral("matchingenabled")},
        {Attribute::Payee::UsingMatchKey,    QStringLiteral("usingmatchkey")},
        {Attribute::Payee::MatchIgnoreCase,  QStringLiteral("matchignorecase")},
        {Attribute::Payee::MatchKey,         QStringLiteral("matchkey")},
        {Attribute::Payee::DefaultAccountID, QStringLiteral("defaultaccountid")},
        {Attribute::Payee::Street,           QStringLiteral("street")},
        {Attribute::Payee::City,             QStringLiteral("city")},
        {Attribute::Payee::PostCode,         QStringLiteral("postcode")},
        {Attribute::Payee::Email,            QStringLiteral("email")},
        {Attribute::Payee::State,            QStringLiteral("state")},
        {Attribute::Payee::Telephone,        QStringLiteral("telephone")},
        {Attribute::Payee::IBAN,             QStringLiteral("iban")},
        {Attribute::Payee::BIC,              QStringLiteral("bic")},
        {Attribute::Payee::OwnerVer1,        QStringLiteral("ownerName")}, // for IBANBIC number
        {Attribute::Payee::OwnerVer2,        QStringLiteral("ownername")}, // for NationaAccount number
        {Attribute::Payee::AccountNumber,    QStringLiteral("accountnumber")},
        {Attribute::Payee::BankCode,         QStringLiteral("bankcode")},
        {Attribute::Payee::Country,          QStringLiteral("country")},
    };
    return attributeNames.value(attributeID);
}

QString attributeName(Attribute::Tag attributeID)
{
    static const QMap<Attribute::Tag, QString> attributeNames {
        {Attribute::Tag::Name,     QStringLiteral("name")},
        {Attribute::Tag::Type,     QStringLiteral("type")},
        {Attribute::Tag::TagColor, QStringLiteral("tagcolor")},
        {Attribute::Tag::Closed,   QStringLiteral("closed")},
        {Attribute::Tag::Notes,    QStringLiteral("notes")},
    };
    return attributeNames.value(attributeID);
}

QString attributeName(Attribute::Security attributeID)
{
    static const QMap<Attribute::Security, QString> attributeNames {
        {Attribute::Security::Name,             QStringLiteral("name")},
        {Attribute::Security::Symbol,           QStringLiteral("symbol")},
        {Attribute::Security::Type,             QStringLiteral("type")},
        {Attribute::Security::RoundingMethod,   QStringLiteral("rounding-method")},
        {Attribute::Security::SAF,              QStringLiteral("saf")},
        {Attribute::Security::PP,               QStringLiteral("pp")},
        {Attribute::Security::SCF,              QStringLiteral("scf")},
        {Attribute::Security::TradingCurrency,  QStringLiteral("trading-currency")},
        {Attribute::Security::TradingMarket,    QStringLiteral("trading-market")}
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::KVP elementID)
{
    static const QMap<Element::KVP, QString> elementNames {
        {Element::KVP::Pair, QStringLiteral("PAIR")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::KVP attributeID)
{
    static const QMap<Attribute::KVP, QString> attributeNames {
        {Attribute::KVP::Key,   QStringLiteral("key")},
        {Attribute::KVP::Value, QStringLiteral("value")}
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::Institution elementID)
{
    static const QMap<Element::Institution, QString> elementNames {
        {Element::Institution::AccountID,  QStringLiteral("ACCOUNTID")},
        {Element::Institution::AccountIDS, QStringLiteral("ACCOUNTIDS")},
        {Element::Institution::Address,    QStringLiteral("ADDRESS")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::Institution attributeID)
{
    static const QMap<Attribute::Institution, QString> attributeNames {
        {Attribute::Institution::ID,         QStringLiteral("id")},
        {Attribute::Institution::Name,       QStringLiteral("name")},
        {Attribute::Institution::Manager,    QStringLiteral("manager")},
        {Attribute::Institution::SortCode,   QStringLiteral("sortcode")},
        {Attribute::Institution::Street,     QStringLiteral("street")},
        {Attribute::Institution::City,       QStringLiteral("city")},
        {Attribute::Institution::Zip,        QStringLiteral("zip")},
        {Attribute::Institution::Telephone,  QStringLiteral("telephone")}
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::Schedule elementID)
{
    static const QMap<Element::Schedule, QString> elementNames {
        {Element::Schedule::Payment,  QStringLiteral("PAYMENT")},
        {Element::Schedule::Payments, QStringLiteral("PAYMENTS")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::Schedule attributeID)
{
    static const QMap<Attribute::Schedule, QString> attributeNames {
        {Attribute::Schedule::Name,                 QStringLiteral("name")},
        {Attribute::Schedule::Type,                 QStringLiteral("type")},
        {Attribute::Schedule::Occurrence,           QStringLiteral("occurence")}, // krazy:exclude=spelling
        {Attribute::Schedule::OccurrenceMultiplier, QStringLiteral("occurenceMultiplier")}, // krazy:exclude=spelling
        {Attribute::Schedule::PaymentType,          QStringLiteral("paymentType")},
        {Attribute::Schedule::Fixed,                QStringLiteral("fixed")},
        {Attribute::Schedule::AutoEnter,            QStringLiteral("autoEnter")},
        {Attribute::Schedule::LastPayment,          QStringLiteral("lastPayment")},
        {Attribute::Schedule::WeekendOption,        QStringLiteral("weekendOption")},
        {Attribute::Schedule::Date,                 QStringLiteral("date")},
        {Attribute::Schedule::StartDate,            QStringLiteral("startDate")},
        {Attribute::Schedule::EndDate,              QStringLiteral("endDate")},
        {Attribute::Schedule::LastDayInMonth,       QStringLiteral("lastDayInMonth")}
    };
    return attributeNames.value(attributeID);
}

QString elementName(Element::OnlineJob elementID)
{
    static const QMap<Element::OnlineJob, QString> elementNames {
        {Element::OnlineJob::OnlineTask, QStringLiteral("onlineTask")}
    };
    return elementNames.value(elementID);
}

QString attributeName(Attribute::OnlineJob attributeID)
{
    static const QMap<Attribute::OnlineJob, QString> attributeNames {
        {Attribute::OnlineJob::Send,             QStringLiteral("send")},
        {Attribute::OnlineJob::BankAnswerDate,   QStringLiteral("bankAnswerDate")},
        {Attribute::OnlineJob::BankAnswerState,  QStringLiteral("bankAnswerState")},
        {Attribute::OnlineJob::IID,              QStringLiteral("iid")},
        {Attribute::OnlineJob::AbortedByUser,    QStringLiteral("abortedByUser")},
        {Attribute::OnlineJob::AcceptedByBank,   QStringLiteral("acceptedByBank")},
        {Attribute::OnlineJob::RejectedByBank,   QStringLiteral("rejectedByBank")},
        {Attribute::OnlineJob::SendingError,     QStringLiteral("sendingError")},
    };
    return attributeNames.value(attributeID);
}

QString attributeName(Attribute::CostCenter attributeID)
{
    static const QMap<Attribute::CostCenter, QString> attributeNames {
        {Attribute::CostCenter::Name, QStringLiteral("name")},
    };
    return attributeNames.value(attributeID);
}
