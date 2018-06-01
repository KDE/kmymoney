/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mymoneystoragenames.h"

namespace MyMoneyStorageTags {

const QHash<tagNameE, QString> tagNames = {
  {tnInstitutions, QStringLiteral("INSTITUTIONS")},
  {tnPayees, QStringLiteral("PAYEES")},
  {tnCostCenters, QStringLiteral("COSTCENTERS")},
  {tnTags, QStringLiteral("TAGS")},
  {tnAccounts, QStringLiteral("ACCOUNTS")},
  {tnTransactions, QStringLiteral("TRANSACTIONS")},
  {tnSchedules, QStringLiteral("SCHEDULES")},
  {tnSecurities, QStringLiteral("SECURITIES")},
  {tnCurrencies, QStringLiteral("CURRENCIES")},
  {tnPrices, QStringLiteral("PRICES")},
  {tnReports, QStringLiteral("REPORTS")},
  {tnBudgets, QStringLiteral("BUDGETS")},
  {tnOnlineJobs, QStringLiteral("ONLINEJOBS")},
  {tnKMMFile, QStringLiteral("KMYMONEY-FILE")},
  {tnFileInfo, QStringLiteral("FILEINFO")},
  {tnUser, QStringLiteral("USER")}
};

}

namespace MyMoneyStorageNodes {

const QHash<ndNameE, QString> nodeNames = {
  {nnInstitution, QStringLiteral("INSTITUTION")},
  {nnPayee, QStringLiteral("PAYEE")},
  {nnCostCenter, QStringLiteral("COSTCENTER")},
  {nnTag, QStringLiteral("TAG")},
  {nnAccount, QStringLiteral("ACCOUNT")},
  {nnTransaction, QStringLiteral("TRANSACTION")},
  {nnScheduleTX, QStringLiteral("SCHEDULED_TX")},
  {nnSecurity, QStringLiteral("SECURITY")},
  {nnCurrency, QStringLiteral("CURRENCY")},
  {nnPrice, QStringLiteral("PRICE")},
  {nnPricePair, QStringLiteral("PRICEPAIR")},
  {nnReport, QStringLiteral("REPORT")},
  {nnBudget, QStringLiteral("BUDGET")},
  {nnOnlineJob, QStringLiteral("ONLINEJOB")},
  {nnKeyValuePairs, QStringLiteral("KEYVALUEPAIRS")},
  {nnEquity, QStringLiteral("EQUITY")},
};

}

namespace MyMoneyStorageAttributes {

const QHash<attrNameE, QString> attrNames = {
  {anID, QStringLiteral("id")},
  {anDate, QStringLiteral("date")},
  {anCount, QStringLiteral("count")},
  {anFrom, QStringLiteral("from")},
  {anTo, QStringLiteral("to")},
  {anSource, QStringLiteral("source")},
  {anKey, QStringLiteral("key")},
  {anValue, QStringLiteral("value")},
  {anPrice, QStringLiteral("price")},
  {anName, QStringLiteral("name")},
  {anEmail, QStringLiteral("email")},
  {anCountry, QStringLiteral("county")},
  {anCity, QStringLiteral("city")},
  {anZipCode, QStringLiteral("zipcode")},
  {anStreet, QStringLiteral("street")},
  {anTelephone, QStringLiteral("telephone")}
};

}

namespace MyMoneyStandardAccounts {

  // definitions for the ID's of the standard accounts
  const QHash<idNameE, QString> stdAccNames {
    {stdAccLiability, QStringLiteral("AStd::Liability")},
    {stdAccAsset,     QStringLiteral("AStd::Asset")},
    {stdAccExpense,   QStringLiteral("AStd::Expense")},
    {stdAccIncome,    QStringLiteral("AStd::Income")},
    {stdAccEquity,    QStringLiteral("AStd::Equity")},
  };

}
