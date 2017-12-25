/***************************************************************************
                          mymoneystoragenames.h
                             -------------------
    begin                : Sun Jun 18 2017
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSTORAGENAMES_H
#define MYMONEYSTORAGENAMES_H

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

extern const QHash<idNameE, QString>    stdAccNames;
}

#endif
