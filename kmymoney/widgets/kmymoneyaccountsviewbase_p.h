/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYACCOUNTSVIEWBASE_P_H
#define KMYMONEYACCOUNTSVIEWBASE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes

#include "KLocalizedString"

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "kmymoneyviewbase_p.h"
#include "kmymoneysettings.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "viewenums.h"

class AccountsViewProxyModel;
class KMyMoneyAccountTreeView;
class KMyMoneyAccountsViewBasePrivate : public KMyMoneyViewBasePrivate
{
public:
    KMyMoneyAccountsViewBasePrivate() :
        m_proxyModel(nullptr),
        m_accountTree(nullptr)
    {
    }

    ~KMyMoneyAccountsViewBasePrivate()
    {
    }

    void netBalProChanged(const MyMoneyMoney &val, QLabel *label, const View view)
    {
        QString s;
        const auto isNegative = val.isNegative();
        switch (view) {
        case View::Institutions:
        case View::Accounts:
            s = i18n("Net Worth: ");
            break;
        case View::Categories:
            if (isNegative)
                s = i18n("Loss: ");
            else
                s = i18n("Profit: ");
            break;
        case View::Budget:
            s = (i18nc("The balance of the selected budget", "Balance: "));
            break;
        default:
            return;
        }

        // FIXME figure out how to deal with the approximate
        // if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
        //  s += "~ ";

        s.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));

        if (isNegative)
            s.append(QLatin1String("<b><font color=\"red\">"));
        const auto sec = MyMoneyFile::instance()->baseCurrency();
        QString v(MyMoneyUtils::formatMoney(val, sec));
        s.append((v.replace(QLatin1Char(' '), QLatin1String("&nbsp;"))));
        if (isNegative)
            s.append(QLatin1String("</font></b>"));

        label->setFont(KMyMoneySettings::listCellFontEx());
        label->setText(s);
    }

    AccountsViewProxyModel  *m_proxyModel;
    KMyMoneyAccountTreeView **m_accountTree;
};

#endif
