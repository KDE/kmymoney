/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSACTION_P_H
#define TRANSACTION_P_H

#include "registeritem_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QColor>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneysplit.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"

namespace KMyMoneyRegister
{
class  TransactionPrivate : public RegisterItemPrivate
{
public:
    TransactionPrivate() :
        m_form(nullptr),
        m_formRowHeight(-1),
        m_selected(false),
        m_focus(false),
        m_erroneous(false),
        m_inEdit(false),
        m_inRegisterEdit(false),
        m_showBalance(true),
        m_reducedIntensity(false)
    {
    }

    virtual ~ TransactionPrivate()
    {
    }

    void init(int uniqueId)
    {
        auto file = MyMoneyFile::instance();

        // load the account
        if (!m_split.accountId().isEmpty())
            m_account = file->account(m_split.accountId());

        // load the payee
        if (!m_split.payeeId().isEmpty()) {
            m_payee = file->payee(m_split.payeeId()).name();
        }
        if (m_parent->account().isIncomeExpense()) {
            m_payeeHeader = m_split.shares().isNegative() ? i18nc("Payer", "From") : i18nc("Payee", "Pay to");
        } else {
            m_payeeHeader = m_split.shares().isNegative() ? i18nc("Payee", "Pay to") : i18nc("Payer", "From");
        }

        // load the tag
        if (!m_split.tagIdList().isEmpty()) {
            const QList<QString> t = m_split.tagIdList();
            for (auto i = 0; i < t.count(); i++) {
                m_tagList << file->tag(t[i]).name();
                m_tagColorList << file->tag(t[i]).tagColor();
            }
        }

        // load the currency
        if (!m_transaction.id().isEmpty())
            m_splitCurrencyId = m_account.currencyId();

        // check if transaction is erroneous or not
        m_erroneous = !m_transaction.splitSum().isZero();

        if (!m_uniqueId.isEmpty()) {
            m_uniqueId += '-';
            QString id;
            id.setNum(uniqueId);
            m_uniqueId += id.rightJustified(3, '0');
        }
    }

    MyMoneyTransaction      m_transaction;
    MyMoneySplit            m_split;
    MyMoneyAccount          m_account;
    MyMoneyMoney            m_balance;
    QTableWidget*           m_form;
    QString                 m_category;
    QString                 m_payee;
    QString                 m_payeeHeader;
    QList<QString>          m_tagList;
    QList<QColor>           m_tagColorList;
    QString                 m_categoryHeader;
    QString                 m_splitCurrencyId;
    QString                 m_uniqueId;
    int                     m_formRowHeight;
    bool                    m_selected;
    bool                    m_focus;
    bool                    m_erroneous;
    bool                    m_inEdit;
    bool                    m_inRegisterEdit;
    bool                    m_showBalance;
    bool                    m_reducedIntensity;
};
}

#endif
