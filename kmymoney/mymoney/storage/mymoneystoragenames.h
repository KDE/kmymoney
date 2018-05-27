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

#ifndef MYMONEYSTORAGENAMES_H
#define MYMONEYSTORAGENAMES_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

namespace MyMoneyStorageTags {

enum tagNameE { tnInstitutions, tnPayees, tnCostCenters,
                tnTags, tnAccounts, tnTransactions,
                tnSchedules, tnSecurities, tnCurrencies,
                tnPrices, tnReports, tnBudgets, tnOnlineJobs,
                tnKMMFile, tnFileInfo, tnUser
              };

extern const QHash<tagNameE, QString>    tagNames;
}

namespace MyMoneyStorageNodes {

enum ndNameE { nnInstitution, nnPayee, nnCostCenter,
               nnTag, nnAccount, nnTransaction,
               nnScheduleTX, nnSecurity, nnCurrency,
               nnPrice, nnPricePair, nnReport, nnBudget, nnOnlineJob,
               nnKeyValuePairs, nnEquity
             };

extern const QHash<ndNameE, QString>    nodeNames;
}

namespace MyMoneyStorageAttributes {

enum attrNameE { anID, anDate, anCount, anFrom, anTo,
                 anSource, anKey, anValue, anPrice,
                 anName, anEmail, anCountry, anCity,
                 anZipCode, anStreet, anTelephone
               };

extern const QHash<attrNameE, QString>    attrNames;
}

namespace MyMoneyStandardAccounts {

enum idNameE { stdAccLiability, stdAccAsset, stdAccExpense, stdAccIncome, stdAccEquity };

extern const KMM_MYMONEY_EXPORT QHash<idNameE, QString>    stdAccNames;
}

#endif
