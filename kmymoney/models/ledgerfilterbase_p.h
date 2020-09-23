/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef LEDGERFILTERBASE_P_H
#define LEDGERFILTERBASE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kconcatenaterowsproxymodel.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"

class LedgerFilterBasePrivate {
public:
  LedgerFilterBasePrivate(LedgerFilterBase* qq)
  : q(qq)
  , concatModel(nullptr)
  , accountType(eMyMoney::Account::Type::Asset)
  , showValuesInverted(false)
  {
  }

  inline bool isAccountsModel(const QAbstractItemModel* model) const
  {
    return (model == static_cast<void*>(MyMoneyFile::instance()->accountsModel()));
  }

  inline bool isSpecialDatesModel(const QAbstractItemModel* model) const
  {
    return (model == static_cast<void*>(MyMoneyFile::instance()->specialDatesModel()));
  }

  LedgerFilterBase*           q;
  KConcatenateRowsProxyModel* concatModel;          // Qt5.13+ use QConcatenateTablesProxyModel
  eMyMoney::Account::Type     accountType;
  QStringList                 filterIds;
  bool                        showValuesInverted;
  QSet<QAbstractItemModel*>   sourceModels;
};

#endif
