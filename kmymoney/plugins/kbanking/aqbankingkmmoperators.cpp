/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
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
 * @brief SEPA Charset
 *
 * Additional lower case letters were added, or should the input mask replace them with the uppercase version? The bank should do that
 * anyway.
 */
static QString sepaChars()
{
    return QLatin1String("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':?.,- (+)/");
}

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

    settings->setAllowedChars(sepaChars());

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
    Q_FOREACH (payeeIdentifier accountNumber, accountNumbers) {
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
    AB_Value_GetNumDenomString(value, &buffer[0], sizeof(buffer));
    return MyMoneyMoney(QString::fromUtf8(buffer));
}
