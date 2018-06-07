/*
 * Copyright 2008-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "stdtransactiondownloaded.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyleOptionViewItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionDownloaded::StdTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  StdTransaction(parent, transaction, split, uniqueId)
{
}

StdTransactionDownloaded::~StdTransactionDownloaded()
{
}

const char* StdTransactionDownloaded::className()
{
  return "StdTransactionDownloaded";
}

bool StdTransactionDownloaded::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)

{
  auto rc = Transaction::paintRegisterCellSetup(painter, option, index);
  // if not selected paint in selected background color
  if (!isSelected()) {
    option.palette.setColor(QPalette::Base, KMyMoneySettings::schemeColor(SchemeColor::TransactionImported));
    option.palette.setColor(QPalette::AlternateBase, KMyMoneySettings::schemeColor(SchemeColor::TransactionImported));
  }
  return rc;
}

InvestTransactionDownloaded::InvestTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  InvestTransaction(parent, transaction, split, uniqueId)
{
}

InvestTransactionDownloaded::~InvestTransactionDownloaded()
{
}

const char* InvestTransactionDownloaded::className()
{
  return "InvestTransactionDownloaded";
}

bool InvestTransactionDownloaded::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  auto rc = Transaction::paintRegisterCellSetup(painter, option, index);
  // if not selected paint in selected background color
  if (!isSelected()) {
    option.palette.setColor(QPalette::Base, KMyMoneySettings::schemeColor(SchemeColor::TransactionImported));
    option.palette.setColor(QPalette::AlternateBase, KMyMoneySettings::schemeColor(SchemeColor::TransactionImported));
  }
  return rc;
}

