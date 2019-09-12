/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013-2015 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aqbankingkmmoperators.h"

#include <aqbanking/types/transactionlimits.h>
#include <aqbanking/types/transaction.h>
#include <aqbanking/types/account_spec.h>
#include <aqbanking/types/value.h>

#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "tasksettings/credittransfersettingsbase.h"
#include "onlinetasks/sepa/sepaonlinetransfer.h"
#include "gwenhywfarqtoperators.h"
#include "mymoneymoney.h"

/**
 * @brief DTAUS Chars
 *
 * @source http://www.hbci-zka.de/dokumente/spezifikation_deutsch/FinTS_3.0_Messages_Finanzdatenformate_2010-08-06_final_version.pdf
 *
 * @note This file is saved in UTF-8!
 *
 * Additional lower case letters were added, or should the input mask replaced them mit the uppercase version? The bank should do that
 * anyway.
 */
static const QString dtausChars = QString::fromUtf8("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜß .,&-+*%/$&abcdefghijklmnopqrstuvwxyzäöü");

/**
 * @brief Sepa Charset
 *
 * Additional lower case letters were added, or should the input mask replaced them mit the uppercase version? The bank should do that
 * anyway.
 */
static const QString sepaChars = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':?.,- (+)/");

/** @todo Check if AB_TransactionLimits_GetMaxLenCustomerReference really is the limit for the sepa reference */
QSharedPointer<sepaOnlineTransfer::settings> AB_TransactionLimits_toSepaOnlineTaskSettings(const AB_TRANSACTION_LIMITS* aqlimits)
{
  Q_CHECK_PTR(aqlimits);

  QSharedPointer<creditTransferSettingsBase> settings(new creditTransferSettingsBase);

  settings->setPurposeLimits(AB_TransactionLimits_GetMaxLinesPurpose(aqlimits),
                             AB_TransactionLimits_GetMaxLenPurpose(aqlimits),
                             AB_TransactionLimits_GetMinLenPurpose(aqlimits)
                            );

  // AqBanking returns 0 as min length even if it requires one
  int minLength = AB_TransactionLimits_GetMinLenRemoteName(aqlimits);
  if (minLength == 0)
    minLength = 1;
  settings->setRecipientNameLimits(1 /*AB_TransactionLimits_GetMaxLinesRemoteName(aqlimits)*/,
                                   AB_TransactionLimits_GetMaxLenRemoteName(aqlimits),
                                   minLength
                                  );

  // AqBanking returns 0 as min length even if it requires one
  minLength = AB_TransactionLimits_GetMinLenLocalName(aqlimits);
  if (minLength == 0)
    minLength = 1;
  settings->setPayeeNameLimits(1, AB_TransactionLimits_GetMaxLenLocalName(aqlimits), minLength);

  //settings->referenceLength = AB_TransactionLimits_GetMax( aqlimits );
  settings->setEndToEndReferenceLength(32);

  settings->setAllowedChars(sepaChars);

  return settings.dynamicCast<sepaOnlineTransfer::settings>();
}

void AB_Transaction_SetRemoteAccount(AB_TRANSACTION* transaction, const payeeIdentifiers::nationalAccount& ident)
{
  Q_CHECK_PTR(transaction);

  AB_Transaction_SetRemoteAccountNumber(transaction, ident.accountNumber().toUtf8().constData());
  AB_Transaction_SetRemoteBankCode(transaction, ident.bankCode().toUtf8().constData());
  AB_Transaction_SetRemoteName(transaction, ident.ownerName().toUtf8().constData());
}

void AB_Transaction_SetRemoteAccount(AB_TRANSACTION* transaction, const payeeIdentifiers::ibanBic& ident)
{
  Q_CHECK_PTR(transaction);

  AB_Transaction_SetRemoteAccountNumber(transaction, ident.electronicIban().toUtf8().constData());
  AB_Transaction_SetRemoteBankCode(transaction, ident.fullStoredBic().toUtf8().constData());
  AB_Transaction_SetRemoteName(transaction, ident.ownerName().toUtf8().constData());
}

void AB_Transaction_SetLocalAccount(AB_TRANSACTION* transaction, const AB_ACCOUNT_SPEC* account)
{
  Q_CHECK_PTR(transaction);
  Q_CHECK_PTR(account);

  AB_Transaction_SetLocalName(transaction, AB_AccountSpec_GetOwnerName(account));
  AB_Transaction_SetLocalAccountNumber(transaction, AB_AccountSpec_GetAccountNumber(account));
  AB_Transaction_SetLocalBankCode(transaction, AB_AccountSpec_GetBankCode(account));

  AB_Transaction_SetLocalIban(transaction, AB_AccountSpec_GetIban(account));
  AB_Transaction_SetLocalBic(transaction, AB_AccountSpec_GetBic(account));
}

void AB_Transaction_SetLocalAccount(AB_TRANSACTION* transaction, const payeeIdentifiers::nationalAccount& ident)
{
  Q_CHECK_PTR(transaction);

  AB_Transaction_SetLocalName(transaction, ident.ownerName().toUtf8().constData());
  AB_Transaction_SetLocalAccountNumber(transaction, ident.accountNumber().toUtf8().constData());
  AB_Transaction_SetLocalBankCode(transaction, ident.bankCode().toUtf8().constData());
}

bool AB_Transaction_SetLocalAccount(AB_TRANSACTION* transaction, const QList<payeeIdentifier>& accountNumbers)
{
  Q_CHECK_PTR(transaction);

  bool validOriginAccountSet = false;
  foreach (payeeIdentifier accountNumber, accountNumbers) {
    if (!accountNumber.isValid())
      continue;

    try {
      payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban(accountNumber);
      AB_Transaction_SetLocalIban(transaction, iban->electronicIban().toUtf8().constData());
      AB_Transaction_SetLocalBic(transaction, iban->fullStoredBic().toUtf8().constData());
    } catch (...) {
    }

    try {
      payeeIdentifierTyped<payeeIdentifiers::nationalAccount> national(accountNumber);
      AB_Transaction_SetLocalAccount(transaction, *(national.data()));
      validOriginAccountSet = true;
    } catch (...) {
    }
  }
  return validOriginAccountSet;
}

AB_VALUE* AB_Value_fromMyMoneyMoney(const MyMoneyMoney& input)
{
  return (AB_Value_fromString(input.toString().toUtf8().constData()));
}

MyMoneyMoney AB_Value_toMyMoneyMoney(const AB_VALUE *const value)
{
  // I've read somewhere that in M1 were about 12 trillion dollar in 2013. So the buffer length of 32 should be sufficient.
  char buffer[32];
  memset(buffer, 0, sizeof(buffer));
  AB_Value_GetNumDenomString(value, static_cast<char*>(buffer), 32);
  return MyMoneyMoney(QString::fromUtf8(buffer));
}
