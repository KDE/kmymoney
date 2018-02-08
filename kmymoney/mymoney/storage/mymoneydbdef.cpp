/***************************************************************************
                          mymoneydbdef.h
                          -------------------
    begin                : 20 February 2010
    copyright            : (C) 2010 by Fernando Vilas
    email                : tonybloom@users.sourceforge.net
                         : Fernando Vilas <fvilas@iname.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneydbdef.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneydbdriver.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneystoragemgr.h"

#include <alkimia/alkvalue.h>

//***************** THE CURRENT VERSION OF THE DATABASE LAYOUT ****************
unsigned int MyMoneyDbDef::m_currentVersion = 12;

// ************************* Build table descriptions ****************************
MyMoneyDbDef::MyMoneyDbDef()
{
  FileInfo();
  PluginInfo();
  Institutions();
  Payees();
  PayeesPayeeIdentifier();
  Tags();
  TagSplits(); // a table to bind tags and splits
  Accounts();
  AccountsPayeeIdentifier();
  Transactions();
  Splits();
  KeyValuePairs();
  Schedules();
  SchedulePaymentHistory();
  Securities();
  Prices();
  Currencies();
  Reports();
  Budgets();
  Balances();
  OnlineJobs();
  PayeeIdentifier();
  CostCenter();
}

/* PRIMARYKEY - these fields combine to form a unique key field on which the db will create an index
   NOTNULL - this field should never be null
   UNSIGNED - for numeric types, indicates the field is UNSIGNED
   ?ISKEY - where there is no primary key, these fields can be used to uniquely identify a record
  Default is that a field is not a part of a primary key, nullable, and if numeric, signed */

static const bool PRIMARYKEY = true;
static const bool NOTNULL = true;
static const bool UNSIGNED = false;

