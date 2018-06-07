/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
