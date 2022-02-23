/*
    SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyaccountcompletion.h"
#include "kmymoneycompletion_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLayout>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyaccountselector.h"
#include "mymoneyenums.h"

KMyMoneyAccountCompletion::KMyMoneyAccountCompletion(QWidget *parent) :
    KMyMoneyCompletion(parent)
{
    Q_D(KMyMoneyCompletion);
    delete d->m_selector;
    d->m_selector = new KMyMoneyAccountSelector(this, 0, false);
    d->m_selector->listView()->setFocusProxy(parent);
    layout()->addWidget(d->m_selector);

#ifndef KMM_DESIGNER
    // Default is to show all accounts
    // FIXME We should leave this also to the caller
    AccountSet set;
    set.addAccountGroup(eMyMoney::Account::Type::Asset);
    set.addAccountGroup(eMyMoney::Account::Type::Liability);
    set.addAccountGroup(eMyMoney::Account::Type::Income);
    set.addAccountGroup(eMyMoney::Account::Type::Expense);
    set.load(selector());
#endif

    connectSignals(d->m_selector, d->m_selector->listView());
}

KMyMoneyAccountCompletion::~KMyMoneyAccountCompletion()
{
}

QStringList KMyMoneyAccountCompletion::accountList(const QList<eMyMoney::Account::Type>& list = QList<eMyMoney::Account::Type>()) const
{
    return selector()->accountList(list);
}

QStringList KMyMoneyAccountCompletion::accountList() const
{
    return accountList(QList<eMyMoney::Account::Type>());
}

KMyMoneyAccountSelector* KMyMoneyAccountCompletion::selector() const
{
    Q_D(const KMyMoneyCompletion);
//  return nullptr;
    return dynamic_cast<KMyMoneyAccountSelector*>(d->m_selector);
}

void KMyMoneyAccountCompletion::slotMakeCompletion(const QString& txt)
{
    Q_D(KMyMoneyCompletion);
    // if(txt.isEmpty() || txt.length() == 0)
    //  return;

    auto cnt = 0;
    if (txt.contains(MyMoneyFile::AccountSeparator) == 0) {
        d->m_lastCompletion = QRegularExpression(QRegularExpression::escape(txt), QRegularExpression::CaseInsensitiveOption);
        cnt = selector()->slotMakeCompletion(txt);
    } else {
        QStringList parts = txt.split(MyMoneyFile::AccountSeparator, QString::SkipEmptyParts);
        QString pattern("^");
        QStringList::iterator it;
        for (it = parts.begin(); it != parts.end(); ++it) {
            if (pattern.length() > 1)
                pattern += MyMoneyFile::AccountSeparator;
            pattern += QRegularExpression::escape(QString(*it).trimmed()) + ".*";
        }
        pattern += '$';
        d->m_lastCompletion = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        cnt = selector()->slotMakeCompletion(d->m_lastCompletion);
        // if we don't have a match, we try it again, but this time
        // we add a wildcard for the top level
        if (cnt == 0) {
            pattern = pattern.insert(1, QString(".*") + MyMoneyFile::AccountSeparator);
            d->m_lastCompletion = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
            cnt = selector()->slotMakeCompletion(d->m_lastCompletion);
        }
    }

    if (d->m_parent && d->m_parent->isVisible() && !isVisible() && cnt)
        show(false);
    else {
        if (cnt != 0) {
            adjustSize();
        } else {
            hide();
        }
    }
}