#define appendField(a) fields.append(QExplicitlySharedDataPointer<MyMoneyDbColumn>(new a))
void MyMoneyDbDef::FileInfo()
{
  QList< QExplicitlySharedDataPointer<MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("version", "varchar(16)"));
  appendField(MyMoneyDbColumn("created", "date"));
  appendField(MyMoneyDbColumn("lastModified", "date"));
  appendField(MyMoneyDbColumn("baseCurrency", "char(3)"));
  appendField(MyMoneyDbIntColumn("institutions", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("accounts", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("payees", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("tags", MyMoneyDbIntColumn::BIG, UNSIGNED, false, false, 7));
  appendField(MyMoneyDbIntColumn("transactions", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("splits", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("securities", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("prices", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("currencies", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("schedules", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("reports", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("kvps", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbColumn("dateRangeStart", "date"));
  appendField(MyMoneyDbColumn("dateRangeEnd", "date"));
  appendField(MyMoneyDbIntColumn("hiInstitutionId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("hiPayeeId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("hiTagId", MyMoneyDbIntColumn::BIG, UNSIGNED, false, false, 7));
  appendField(MyMoneyDbIntColumn("hiAccountId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("hiTransactionId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("hiScheduleId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("hiSecurityId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbIntColumn("hiReportId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  appendField(MyMoneyDbColumn("encryptData", "varchar(255)"));
  appendField(MyMoneyDbColumn("updateInProgress", "char(1)"));
  appendField(MyMoneyDbIntColumn("budgets", MyMoneyDbIntColumn::BIG, UNSIGNED, false, false, 1));
  appendField(MyMoneyDbIntColumn("hiBudgetId", MyMoneyDbIntColumn::BIG, UNSIGNED, false, false, 1));
  appendField(MyMoneyDbIntColumn("hiOnlineJobId", MyMoneyDbIntColumn::BIG, UNSIGNED, false, false, 8));
  appendField(MyMoneyDbIntColumn("hiPayeeIdentifierId", MyMoneyDbIntColumn::BIG, UNSIGNED, false, false, 8));
  appendField(MyMoneyDbColumn("logonUser", "varchar(255)", false, false, 1));
  appendField(MyMoneyDbDatetimeColumn("logonAt", false, false, 1));
  appendField(MyMoneyDbIntColumn("fixLevel",
                                 MyMoneyDbIntColumn::MEDIUM, UNSIGNED, false, false, 6));
  MyMoneyDbTable t("kmmFileInfo", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Institutions()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbTextColumn("name", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  appendField(MyMoneyDbTextColumn("manager"));
  appendField(MyMoneyDbTextColumn("routingCode"));
  appendField(MyMoneyDbTextColumn("addressStreet"));
  appendField(MyMoneyDbTextColumn("addressCity"));
  appendField(MyMoneyDbTextColumn("addressZipcode"));
  appendField(MyMoneyDbTextColumn("telephone"));
  MyMoneyDbTable t("kmmInstitutions", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Payees()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)",  PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbTextColumn("name"));
  appendField(MyMoneyDbTextColumn("reference"));
  appendField(MyMoneyDbTextColumn("email"));
  appendField(MyMoneyDbTextColumn("addressStreet"));
  appendField(MyMoneyDbTextColumn("addressCity"));
  appendField(MyMoneyDbTextColumn("addressZipcode"));
  appendField(MyMoneyDbTextColumn("addressState"));
  appendField(MyMoneyDbTextColumn("telephone"));
  appendField(MyMoneyDbTextColumn("notes", MyMoneyDbTextColumn::LONG, false, false, 5));
  appendField(MyMoneyDbColumn("defaultAccountId", "varchar(32)", false, false, 5));
  appendField(MyMoneyDbIntColumn("matchData", MyMoneyDbIntColumn::TINY, UNSIGNED, false, false, 5));
  appendField(MyMoneyDbColumn("matchIgnoreCase", "char(1)", false, false, 5));
  appendField(MyMoneyDbTextColumn("matchKeys", MyMoneyDbTextColumn::MEDIUM, false, false, 5));
  MyMoneyDbTable t("kmmPayees", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::PayeesPayeeIdentifier()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("payeeId", "varchar(32)",  PRIMARYKEY, NOTNULL, 8));
  appendField(MyMoneyDbIntColumn("\"order\"", MyMoneyDbIntColumn::SMALL, UNSIGNED, PRIMARYKEY, NOTNULL, 8, 9));
  appendField(MyMoneyDbIntColumn("userOrder", MyMoneyDbIntColumn::SMALL, UNSIGNED, PRIMARYKEY, NOTNULL, 10));
  appendField(MyMoneyDbColumn("identifierId", "varchar(32)", false, NOTNULL, 8));
  MyMoneyDbTable t("kmmPayeesPayeeIdentifier", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Tags()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)",  PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbTextColumn("name"));
  appendField(MyMoneyDbColumn("closed", "char(1)", false, false, 5));
  appendField(MyMoneyDbTextColumn("notes", MyMoneyDbTextColumn::LONG, false, false, 5));
  appendField(MyMoneyDbTextColumn("tagColor"));
  MyMoneyDbTable t("kmmTags", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::TagSplits()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("transactionId", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("tagId", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbIntColumn("splitId", MyMoneyDbIntColumn::SMALL, UNSIGNED, PRIMARYKEY, NOTNULL));
  MyMoneyDbTable t("kmmTagSplits", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Accounts()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)",  PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("institutionId", "varchar(32)"));
  appendField(MyMoneyDbColumn("parentId", "varchar(32)"));
  appendField(MyMoneyDbDatetimeColumn("lastReconciled"));
  appendField(MyMoneyDbDatetimeColumn("lastModified"));
  appendField(MyMoneyDbColumn("openingDate", "date"));
  appendField(MyMoneyDbTextColumn("accountNumber"));
  appendField(MyMoneyDbColumn("accountType", "varchar(16)", false, NOTNULL));
  appendField(MyMoneyDbTextColumn("accountTypeString"));
  appendField(MyMoneyDbColumn("isStockAccount", "char(1)"));
  appendField(MyMoneyDbTextColumn("accountName"));
  appendField(MyMoneyDbTextColumn("description"));
  appendField(MyMoneyDbColumn("currencyId", "varchar(32)"));
  appendField(MyMoneyDbTextColumn("balance"));
  appendField(MyMoneyDbTextColumn("balanceFormatted"));
  appendField(MyMoneyDbIntColumn("transactionCount", MyMoneyDbIntColumn::BIG, UNSIGNED, false, false, 1));
  MyMoneyDbTable t("kmmAccounts", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::AccountsPayeeIdentifier()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("accountId", "varchar(32)",  PRIMARYKEY, NOTNULL, 8));
  appendField(MyMoneyDbIntColumn("\"order\"", MyMoneyDbIntColumn::SMALL, UNSIGNED, PRIMARYKEY, NOTNULL, 8, 9));
  appendField(MyMoneyDbIntColumn("userOrder", MyMoneyDbIntColumn::SMALL, UNSIGNED, PRIMARYKEY, NOTNULL, 10));
  appendField(MyMoneyDbColumn("identifierId", "varchar(32)", false, NOTNULL, 8));
  MyMoneyDbTable t("kmmAccountsPayeeIdentifier", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Transactions()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("txType", "char(1)"));
  appendField(MyMoneyDbDatetimeColumn("postDate"));
  appendField(MyMoneyDbTextColumn("memo"));
  appendField(MyMoneyDbDatetimeColumn("entryDate"));
  appendField(MyMoneyDbColumn("currencyId", "char(3)"));
  appendField(MyMoneyDbTextColumn("bankId"));
  MyMoneyDbTable t("kmmTransactions", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Splits()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("transactionId", "varchar(32)",  PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("txType", "char(1)"));
  appendField(MyMoneyDbIntColumn("splitId", MyMoneyDbIntColumn::SMALL, UNSIGNED,  PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("payeeId", "varchar(32)"));
  appendField(MyMoneyDbDatetimeColumn("reconcileDate"));
  appendField(MyMoneyDbColumn("action", "varchar(16)"));
  appendField(MyMoneyDbColumn("reconcileFlag", "char(1)"));
  appendField(MyMoneyDbTextColumn("value", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  appendField(MyMoneyDbColumn("valueFormatted", "text"));
  appendField(MyMoneyDbTextColumn("shares", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  appendField(MyMoneyDbTextColumn("sharesFormatted"));
  appendField(MyMoneyDbTextColumn("price", MyMoneyDbTextColumn::NORMAL, false, false, 2));
  appendField(MyMoneyDbTextColumn("priceFormatted", MyMoneyDbTextColumn::MEDIUM, false, false, 2));
  appendField(MyMoneyDbTextColumn("memo"));
  appendField(MyMoneyDbColumn("accountId", "varchar(32)", false, NOTNULL));
  appendField(MyMoneyDbColumn("costCenterId", "varchar(32)", false, false, 9));
  appendField(MyMoneyDbColumn("checkNumber", "varchar(32)"));
  appendField(MyMoneyDbDatetimeColumn("postDate", false, false, 1));
  appendField(MyMoneyDbTextColumn("bankId", MyMoneyDbTextColumn::MEDIUM, false, false, 5));
  MyMoneyDbTable t("kmmSplits", fields);
  QStringList list;
  list << "accountId" << "txType";
  t.addIndex("kmmSplitsaccount_type", list, false);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::KeyValuePairs()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("kvpType", "varchar(16)", false, NOTNULL));
  appendField(MyMoneyDbColumn("kvpId", "varchar(32)"));
  appendField(MyMoneyDbColumn("kvpKey", "varchar(255)", false, NOTNULL));
  appendField(MyMoneyDbTextColumn("kvpData"));
  MyMoneyDbTable t("kmmKeyValuePairs", fields);
  QStringList list;
  list << "kvpType" << "kvpId";
  t.addIndex("type_id", list, false);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Schedules()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbTextColumn("name", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  appendField(MyMoneyDbIntColumn("type", MyMoneyDbIntColumn::TINY, UNSIGNED, false, NOTNULL));
  appendField(MyMoneyDbTextColumn("typeString"));
  appendField(MyMoneyDbIntColumn("occurence", MyMoneyDbIntColumn::SMALL, UNSIGNED, false, // krazy:exclude=spelling
                                 NOTNULL));
  appendField(MyMoneyDbIntColumn("occurenceMultiplier", MyMoneyDbIntColumn::SMALL, UNSIGNED, // krazy:exclude=spelling
                                 false, NOTNULL, 3));
  appendField(MyMoneyDbTextColumn("occurenceString")); // krazy:exclude=spelling
  appendField(MyMoneyDbIntColumn("paymentType", MyMoneyDbIntColumn::TINY, UNSIGNED));
  appendField(MyMoneyDbTextColumn("paymentTypeString", MyMoneyDbTextColumn::LONG));
  appendField(MyMoneyDbColumn("startDate", "date", false, NOTNULL));
  appendField(MyMoneyDbColumn("endDate", "date"));
  appendField(MyMoneyDbColumn("fixed", "char(1)", false, NOTNULL));
  appendField(MyMoneyDbColumn("lastDayInMonth", "char(1)", false, NOTNULL, 11, std::numeric_limits<int>::max(), QLatin1String("N")));
  appendField(MyMoneyDbColumn("autoEnter", "char(1)", false, NOTNULL));
  appendField(MyMoneyDbColumn("lastPayment", "date"));
  appendField(MyMoneyDbColumn("nextPaymentDue", "date"));
  appendField(MyMoneyDbIntColumn("weekendOption", MyMoneyDbIntColumn::TINY, UNSIGNED, false,
                                 NOTNULL));
  appendField(MyMoneyDbTextColumn("weekendOptionString"));
  MyMoneyDbTable t("kmmSchedules", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::SchedulePaymentHistory()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("schedId", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("payDate", "date", PRIMARYKEY,  NOTNULL));
  MyMoneyDbTable t("kmmSchedulePaymentHistory", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Securities()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("name", "text", false, NOTNULL));
  appendField(MyMoneyDbTextColumn("symbol"));
  appendField(MyMoneyDbIntColumn("type", MyMoneyDbIntColumn::SMALL, UNSIGNED, false, NOTNULL));
  appendField(MyMoneyDbTextColumn("typeString"));
  appendField(MyMoneyDbColumn("smallestAccountFraction", "varchar(24)"));
  appendField(MyMoneyDbIntColumn("pricePrecision", MyMoneyDbIntColumn::SMALL, UNSIGNED, false, NOTNULL));
  appendField(MyMoneyDbTextColumn("tradingMarket"));
  appendField(MyMoneyDbColumn("tradingCurrency", "char(3)"));
  appendField(MyMoneyDbIntColumn("roundingMethod", MyMoneyDbIntColumn::SMALL, UNSIGNED, false, NOTNULL, 11, std::numeric_limits<int>::max(), QString("%1").arg(AlkValue::RoundRound)));
  MyMoneyDbTable t("kmmSecurities", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Prices()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("fromId", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("toId", "varchar(32)",  PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("priceDate", "date", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbTextColumn("price", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  appendField(MyMoneyDbTextColumn("priceFormatted"));
  appendField(MyMoneyDbTextColumn("priceSource"));
  MyMoneyDbTable t("kmmPrices", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Currencies()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("ISOcode", "char(3)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbTextColumn("name", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  appendField(MyMoneyDbIntColumn("type", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  appendField(MyMoneyDbTextColumn("typeString"));
  appendField(MyMoneyDbIntColumn("symbol1", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  appendField(MyMoneyDbIntColumn("symbol2", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  appendField(MyMoneyDbIntColumn("symbol3", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  appendField(MyMoneyDbColumn("symbolString", "varchar(255)"));
  appendField(MyMoneyDbColumn("smallestCashFraction", "varchar(24)"));
  appendField(MyMoneyDbColumn("smallestAccountFraction", "varchar(24)"));
  // the default for price precision was taken from MyMoneySecurity
  appendField(MyMoneyDbIntColumn("pricePrecision", MyMoneyDbIntColumn::SMALL, UNSIGNED, false, NOTNULL, 11, std::numeric_limits<int>::max(), QLatin1String("4")));
  MyMoneyDbTable t("kmmCurrencies", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Reports()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("name", "varchar(255)", false, NOTNULL));
  appendField(MyMoneyDbTextColumn("XML", MyMoneyDbTextColumn::LONG));
  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL, 6));
  MyMoneyDbTable t("kmmReportConfig", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::OnlineJobs()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;

  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL, 8));
  appendField(MyMoneyDbColumn("type", "varchar(255)", false, NOTNULL, 8));
  appendField(MyMoneyDbDatetimeColumn("jobSend", false, false, 8));
  appendField(MyMoneyDbDatetimeColumn("bankAnswerDate", false, false, 8));
  appendField(MyMoneyDbColumn("state", "varchar(15)", false, NOTNULL, 8));
  appendField(MyMoneyDbColumn("locked", "char(1)", false, NOTNULL, 8));

  MyMoneyDbTable t("kmmOnlineJobs", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::PayeeIdentifier()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;

  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL, 8));
  appendField(MyMoneyDbColumn("type", "varchar(255)", false, false, 8));

  MyMoneyDbTable t("kmmPayeeIdentifier", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::PluginInfo()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;

  appendField(MyMoneyDbColumn("iid", "varchar(255)", PRIMARYKEY, NOTNULL, 8));
  appendField(MyMoneyDbIntColumn("versionMajor", MyMoneyDbIntColumn::TINY, false, false, NOTNULL, 8));
  appendField(MyMoneyDbIntColumn("versionMinor", MyMoneyDbIntColumn::TINY, false, false, false, 8));
  appendField(MyMoneyDbTextColumn("uninstallQuery", MyMoneyDbTextColumn::LONG, false, false, 8));

  MyMoneyDbTable t("kmmPluginInfo", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Budgets()
{
  QList<QExplicitlySharedDataPointer <MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("name", "text", false, NOTNULL));
  appendField(MyMoneyDbColumn("start", "date", false, NOTNULL));
  appendField(MyMoneyDbTextColumn("XML", MyMoneyDbTextColumn::LONG));
  MyMoneyDbTable t("kmmBudgetConfig", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::CostCenter()
{
  QList<QExplicitlySharedDataPointer<MyMoneyDbColumn> > fields;
  appendField(MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  appendField(MyMoneyDbColumn("name", "text", false, NOTNULL));
  MyMoneyDbTable t("kmmCostCenter", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Balances()
{
  MyMoneyDbView v("kmmBalances", "CREATE VIEW kmmBalances AS "
                  "SELECT kmmAccounts.id AS id, kmmAccounts.currencyId, "
                  "kmmSplits.txType, kmmSplits.value, kmmSplits.shares, "
                  "kmmSplits.postDate AS balDate, "
                  "kmmTransactions.currencyId AS txCurrencyId "
                  "FROM kmmAccounts, kmmSplits, kmmTransactions "
                  "WHERE kmmSplits.txType = 'N' "
                  "AND kmmSplits.accountId = kmmAccounts.id "
                  "AND kmmSplits.transactionId = kmmTransactions.id;");
  m_views[v.name()] = v;
}


// function to write create SQL to a stream
const QString MyMoneyDbDef::generateSQL(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver) const
{
  QString retval;

  // Add the CREATE TABLE strings
  table_iterator tt = tableBegin();
  while (tt != tableEnd()) {
    retval += (*tt).generateCreateSQL(driver) + '\n';
    ++tt;
  }

  // Add the CREATE OR REPLACE VIEW strings
  view_iterator vt = viewBegin();
  while (vt != viewEnd()) {
    retval += (*vt).createString() + '\n';
    ++vt;
  }
  retval += '\n';

  // Add the strings to populate kmmFileInfo with initial values
  MyMoneyDbTable fi = m_tables["kmmFileInfo"];
  QString qs = fi.insertString();
  MyMoneyDbTable::field_iterator fit;
  for (fit = fi.begin(); fit != fi.end(); ++fit) {
    QString toReplace = (*fit)->name();
    toReplace.prepend(':');
    QString replace = "NULL";
    if ((*fit)->name() == "version")
      replace = QString::number(m_currentVersion);
    if ((*fit)->name() == "fixLevel")
      replace =  QString::number
                 (MyMoneyFile::instance()->storage()->currentFixVersion());
    if ((*fit)->name() == "created")
      replace = QLatin1Char('\'')
                + QDate::currentDate().toString(Qt::ISODate)
                + QLatin1Char('\'');
    if ((*fit)->name() == "lastModified")
      replace = QLatin1Char('\'')
                + QDate::currentDate().toString(Qt::ISODate)
                + QLatin1Char('\'');
    if ((*fit)->name() == "updateInProgress")
      replace = enclose("N");

    qs.replace(QRegExp(toReplace + "(?=[,\\s\\)])"), replace);
    // only replace parameters followed by comma, whitespace, closing parenthesis - otherwise
    // conflicts may occur if one parameter starts with the name of another one.
  }
  qs += "\n\n";
  retval += qs;

  // Add the strings to create the initial accounts
  qs.clear();
  QList<MyMoneyAccount> stdList;
  stdList.append(MyMoneyFile::instance()->asset());
  stdList.append(MyMoneyFile::instance()->equity());
  stdList.append(MyMoneyFile::instance()->expense());
  stdList.append(MyMoneyFile::instance()->income());
  stdList.append(MyMoneyFile::instance()->liability());
  for (int i = 0; i < stdList.count(); ++i) {
    MyMoneyAccount* pac = &stdList[i];
    MyMoneyDbTable ac = m_tables["kmmAccounts"];
    qs = ac.insertString();
    MyMoneyDbTable::field_iterator act;
    // do the following in reverse so the 'formatted' fields are
    // correctly handled.
    // Hmm, how does one use a QValueListIterator in reverse
    // It'll be okay in Qt4 with QListIterator
    for (act = ac.end(), --act; act != ac.begin(); --act) {
      QString toReplace = (*act)->name();
      toReplace.prepend(':');
      QString replace = "NULL";
      if ((*act)->name() == "accountType")
        replace = QString::number((int)pac->accountType());
      if ((*act)->name() == "accountTypeString")
        replace = enclose(pac->name());
      if ((*act)->name() == "isStockAccount")
        replace = enclose("N");
      if ((*act)->name() == "accountName")
        replace = enclose(pac->name());
      qs.replace(toReplace, replace);
    }
    qs.replace(":id", enclose(pac->id()));  // a real kludge
    qs += "\n\n";
    retval += qs;
  }

  return retval;
}

//*****************************************************************************

void MyMoneyDbTable::addIndex(const QString& name, const QStringList& columns, bool unique)
{
  m_indices.push_back(MyMoneyDbIndex(m_name, name, columns, unique));
}

void MyMoneyDbTable::buildSQLStrings()
{
  // build fixed SQL strings for this table
  // build the insert string with placeholders for each field
  QString qs = QString("INSERT INTO %1 (").arg(name());
  QString ws = ") VALUES (";
  field_iterator ft = m_fields.constBegin();
  while (ft != m_fields.constEnd()) {
    qs += QString("%1, ").arg((*ft)->name());
    ws += QString(":%1, ").arg((*ft)->name());
    ++ft;
  }
  qs = qs.left(qs.length() - 2);
  ws = ws.left(ws.length() - 2);
  m_insertString = qs + ws + ");";
  // build a 'select all' string (select * is deprecated)
  // don't terminate with semicolon coz we may want a where or order clause
  m_selectAllString = "SELECT " + columnList() + " FROM " + name();

  // build an update string; key fields go in the where clause
  qs = "UPDATE " + name() + " SET ";
  ws.clear();
  ft = m_fields.constBegin();
  while (ft != m_fields.constEnd()) {
    if ((*ft)->isPrimaryKey()) {
      if (!ws.isEmpty()) ws += " AND ";
      ws += QString("%1 = :%2").arg((*ft)->name()).arg((*ft)->name());
    } else {
      qs += QString("%1 = :%2, ").arg((*ft)->name()).arg((*ft)->name());
    }
    ++ft;
  }
  qs = qs.left(qs.length() - 2);
  if (!ws.isEmpty()) qs += " WHERE " + ws;
  m_updateString = qs + ';';
  // build a delete string; where clause as for update
  qs = "DELETE FROM " + name();
  if (!ws.isEmpty()) qs += " WHERE " + ws;
  m_deleteString = qs + ';';

  // Setup the column name hash
  ft = m_fields.constBegin();
  m_fieldOrder.reserve(m_fields.size());
  int i = 0;
  while (ft != m_fields.constEnd()) {
    m_fieldOrder[(*ft)->name()] = i;
    ++i; ++ft;
  }
}

const QString MyMoneyDbTable::columnList(const int version) const
{
  field_iterator ft = m_fields.begin();
  QString qs;
  ft = m_fields.begin();
  while (ft != m_fields.end()) {
    if ((*ft)->initVersion() <= version && (*ft)->lastVersion() >= version) {
      qs += QString("%1, ").arg((*ft)->name());
    }
    ++ft;
  }
  return (qs.left(qs.length() - 2));
}

const QString MyMoneyDbTable::generateCreateSQL(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver, int version) const
{
  QString qs = QString("CREATE TABLE %1 (").arg(name());
  QString pkey;
  for (field_iterator it = m_fields.begin(); it != m_fields.end(); ++it) {
    if ((*it)->initVersion() <= version && (*it)->lastVersion() >= version) {
      qs += (*it)->generateDDL(driver) + ", ";
      if ((*it)->isPrimaryKey())
        pkey += (*it)->name() + ", ";
    }
  }

  if (!pkey.isEmpty()) {
    qs += "PRIMARY KEY (" + pkey;
    qs = qs.left(qs.length() - 2) + "))";
  } else {
    qs = qs.left(qs.length() - 2) + ')';
  }

  qs += driver->tableOptionString();
  qs += ";\n";

  for (index_iterator ii = m_indices.begin(); ii != m_indices.end(); ++ii) {
    qs += (*ii).generateDDL(driver);
  }
  return qs;
}

const QString MyMoneyDbTable::dropPrimaryKeyString(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver) const
{
  return driver->dropPrimaryKeyString(m_name);
}

bool MyMoneyDbTable::hasPrimaryKey(int version) const
{
  field_iterator ft = m_fields.constBegin();
  while (ft != m_fields.constEnd()) {
    if ((*ft)->initVersion() <= version && (*ft)->lastVersion() >= version) {
      if ((*ft)->isPrimaryKey())
        return (true);
    }
    ++ft;
  }
  return (false);
}

const QString MyMoneyDbTable::modifyColumnString(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver, const QString& columnName, const MyMoneyDbColumn& newDef) const
{
  return driver->modifyColumnString(m_name, columnName, newDef);
}

int MyMoneyDbTable::fieldNumber(const QString& name) const
{
  QHash<QString, int>::ConstIterator i = m_fieldOrder.find(name);
  if (m_fieldOrder.constEnd() == i) {
    throw MYMONEYEXCEPTION(QString("Unknown field %1 in table %2").arg(name).arg(m_name));
  }
  return i.value();
}

//*****************************************************************************
const QString MyMoneyDbIndex::generateDDL(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver) const
{
  Q_UNUSED(driver);

  QString qs = "CREATE ";

  if (m_unique)
    qs += "UNIQUE ";

  qs += "INDEX " + m_table + '_' + m_name + "_idx ON "
        + m_table + " (";

  // The following should probably be revised.  MySQL supports an index on
  // partial columns, but not on a function.  Postgres supports an index on
  // the result of an SQL function, but not a partial column.  There should be
  // a way to merge these, and support other DBMSs like SQLite at the same time.
  // For now, if we just use plain columns, this will work fine.
  for (QStringList::ConstIterator it = m_columns.constBegin(); it != m_columns.constEnd(); ++it) {
    qs += *it + ',';
  }

  qs = qs.left(qs.length() - 1) + ");\n";

  return qs;
}

//*****************************************************************************
// These are the actual column types.
//

MyMoneyDbColumn*         MyMoneyDbColumn::clone() const
{
  return (new MyMoneyDbColumn(*this));
}

MyMoneyDbIntColumn*      MyMoneyDbIntColumn::clone() const
{
  return (new MyMoneyDbIntColumn(*this));
}

MyMoneyDbDatetimeColumn* MyMoneyDbDatetimeColumn::clone() const
{
  return (new MyMoneyDbDatetimeColumn(*this));
}

MyMoneyDbTextColumn* MyMoneyDbTextColumn::clone() const
{
  return (new MyMoneyDbTextColumn(*this));
}

const QString MyMoneyDbColumn::generateDDL(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver) const
{
  Q_UNUSED(driver);

  QString qs = name() + ' ' + type();
  if (isNotNull()) qs += " NOT NULL";
  if (!defaultValue().isEmpty())
      qs += QString(" DEFAULT \"%1\"").arg(defaultValue());
  return qs;
}

const QString MyMoneyDbIntColumn::generateDDL(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver) const
{
  QString qs = driver->intString(*this);
  if (!defaultValue().isEmpty())
      qs += QString(" DEFAULT %1").arg(defaultValue());
  return qs;
}

const QString MyMoneyDbTextColumn::generateDDL(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver) const
{
  return driver->textString(*this);
}

const QString MyMoneyDbDatetimeColumn::generateDDL(const QExplicitlySharedDataPointer<MyMoneyDbDriver>& driver) const
{
  return driver->timestampString(*this);
}


