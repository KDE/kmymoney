/*
 * SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

