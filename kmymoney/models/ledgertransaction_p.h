/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERTRANSACTION_P_H
#define LEDGERTRANSACTION_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneypayee.h"
#include "mymoneymoney.h"

class LedgerTransactionPrivate
{
public:
  LedgerTransactionPrivate() :
    m_erroneous(false)
  {
  }

  virtual ~LedgerTransactionPrivate()
  {
  }

  void init(const MyMoneyTransaction& t, const MyMoneySplit& s)
  {
    m_transaction = t;
    m_split = s;
    // extract the payee id
    auto payeeId = m_split.payeeId();
    if(payeeId.isEmpty()) {
      foreach (const auto split, m_transaction.splits()) {
        if(!split.payeeId().isEmpty()) {
          payeeId = split.payeeId();
          break;
        }
      }
    }
    if(!payeeId.isEmpty()) {
      m_payeeId = payeeId;
      m_payeeName = MyMoneyFile::instance()->payee(payeeId).name();
    }

    m_account = MyMoneyFile::instance()->accountToCategory(m_split.accountId());
    m_costCenterId = m_split.costCenterId();

    // A transaction can have more than 2 splits ...
    if(m_transaction.splitCount() > 2) {
      m_counterAccount = i18n("Split transaction");

    // ... exactly two splits ...
    } else if(m_transaction.splitCount() == 2) {
      foreach (const auto split, m_transaction.splits()) {
        if(split.id() != m_split.id()) {
          m_counterAccountId = split.accountId();
          m_counterAccount = MyMoneyFile::instance()->accountToCategory(m_counterAccountId);
          // in case the own split does not have a costcenter, but the counter split does
          // we use it nevertheless
          if(m_costCenterId.isEmpty())
            m_costCenterId = split.costCenterId();
          break;
        }
      }

    // ... or a single split
    } else if(!m_split.shares().isZero()) {
      m_counterAccount = i18n("*** UNASSIGNED ***");
    }

    // The transaction is erroneous in case it is not balanced
    m_erroneous = !m_transaction.splitSum().isZero();

    // now take care of the values
    setupValueDisplay();
  }

  void setupValueDisplay()
  {
    const auto file = MyMoneyFile::instance();
    const auto acc = file->account(m_split.accountId());

    auto value = m_split.value(m_transaction.commodity(), acc.currencyId());
    m_signedShares = value.formatMoney(acc.fraction());

    if(value.isNegative()) {
      m_shares = m_payment = (-value).formatMoney(acc.fraction());
    } else {
      m_shares = m_deposit = m_signedShares;
    }

    // figure out if it is a debit or credit split. s.a. https://en.wikipedia.org/wiki/Debits_and_credits#Aspects_of_transactions
    if(m_split.shares().isNegative()) {
      m_sharesSuffix = i18nc("Credit suffix", "Cr.");
    } else {
      m_sharesSuffix = i18nc("Debit suffix", "Dr.");
    }
  }

  MyMoneyTransaction  m_transaction;
  MyMoneySplit        m_split;
  QString             m_counterAccountId;
  QString             m_counterAccount;
  QString             m_account;
  QString             m_costCenterId;
  QString             m_payeeName;
  QString             m_payeeId;
  QString             m_shares;
  QString             m_signedShares;
  QString             m_payment;
  QString             m_deposit;
  QString             m_balance;
  QString             m_sharesSuffix; // shows Cr or Dr
  bool                m_erroneous;
};

#endif // LEDGERTRANSACTION_P_H
